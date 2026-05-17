#include <Arduino.h>
#include "DualHWPwm.hpp"
#include "ProtocolLayer.hpp"
#include "CommandHandler.hpp"

const uint8_t DRIVE_PWM_PIN = 9;
const uint8_t STEER_PWM_PIN = 5;

DualHardwarePWM pwm(DRIVE_PWM_PIN, STEER_PWM_PIN);
RF69_Comm comm(1, 868.0f);
ProtocolLayer protocol(comm);
CommandHandler commandHandler(pwm);

void throttleCallback(uint8_t duty) {
    commandHandler.setThrottleDuty(duty);
}

void steeringCallback(uint8_t duty) {
    commandHandler.setSteeringDuty(duty);
}

void setup() {
    Serial.begin(9600);
    comm.begin(nullptr, "encryptionkey16");
    commandHandler.initialize(50); // 50 Hz for servos
    protocol.setThrottleCallback(throttleCallback);
    protocol.setSteeringDutyCallback(steeringCallback);
    Serial.println("Robot started");
}

void loop() {
    protocol.process();
}
