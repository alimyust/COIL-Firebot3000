#include "Speaker.hpp"

Speaker::Speaker(uint8_t dac_pin) : _dac_pin(dac_pin) {}

void Speaker::begin() {
    Serial.println("Speaker initialized");
}

void Speaker::write(int16_t pcm_sample) {
    // Convert signed 16-bit (-32768 to 32767) to unsigned 10-bit (0 to 1023)
    int32_t dac_val = ((int32_t)pcm_sample + 32768) >> 6;
    Serial.print("PCM Sample: "); Serial.print(pcm_sample);
    Serial.print("  DAC Value: "); Serial.println(dac_val);
    analogWrite(_dac_pin, (uint16_t)dac_val);
    
    // Maintaining sample rate (approx. 16kHz -> 62.5 microseconds spacing)
    // delayMicroseconds(62); 
}