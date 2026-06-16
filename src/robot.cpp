

#include "ProtocolLayer.hpp"
#include "DualHWPwm.hpp"
#include <Servo.h>
#include "Arduino.h"

namespace {
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";
constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;

RF69_Comm comm(ROBOT_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, CONTROLLER_NODE_ID);
DualHardwarePWM pwm(9, 5);
// Servo throttleServo;
// Servo steeringServo;

uint16_t dutyToMicroseconds(uint8_t duty) {
    return map(duty, -100, 100, SERVO_MIN_US, SERVO_MAX_US);
}

uint16_t rangeToDuty(int16_t value) {
    return map(value, 0, 50, 7, 13);
}

void onThrottleCommand(uint8_t duty) {
    Serial.print("RX throttle=");
    Serial.println(rangeToDuty(duty));
    // throttleServo.writeMicroseconds(dutyToMicroseconds(duty));
    pwm.setDutyCycle1(rangeToDuty(duty));
}

void onSteeringCommand(uint8_t duty) {
    Serial.print("RX steering=");
    Serial.println(rangeToDuty(duty));
    // steeringServo.writeMicroseconds(dutyToMicroseconds(duty));

    pwm.setDutyCycle2(rangeToDuty(duty));
}

void onMotorSpeed(uint8_t speed) {
    Serial.print("RX motor speed=");
    Serial.println(rangeToDuty(speed));
}

void onSteeringAngle(uint8_t angle) {
    Serial.print("RX steering angle=");
    Serial.println(rangeToDuty(angle));
}
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    if (!comm.begin(nullptr, ENCRYPTION_KEY)) {
        Serial.println("Robot radio init failed");
        return;
    }

    // throttleServo.attach(9);
    // steeringServo.attach(11);
    // throttleServo.writeMicroseconds(SERVO_MIN_US);
    // steeringServo.writeMicroseconds(SERVO_MIN_US);

    pwm.begin(60);
    pwm.setDutyCycle1(0);
    pwm.setDutyCycle2(0);

    protocol.setRemoteNodeId(CONTROLLER_NODE_ID);
    
    protocol.setThrottleCallback(onThrottleCommand);
    protocol.setSteeringDutyCallback(onSteeringCommand);
    protocol.setMotorCallback(onMotorSpeed);
    protocol.setSteeringCallback(onSteeringAngle);

    Serial.println("Robot radio started");
}

void loop() {
    protocol.process();
}
