#include <Arduino.h>
#include "wiring_private.h"

static const uint8_t PIN_THROTTLE = 9;
static const uint8_t PIN_STEERING = 11;

static const uint32_t SERVO_HZ = 60;
static const uint32_t PRESCALER_DIV = 64;
static const uint32_t CPU_HZ = 48000000UL;
static const uint32_t PWM_PERIOD_COUNTS = (CPU_HZ / (PRESCALER_DIV * SERVO_HZ)) - 1;

static const uint16_t THROTTLE_MIN_US = 1250;
static const uint16_t THROTTLE_NEUTRAL_US = 1500;
static const uint16_t THROTTLE_MAX_US = 1750;

static const uint16_t STEERING_MIN_US = 1083;
static const uint16_t STEERING_NEUTRAL_US = 1500;
static const uint16_t STEERING_MAX_US = 2083;

static const uint32_t STEP_DELAY_MS = 2000;

static inline uint32_t microsecondsToCounts(uint16_t pulseUs) {
    return (pulseUs * (CPU_HZ / PRESCALER_DIV)) / 1000000UL;
}

static void setupTcc1For60Hz() {
    GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC0_TCC1_Val) |
                                   GCLK_CLKCTRL_GEN_GCLK0 |
                                   GCLK_CLKCTRL_CLKEN);
    while (GCLK->STATUS.bit.SYNCBUSY) {}

    PM->APBCMASK.reg |= PM_APBCMASK_TCC1;

    TCC1->CTRLA.bit.ENABLE = 0;
    while (TCC1->SYNCBUSY.bit.ENABLE) {}

    TCC1->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
    while (TCC1->SYNCBUSY.bit.WAVE) {}

    TCC1->PER.reg = PWM_PERIOD_COUNTS;
    while (TCC1->SYNCBUSY.bit.PER) {}

    TCC1->CTRLA.reg = (TCC1->CTRLA.reg & ~TCC_CTRLA_PRESCALER_Msk) |
                      TCC_CTRLA_PRESCALER_DIV64;

    TCC1->CTRLA.bit.ENABLE = 1;
    while (TCC1->SYNCBUSY.bit.ENABLE) {}
}

static void setupTcc2For60Hz() {
    GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3_Val) |
                                   GCLK_CLKCTRL_GEN_GCLK0 |
                                   GCLK_CLKCTRL_CLKEN);
    while (GCLK->STATUS.bit.SYNCBUSY) {}

    PM->APBCMASK.reg |= PM_APBCMASK_TCC2;

    TCC2->CTRLA.bit.ENABLE = 0;
    while (TCC2->SYNCBUSY.bit.ENABLE) {}

    TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
    while (TCC2->SYNCBUSY.bit.WAVE) {}

    TCC2->PER.reg = PWM_PERIOD_COUNTS;
    while (TCC2->SYNCBUSY.bit.PER) {}

    TCC2->CTRLA.reg = (TCC2->CTRLA.reg & ~TCC_CTRLA_PRESCALER_Msk) |
                      TCC_CTRLA_PRESCALER_DIV64;

    TCC2->CTRLA.bit.ENABLE = 1;
    while (TCC2->SYNCBUSY.bit.ENABLE) {}
}

static void writePulseMicroseconds(uint8_t pin, uint16_t pulseUs) {
    pulseUs = constrain(pulseUs, 500, 2500);

    uint32_t compareValue = microsecondsToCounts(pulseUs);
    if (compareValue > PWM_PERIOD_COUNTS) {
        compareValue = PWM_PERIOD_COUNTS;
    }

    if (pin == PIN_THROTTLE) {
        pinPeripheral(PIN_THROTTLE, PIO_TIMER);
        TCC1->CC[1].reg = compareValue;
        while (TCC1->SYNCBUSY.bit.CC1) {}
        return;
    }

    if (pin == PIN_STEERING) {
        pinPeripheral(PIN_STEERING, PIO_TIMER);
        TCC2->CC[0].reg = compareValue;
        while (TCC2->SYNCBUSY.bit.CC0) {}
    }
}

static void setOutputs(uint16_t throttleUs, uint16_t steeringUs, const char* label) {
    writePulseMicroseconds(PIN_THROTTLE, throttleUs);
    writePulseMicroseconds(PIN_STEERING, steeringUs);

    Serial.print(label);
    Serial.print(" throttle=");
    Serial.print(throttleUs);
    Serial.print("us steering=");
    Serial.print(steeringUs);
    Serial.println("us");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    setupTcc1For60Hz();
    setupTcc2For60Hz();

    setOutputs(THROTTLE_NEUTRAL_US, STEERING_NEUTRAL_US, "Startup neutral");
    delay(STEP_DELAY_MS);
}

void loop() {
    setOutputs(THROTTLE_NEUTRAL_US, STEERING_NEUTRAL_US, "Neutral");
    delay(STEP_DELAY_MS);

    setOutputs(THROTTLE_MIN_US, STEERING_NEUTRAL_US, "Throttle min");
    delay(STEP_DELAY_MS);

    setOutputs(THROTTLE_MAX_US, STEERING_NEUTRAL_US, "Throttle max");
    delay(STEP_DELAY_MS);

    setOutputs(THROTTLE_NEUTRAL_US, STEERING_MIN_US, "Steering min");
    delay(STEP_DELAY_MS);

    setOutputs(THROTTLE_NEUTRAL_US, STEERING_MAX_US, "Steering max");
    delay(STEP_DELAY_MS);
}
