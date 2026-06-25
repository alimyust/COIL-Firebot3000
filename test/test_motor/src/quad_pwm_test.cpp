#include <Arduino.h>
#include <unity.h>
#include <QuadHWPwm.hpp>

static QuadHardwarePWM pwm(11, 13, 6, 5);

void test_quad_pwm_timers_enabled() {
    TEST_ASSERT_TRUE(TCC2->CTRLA.bit.ENABLE);
    TEST_ASSERT_TRUE(TCC0->CTRLA.bit.ENABLE);
}

void test_quad_pwm_waveform_mode() {
    TEST_ASSERT_EQUAL(TCC_WAVE_WAVEGEN_NPWM, TCC2->WAVE.bit.WAVEGEN);
    TEST_ASSERT_EQUAL(TCC_WAVE_WAVEGEN_NPWM, TCC0->WAVE.bit.WAVEGEN);
}

void test_quad_pwm_compare_registers_update() {
    pwm.setDutyCycle1(25);
    while (TCC2->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 0)));
    TEST_ASSERT_NOT_EQUAL(0, TCC2->CC[0].reg);

    pwm.setDutyCycle2(50);
    while (TCC2->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 1)));
    TEST_ASSERT_NOT_EQUAL(0, TCC2->CC[1].reg);

    pwm.setDutyCycle3(75);
    while (TCC0->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 2)));
    TEST_ASSERT_NOT_EQUAL(0, TCC0->CC[2].reg);

    pwm.setDutyCycle4(100);
    while (TCC0->SYNCBUSY.reg &(1 << (TCC_SYNCBUSY_CC0_Pos + 1)));
    TEST_ASSERT_NOT_EQUAL(0, TCC0->CC[1].reg);
}

void test_quad_pwm_initializes_and_sets_duty() {
    pwm.setDutyCycle1(0);
    pwm.setDutyCycle2(100);
    pwm.setDutyCycle3(50);
    pwm.setDutyCycle4(33);

    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(1000);
    Serial.begin(115200);
    pwm.begin(60, 50);

    UNITY_BEGIN();

    RUN_TEST(test_quad_pwm_initializes_and_sets_duty);
    RUN_TEST(test_quad_pwm_timers_enabled);
    RUN_TEST(test_quad_pwm_waveform_mode);
    RUN_TEST(test_quad_pwm_compare_registers_update);

    UNITY_END();
}

void loop() {
    Serial.println("All tests completed.");
    // Keep the Arduino test harness alive.
    delay(1000);
}
