#include "Microphone.hpp"
#include <Arduino.h>

//sample block length at 64

// ============================================================================
// STATIC VARIABLE DEFINITIONS (This allocates the actual memory!)
// ============================================================================
uint16_t Microphone::adc_buffer[SAMPLE_BLOCK_LENGTH * 2];
bool Microphone::filling_first_half = true;
volatile uint16_t* Microphone::active_adc_buffer = nullptr;
bool Microphone::adc_buffer_filled = false;

// ============================================================================
// MEMBER FUNCTION IMPLEMENTATIONS
// ============================================================================


Microphone::Microphone() {
    filling_first_half = true;
    active_adc_buffer = nullptr;
    adc_buffer_filled = false;
}

void Microphone::begin() {
    adc_init();
    dma_init();
    ADC_DMA.startJob();
}

bool Microphone::isBufferReady() {
    return adc_buffer_filled;
}

void Microphone::readActiveBuffer(int16_t* output_buffer) {
    if (!adc_buffer_filled || active_adc_buffer == nullptr) return;

    for (int i = 0; i < SAMPLE_BLOCK_LENGTH; i++) {
        int32_t val = (int32_t)active_adc_buffer[i] - 2048;
        output_buffer[i] = (int16_t)(val << 4);
    }
    adc_buffer_filled = false;
}

void Microphone::adc_init() {
    analogRead(ADC_PIN);
    ADC->CTRLA.bit.ENABLE = 0;
    ADCsync();
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
    ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1;
    ADCsync();
    ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[ADC_PIN].ulADCChannelNumber;
    ADCsync();
    ADC->AVGCTRL.reg = 0;
    ADC->SAMPCTRL.reg = 2;
    ADCsync();
    // 1. DIV32 prescaler (1.5 MHz ADC clock for fast conversion)
    // 2. NO FREERUN MODE!
    ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_RESSEL_12BIT;
    ADCsync();

    // Enable Start Conversion on Event Input
    ADC->EVCTRL.bit.STARTEI = 1; 
    ADCsync();

    ADC->CTRLA.bit.ENABLE = 1;
    ADCsync();

    // Start TC4 to generate the 8 kHz event stream
    beginAdcTimer();
}

void Microphone::beginAdcTimer() {
    // 1. Enable GCLK0 (48 MHz) for TC4/TC5
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID_TC4_TC5;
    while (GCLK->STATUS.bit.SYNCBUSY);

    // 2. Disable TC4
    TC4->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

    // 3. 48 MHz / 16 prescaler = 3 MHz clock
    TC4->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | 
                             TC_CTRLA_PRESCALER_DIV16 | 
                             TC_CTRLA_WAVEGEN_MFRQ;
    while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

    // 3,000,000 / 8,000 Hz = 375 ticks (CC0 = 374)
    TC4->COUNT16.CC[0].reg = 374; 
    while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

    // Enable Event Output on Match (No CPU Interrupt needed!)
    TC4->COUNT16.EVCTRL.reg = TC_EVCTRL_MCEO0;

    // 4. Connect TC4 Match Event -> EVSYS Channel 0 -> ADC STARTEI
    PM->APBCMASK.reg |= PM_APBCMASK_EVSYS; // Enable EVSYS bus clock
    
    // Wire EVSYS User ADC_START to Channel 0
    EVSYS->USER.reg = EVSYS_USER_CHANNEL(1) | EVSYS_USER_USER(EVSYS_ID_USER_ADC_START);
    
    // Set Channel 0 generator to TC4 MCX0 (Match 0)
// Set Channel 0 generator to TC4 Match 0 (Notice the underscore: MC_0)
    EVSYS->CHANNEL.reg = EVSYS_CHANNEL_CHANNEL(0) | 
                        EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_TC4_MCX_0) | 
                        EVSYS_CHANNEL_PATH_ASYNCHRONOUS;
    // 5. Enable TC4
    TC4->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
}

void Microphone::dma_init() {
    ADC_DMA.allocate();
    ADC_DMA.setTrigger(ADC_DMAC_ID_RESRDY);
    ADC_DMA.setAction(DMA_TRIGGER_ACTON_BEAT);
    
    dmac_descriptor_1 = ADC_DMA.addDescriptor(
            (void *)(&ADC->RESULT.reg),
            adc_buffer,
            SAMPLE_BLOCK_LENGTH,
            DMA_BEAT_SIZE_HWORD,
            false,
            true);
    dmac_descriptor_1->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;

    dmac_descriptor_2 = ADC_DMA.addDescriptor(
            (void *)(&ADC->RESULT.reg),
            adc_buffer + SAMPLE_BLOCK_LENGTH,
            SAMPLE_BLOCK_LENGTH,
            DMA_BEAT_SIZE_HWORD,
            false,
            true);
    dmac_descriptor_2->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;

    ADC_DMA.loop(true);
    ADC_DMA.setCallback(dma_callback);
}

void Microphone::dma_callback(Adafruit_ZeroDMA *dma) {
    (void) dma;
    if (filling_first_half) {
        active_adc_buffer = &adc_buffer[0];
        filling_first_half = false;
    } else {
        active_adc_buffer = &adc_buffer[SAMPLE_BLOCK_LENGTH];
        filling_first_half = true;
    }
    adc_buffer_filled = true;
}