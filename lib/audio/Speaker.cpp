#include "Speaker.hpp"

// Initialize static instance pointer
Speaker* Speaker::instance = nullptr;

Speaker::Speaker() : _dac_pin(A0), _head(0), _tail(0), _count(0) {}

void Speaker::begin() {
    instance = this; // Store reference for the ISR

    // Configure DAC Pin (A0 / PA02)
    analogWrite(_dac_pin, 512); 
    
    // Disable DAC to modify control register safely
    DAC->CTRLA.bit.ENABLE = 0;
    while (DAC->STATUS.bit.SYNCBUSY);

    // Set 10-bit mode, fast drive, internal VDDANA reference
    DAC->CTRLB.reg = DAC_CTRLB_REFSEL_INT1V;    
    DAC->CTRLA.bit.ENABLE = 1;
    while (DAC->STATUS.bit.SYNCBUSY);

    Serial.println("Hardware DAC Initialized on A0 (10-bit mode, 8kHz interrupt)");
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