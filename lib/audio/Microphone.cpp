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
    ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_FREERUN | ADC_CTRLB_RESSEL_12BIT;
    ADCsync();
    ADC->CTRLA.bit.ENABLE = 1;
    ADCsync();
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