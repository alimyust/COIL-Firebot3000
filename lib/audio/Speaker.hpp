#ifndef SPEAKER_HPP
#define SPEAKER_HPP

#include <Arduino.h>

class Speaker {
public:
    Speaker(uint8_t dac_pin = A0);
    void begin();
    
    // Writes a single signed 16-bit PCM sample to the 10-bit hardware DAC
    void write(int16_t pcm_sample);

private:
    uint8_t _dac_pin;
};

#endif // SPEAKER_HPP