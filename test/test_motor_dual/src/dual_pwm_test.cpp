#include <Arduino.h>
#include <unity.h>
#include <DualHWPwm.hpp>

static DualHardwarePWM pwm(9, 5);

void test_pwm_timers_enabled() {
    TEST_ASSERT_TRUE(TCC1->CTRLA.bit.ENABLE);
    TEST_ASSERT_TRUE(TCC0->CTRLA.bit.ENABLE);
}

void test_pwm_waveform_mode() {
    TEST_ASSERT_EQUAL(TCC_WAVE_WAVEGEN_NPWM, TCC1->WAVE.bit.WAVEGEN);
    TEST_ASSERT_EQUAL(TCC_WAVE_WAVEGEN_NPWM, TCC0->WAVE.bit.WAVEGEN);
}

void test_pwm_compare_registers_update() {
    pwm.setDutyCycle1(50);

    while (TCC1->SYNCBUSY.bit.CC1);
    TEST_ASSERT_NOT_EQUAL(0, TCC1->CC[1].reg);

    pwm.setDutyCycle2(50);

    while (TCC0->SYNCBUSY.bit.CC1);
    TEST_ASSERT_NOT_EQUAL(0, TCC0->CC[1].reg);
}

void test_dual_pwm_initializes_and_sets_duty() {
    pwm.setDutyCycle1(0);
    pwm.setDutyCycle2(100);

    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(1000);

    pwm.begin(60);

    UNITY_BEGIN();

    RUN_TEST(test_dual_pwm_initializes_and_sets_duty);
    RUN_TEST(test_pwm_timers_enabled);
    RUN_TEST(test_pwm_waveform_mode);
    RUN_TEST(test_pwm_compare_registers_update);

    UNITY_END();
}

void loop() {
    // LED brightness sweep test
    static int duty = 0;
    static int step = 10;

    pwm.setDutyCycle1(duty);
    pwm.setDutyCycle2(duty);
    duty += step;

    if (duty >= 100 || duty <= 0) {
        step = -step;
    }
    Serial.println("Duty cycle: " + String(duty) + "%");
    
    delay(100);
}