

#include "ProtocolLayer.hpp"
#include "Arduino.h"

namespace {
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";

RF69_Comm comm(ROBOT_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, CONTROLLER_NODE_ID);

void onThrottleCommand(uint8_t duty) {
    Serial.print("RX throttle=");
    Serial.println(duty);
    // TODO: drive motor PWM or ESC using this duty value.
}

void onSteeringCommand(uint8_t duty) {
    Serial.print("RX steering=");
    Serial.println(duty);
    // TODO: control steering servo using this duty value.
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
