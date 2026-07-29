#include <unity.h>
#include <Arduino.h>
#include "Speaker.hpp"

Speaker speaker(A0);

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

// 1. Verify DAC peripheral registers initialization
void test_dac_initialization(void) {
    speaker.begin();

    // Check DAC CTRLA Enable bit
    TEST_ASSERT_TRUE_MESSAGE(DAC->CTRLA.bit.ENABLE == 1, "DAC CTRLA bit ENABLE was not set!");
    
    // Check DAC CTRLB Reference is set to internal 1V
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(
        DAC_CTRLB_REFSEL_INT1V, 
        DAC->CTRLB.reg & DAC_CTRLB_REFSEL_Msk, 
        "DAC CTRLB Reference is not set to INT1V!"
    );
}

// 2. Verify PCM signed to 10-bit DAC offset mapping math
void test_pcm_to_dac_scaling(void) {
    // Silence (0 PCM) -> should yield 512 mid-rail
    int16_t pcm_zero = 0;
    uint16_t dac_mid = (uint16_t)(((int32_t)pcm_zero + 32768) >> 6);
    TEST_ASSERT_EQUAL_UINT16(512, dac_mid);

    // Max Positive PCM (32767) -> should yield ~1023
    int16_t pcm_max = 32767;
    uint16_t dac_max = (uint16_t)(((int32_t)pcm_max + 32768) >> 6);
    TEST_ASSERT_EQUAL_UINT16(1023, dac_max);

    // Max Negative PCM (-32768) -> should yield 0
    int16_t pcm_min = -32768;
    uint16_t dac_min = (uint16_t)(((int32_t)pcm_min + 32768) >> 6);
    TEST_ASSERT_EQUAL_UINT16(0, dac_min);
}

// 3. Output a 1 kHz test sine wave on A0 for oscilloscope or audio probe verification
void test_dac_sine_wave_output(void) {
    speaker.begin();

    // Generate 16 kHz sample rate output for 1 second (16000 iterations @ 62.5 us delay)
    const float frequency = 1000.0; // 1 kHz sine tone
    const float sample_rate = 16000.0;
    
    for (int i = 0; i < 16000; i++) {
        float sample = sin(2.0 * M_PI * frequency * ((float)i / sample_rate));
        int16_t pcm_val = (int16_t)(sample * 16000.0); // Scale amplitude safely
        
        speaker.write(pcm_val);
        delayMicroseconds(62); // ~16 kHz pacing for test output
    }

    // Pass if DAC was synced and completed output loop without hanging
    TEST_ASSERT_FALSE(DAC->STATUS.bit.SYNCBUSY);
}

void setup() {
    delay(2000); // Wait for Serial Monitor on ATSAMD21
    UNITY_BEGIN();
    RUN_TEST(test_dac_initialization);
    RUN_TEST(test_pcm_to_dac_scaling);
    RUN_TEST(test_dac_sine_wave_output);
    UNITY_END();
}

void loop() {
    // Tests run once in setup()
}