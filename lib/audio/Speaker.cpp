#include "Speaker.hpp"

// Initialize static instance pointer
Speaker* Speaker::instance = nullptr;

Speaker::Speaker() : _dac_pin(A0), _head(0), _tail(0), _count(0) {}

void Speaker::begin() {
    instance = this; // Store reference for the ISR

    // Configure DAC Pin (A0 / PA02)
    analogWrite(_dac_pin, 512); 
    
    // Disable DAC to modify control register safelys
    DAC->CTRLA.bit.ENABLE = 0;
    while (DAC->STATUS.bit.SYNCBUSY);

    // Set 10-bit mode, fast drive, internal VDDANA reference
    // DAC->CTRLB.reg = DAC_CTRLB_REFSEL_INT1V;    
    DAC->CTRLB.reg = DAC_CTRLB_REFSEL_AVCC | DAC_CTRLB_EOEN;
    DAC->CTRLA.bit.ENABLE = 1;
    while (DAC->STATUS.bit.SYNCBUSY);

    Serial.println("Hardware DAC Initialized on A0 (10-bit mode, 8kHz interrupt)");
    beginTimer();
    Serial.println("8khz Speaker Timer initialized)");

}

void Speaker::beginTimer() {
    // Enable GCLK0 (48 MHz) for TCC2_TC3
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID_TCC2_TC3;
    while (GCLK->STATUS.bit.SYNCBUSY);

    // Disable TC3 to modify configuration registers safely
    TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    // 48 MHz / 16 prescaler = 3,000,000 Hz timer clock
    // Target: 8,000 Hz sample rate
    // Count = 3,000,000 / 8,000 = 375 total ticks per cycle
    // Match Register (CC0) = 375 - 1 = 374
    TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | 
                             TC_CTRLA_PRESCALER_DIV16 | 
                             TC_CTRLA_WAVEGEN_MFRQ;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    TC3->COUNT16.CC[0].reg = 374; // Exactly 8 kHz interrupt frequency (125 us period)
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    // Enable Match Channel 0 interrupt
    TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;
    
    // Set high priority for audio playback ISR to avoid jitter
    NVIC_SetPriority(TC3_IRQn, 1);
    NVIC_EnableIRQ(TC3_IRQn);

    // Enable TC3
    TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

bool Speaker::queueAudio(int16_t pcm_sample) {
    if (_count >= SPEAKER_BUFFER_SIZE) {
        return false; // Buffer overflow, drop sample
    }
    
    _buffer[_head] = pcm_sample;
    _head = (_head + 1) % SPEAKER_BUFFER_SIZE;
    
    // Temporarily disable the TC3 interrupt while updating the shared count variable
    NVIC_DisableIRQ(TC3_IRQn);
    _count++;
    NVIC_EnableIRQ(TC3_IRQn);
    
    return true;
}

void Speaker::isr_playNextSample() {
    if (_count == 0) {
        // Buffer underflow: Maintain the last DC voltage or center at 0
        return; 
    }
    // Pop sample
    int16_t pcm_sample = _buffer[_tail];
    _tail = (_tail + 1) % SPEAKER_BUFFER_SIZE;
    _count--;   

    // Convert signed 16-bit to 10-bit unsigned (0 to 1023)
    uint16_t dac_val = (uint16_t)(((int32_t)pcm_sample + 32768) >> 6);
    
    // Write directly to DAC Data register for maximum ISR speed
    while (DAC->STATUS.bit.SYNCBUSY);
    DAC->DATA.reg = dac_val;
}

extern "C" void TC3_Handler() {
    // Clear the Match 0 interrupt flag
    if (TC3->COUNT16.INTFLAG.bit.MC0) {
        TC3->COUNT16.INTFLAG.bit.MC0 = 1; 
        
        // Output one sample to the DAC
        if (Speaker::instance) {
            Speaker::instance->isr_playNextSample();
        }
    }
}