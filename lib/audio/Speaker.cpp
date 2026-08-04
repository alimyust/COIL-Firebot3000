#include "Speaker.hpp"

// Initialize static instance pointer
Speaker* Speaker::instance = nullptr;

Speaker::Speaker() : _dac_pin(A0), _head(0), _tail(0), _count(0), _last_sample(0), _has_last_sample(false) {}

void Speaker::begin() {
    instance = this;

    // Set resolution to 10-bit (0-1023) if supported by the board core
    analogWriteResolution(10);

    // Initialize the DAC pin to mid-scale (1.65V)
    // analogWrite() on pin A0 handles all SAMD pin peripheral muxing, 
    // APBC clocks, and DAC CTRLA/CTRLB register configuration automatically.
    analogWrite(_dac_pin, 512);

    beginTimer();
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
    // Target: 16,000 Hz sample rate
    // Count = 3,000,000 / 16,000 = 187.5 total ticks per cycle (~188)
    // Match Register (CC0) = 188 - 1 = 187
    TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | 
                             TC_CTRLA_PRESCALER_DIV16 | 
                             TC_CTRLA_WAVEGEN_MFRQ;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    TC3->COUNT16.CC[0].reg = 187; // ~16 kHz interrupt frequency (62.5 us period)
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
    // Keep the most recent sample available even if the buffer is momentarily full.
    _last_sample = pcm_sample;
    _has_last_sample = true;

    // Temporarily disable the TC3 interrupt while updating the shared ring-buffer state
    NVIC_DisableIRQ(TC3_IRQn);

    if (_count >= SPEAKER_BUFFER_SIZE) {
        // Drop the oldest sample to make space for the newest one and avoid stalling.
        _tail = (_tail + 1) % SPEAKER_BUFFER_SIZE;
        _count--;
    }

    _buffer[_head] = pcm_sample;
    _head = (_head + 1) % SPEAKER_BUFFER_SIZE;
    _count++;

    NVIC_EnableIRQ(TC3_IRQn);
    return true;
}

void Speaker::isr_playNextSample() {
    if (_count == 0) {
        if (_has_last_sample) {
            int32_t scaled = ((int32_t)_last_sample + 32768) >> 6;
            if (scaled > 1023) scaled = 1023;
            if (scaled < 0) scaled = 0;
            analogWrite(_dac_pin, (int)scaled);
        } else {
            analogWrite(_dac_pin, 512);
        }
        return;
    }

    // 1. Pop sample from ring buffer
    int16_t pcm_sample = _buffer[_tail];
    _tail = (_tail + 1) % SPEAKER_BUFFER_SIZE;
    
    // 2. Decrement count
    _count--;   

    // 3. Keep the latest sample available for underflow recovery
    _last_sample = pcm_sample;
    _has_last_sample = true;

    // 4. Map signed 16-bit (-32768 to +32767) -> 10-bit DAC (0 to 1023)
    int32_t scaled = ((int32_t)pcm_sample + 32768) >> 6;

    // 5. Clamp bounds
    if (scaled > 1023) scaled = 1023;
    if (scaled < 0)    scaled = 0;

    // 6. Update DAC output
    analogWrite(_dac_pin, (int)scaled);
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