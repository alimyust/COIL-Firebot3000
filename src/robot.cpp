

#include "ProtocolLayer.hpp"
#include "DualHWPwm.hpp"
#include "Arduino.h"

namespace {
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";
constexpr uint32_t PWM_FREQUENCY_HZ = 60;

RF69_Comm comm(ROBOT_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, CONTROLLER_NODE_ID);
DualHardwarePWM pwm(9, 5);

void onThrottleCommand(uint8_t duty) {
    Serial.print("RX throttle=");
    Serial.println(duty);
    pwm.setDutyCycle1(duty);
}

void onSteeringCommand(uint8_t duty) {
    Serial.print("RX steering=");
    Serial.println(duty);
    pwm.setDutyCycle2(duty);
}

void onMotorSpeed(uint8_t speed) {
    Serial.print("RX motor speed=");
    Serial.println(speed);
}

void onSteeringAngle(uint8_t angle) {
    Serial.print("RX steering angle=");
    Serial.println(angle);
}
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    if (!comm.begin(nullptr, ENCRYPTION_KEY)) {
        Serial.println("Robot radio init failed");
        return;
    }

    pwm.begin(PWM_FREQUENCY_HZ);
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
