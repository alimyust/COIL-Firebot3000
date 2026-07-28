#include "Speaker.hpp"

Speaker::Speaker(uint8_t dac_pin) : _dac_pin(dac_pin) {}

void Speaker::begin() {
    // Configure DAC Pin (A0 / PA02)
    analogWrite(_dac_pin, 512); // Enables SAMD21 DAC peripheral
    
    // Disable DAC to modify control register safely
    DAC->CTRLA.bit.ENABLE = 0;
    while (DAC->STATUS.bit.SYNCBUSY);

    // Set 10-bit mode, fast drive, internal VDDANA reference
    DAC->CTRLB.reg = DAC_CTRLB_REFSEL_INT1V;    
    DAC->CTRLA.bit.ENABLE = 1;
    while (DAC->STATUS.bit.SYNCBUSY);

    Serial.println("Hardware DAC Initialized on A0 (10-bit mode)");
}

void Speaker::write(int16_t pcm_sample) {
    // Convert signed 16-bit (-32768 to 32767) to 10-bit unsigned (0 to 1023)
    // Offset by +32768 to shift range to [0, 65535], then right-shift 6 bits for 10-bit
    uint16_t dac_val = (uint16_t)(((int32_t)pcm_sample + 32768) >> 6);
    
    writeRawDAC(dac_val);
}