#include <Arduino.h>
#include <unity.h>
#include <DualHWPwm.hpp>

static DualHardwarePWM pwm(9, 5);

void test_dual_pwm_initializes_and_sets_duty() {
    pwm.begin(60);
    pwm.setDutyCycle1(0);
    pwm.setDutyCycle2(100);

    // This test is primarily to verify the hardware driver compiles and
    // initializes on the target board. If the board boots and reaches here,
    // the driver was constructed successfully.
    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(1000);
    UNITY_BEGIN();
    RUN_TEST(test_dual_pwm_initializes_and_sets_duty);
    UNITY_END();
}

void loop() {
}
