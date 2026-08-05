#include <Arduino.h>
#include <unity.h>
#include <PentaHWPwm.hpp>

static PentaHardwarePWM pwm;

static bool pinHasBothStates(uint8_t pin, uint8_t portGroup, uint32_t timeoutMs = 100) {
    const uint32_t mask = (1u << pin);
    uint32_t seen = 0;
    uint32_t start = millis();

    // Enable input sampling while peripheral is active.
    PORT->Group[portGroup].PINCFG[pin].bit.INEN = 1;

    while ((millis() - start) < timeoutMs) {
        uint32_t value = PORT->Group[portGroup].IN.reg;
        if (value & mask) {
            seen |= 1;
        } else {
            seen |= 2;
        }
        if (seen == 3) {
            return true;
        }
    }
    return false;
}

void test_penta_pwm_pin_mux_configuration() {
    // D13 -> PA17, TCC2/WO1
    TEST_ASSERT_TRUE(PORT->Group[PORTA].PINCFG[17].bit.PMUXEN);
    TEST_ASSERT_EQUAL(PORT_PMUX_PMUXO_E_Val,
                      PORT->Group[PORTA].PMUX[8].bit.PMUXO);

    // D11 -> PA16, TCC2/WO0
    TEST_ASSERT_TRUE(PORT->Group[PORTA].PINCFG[16].bit.PMUXEN);
    TEST_ASSERT_EQUAL(PORT_PMUX_PMUXE_E_Val,
                      PORT->Group[PORTA].PMUX[8].bit.PMUXE);

    // D5 -> PA15, TCC0/WO5
    TEST_ASSERT_TRUE(PORT->Group[PORTA].PINCFG[15].bit.PMUXEN);
    TEST_ASSERT_EQUAL(PORT_PMUX_PMUXO_F_Val,
                      PORT->Group[PORTA].PMUX[7].bit.PMUXO);

    // D6 -> PA20, TCC0/WO6
    TEST_ASSERT_TRUE(PORT->Group[PORTA].PINCFG[20].bit.PMUXEN);
    TEST_ASSERT_EQUAL(PORT_PMUX_PMUXE_F_Val,
                      PORT->Group[PORTA].PMUX[10].bit.PMUXE);

    // D12 -> PA19, TCC0/WO3
    TEST_ASSERT_TRUE(PORT->Group[PORTA].PINCFG[19].bit.PMUXEN);
    TEST_ASSERT_EQUAL(PORT_PMUX_PMUXO_F_Val,
                      PORT->Group[PORTA].PMUX[9].bit.PMUXO);
}

void test_penta_pwm_timers_enabled() {
    TEST_ASSERT_TRUE(TCC2->CTRLA.bit.ENABLE);
    TEST_ASSERT_TRUE(TCC0->CTRLA.bit.ENABLE);
}

void test_penta_pwm_waveform_mode() {
    TEST_ASSERT_EQUAL(TCC_WAVE_WAVEGEN_NPWM, TCC2->WAVE.bit.WAVEGEN);
    TEST_ASSERT_EQUAL(TCC_WAVE_WAVEGEN_NPWM, TCC0->WAVE.bit.WAVEGEN);
}

void test_penta_pwm_timer_period_and_duty_math() {
    const uint32_t expectedMotorPER = (48000000UL / (1024UL * 60UL)) - 1UL;
    const uint32_t expectedServoPER = (48000000UL / (1024UL * 50UL)) - 1UL;

    TEST_ASSERT_EQUAL(expectedMotorPER, TCC2->PER.reg);
    TEST_ASSERT_EQUAL(expectedServoPER, TCC0->PER.reg);

    pwm.setDutyThrottle(25);
    while (TCC2->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 0)));
    TEST_ASSERT_EQUAL_UINT32((expectedMotorPER * 25UL) / 100UL,
                             TCC2->CC[0].reg);

    pwm.setDutySteer(50);
    while (TCC2->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 1)));
    TEST_ASSERT_EQUAL_UINT32((expectedMotorPER * 50UL) / 100UL,
                             TCC2->CC[1].reg);

    pwm.setDutyPinTurretZ(75);
    while (TCC0->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 2)));
    TEST_ASSERT_EQUAL_UINT32((expectedServoPER * 75UL) / 100UL,
                             TCC0->CC[2].reg);

    pwm.setDutyPinTurretX(100);
    while (TCC0->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 1)));
    TEST_ASSERT_EQUAL_UINT32((expectedServoPER * 100UL) / 100UL,
                             TCC0->CC[1].reg);

    pwm.setDutyPinTurretY(33);
    while (TCC0->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 3)));
    TEST_ASSERT_EQUAL_UINT32((expectedServoPER * 33UL) / 100UL,
                             TCC0->CC[3].reg);
}

void test_penta_pwm_compare_registers_update() {
    pwm.setDutyThrottle(25);
    while (TCC2->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 0)));
    TEST_ASSERT_NOT_EQUAL(0, TCC2->CC[0].reg);

    pwm.setDutySteer(50);
    while (TCC2->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 1)));
    TEST_ASSERT_NOT_EQUAL(0, TCC2->CC[1].reg);

    pwm.setDutyPinTurretZ(75);
    while (TCC0->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 2)));
    TEST_ASSERT_NOT_EQUAL(0, TCC0->CC[2].reg);

    pwm.setDutyPinTurretX(100);
    while (TCC0->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 1)));
    TEST_ASSERT_NOT_EQUAL(0, TCC0->CC[1].reg);

    pwm.setDutyPinTurretY(33);
    while (TCC0->SYNCBUSY.reg & (1 << (TCC_SYNCBUSY_CC0_Pos + 3)));
    TEST_ASSERT_NOT_EQUAL(0, TCC0->CC[3].reg);
}

void test_penta_pwm_output_pin_toggles() {
    pwm.setDutyThrottle(50);
    TEST_ASSERT_TRUE(pinHasBothStates(16, PORTA));

    pwm.setDutySteer(50);
    TEST_ASSERT_TRUE(pinHasBothStates(17, PORTA));

    pwm.setDutyPinTurretZ(50);
    TEST_ASSERT_TRUE(pinHasBothStates(20, PORTA));

    pwm.setDutyPinTurretX(50);
    TEST_ASSERT_TRUE(pinHasBothStates(15, PORTA));

    pwm.setDutyPinTurretY(50);
    TEST_ASSERT_TRUE(pinHasBothStates(19, PORTA));
}

void test_penta_pwm_initializes_and_sets_duty() {
    pwm.setDutyThrottle(0);
    pwm.setDutySteer(100);
    pwm.setDutyPinTurretZ(50);
    pwm.setDutyPinTurretX(33);
    pwm.setDutyPinTurretY(66);

    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(1000);
    Serial.begin(115200);
    pwm.begin(60, 50);

    UNITY_BEGIN();

    RUN_TEST(test_penta_pwm_initializes_and_sets_duty);
    RUN_TEST(test_penta_pwm_pin_mux_configuration);
    RUN_TEST(test_penta_pwm_timers_enabled);
    RUN_TEST(test_penta_pwm_waveform_mode);
    RUN_TEST(test_penta_pwm_timer_period_and_duty_math);
    RUN_TEST(test_penta_pwm_compare_registers_update);
    RUN_TEST(test_penta_pwm_output_pin_toggles);

    UNITY_END();
}

void loop() {
    pwm.setDutyThrottle(50);
    pwm.setDutySteer(50);
    pwm.setDutyPinTurretZ(50);
    pwm.setDutyPinTurretX(50);
    pwm.setDutyPinTurretY(50);
}