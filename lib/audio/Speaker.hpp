#ifndef SPEAKER_HPP
#define SPEAKER_HPP

#include <Arduino.h>

class Speaker {
public:
    Speaker(uint8_t dac_pin = A0);
    void begin();
    
    // Inline direct register write for speed inside ISR
    inline void writeRawDAC(uint16_t dac_val) {
        // Wait for DAC to be ready
        while (DAC->STATUS.bit.SYNCBUSY);
        // Write 10-bit value directly to DAC DATABUF register
        DAC->DATABUF.reg = dac_val & 0x03FF;
    }

    void write(int16_t pcm_sample);

private:
    uint8_t _dac_pin;
};

#endif