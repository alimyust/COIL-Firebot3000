#include <Arduino.h>
#include "DualHWPwm.hpp"

const uint8_t DRIVE_PWM_PIN = 9;
const uint8_t STEER_PWM_PIN = 5;
const uint8_t JOY_X_PIN = A1;
const uint8_t JOY_Y_PIN = A2;

const uint32_t PWM_FREQUENCY = 50;
const uint8_t SERVO_DUTY_MIN = 5;   // ~1.0 ms at 50 Hz
const uint8_t SERVO_DUTY_CENTER = 8; // ~1.5 ms at 50 Hz
const uint8_t SERVO_DUTY_MAX = 10;  // ~2.0 ms at 50 Hz
const uint16_t JOYSTICK_DEADZONE = 32;

DualHardwarePWM pwm(DRIVE_PWM_PIN, STEER_PWM_PIN);

uint8_t joystickToServoDuty(uint8_t analogPin) {
    int raw = analogRead(analogPin);
    raw = constrain(raw, 0, 1023);

    if (raw >= 512 - JOYSTICK_DEADZONE && raw <= 512 + JOYSTICK_DEADZONE) {
        return SERVO_DUTY_CENTER;
    }

    return (uint8_t)map(raw, 0, 1023, SERVO_DUTY_MIN, SERVO_DUTY_MAX);
}

void setup() {
    Serial.begin(9600);
    pwm.begin(PWM_FREQUENCY);

    pinMode(JOY_X_PIN, INPUT);
    pinMode(JOY_Y_PIN, INPUT);

    Serial.println("DualHardwarePWM joystick control started");
}

void loop() {
    uint8_t driveDuty = joystickToServoDuty(JOY_Y_PIN);
    uint8_t steerDuty = joystickToServoDuty(JOY_X_PIN);

    pwm.setDutyCycle1(driveDuty);
    pwm.setDutyCycle2(steerDuty);

    Serial.print("Drive duty: ");
    Serial.print(driveDuty);
    Serial.print("%  Steer duty: ");
    Serial.print(steerDuty);
    Serial.println("%");

    delay(50);
}
