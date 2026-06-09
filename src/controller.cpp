#include "Joystick.hpp"
#include "ProtocolLayer.hpp"
#include "Arduino.h"

namespace {
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";
constexpr unsigned long SEND_INTERVAL_MS = 100;

RF69_Comm comm(CONTROLLER_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, ROBOT_NODE_ID);
Joystick robot_joy(A3, A2, true);

int lastThrottleDuty = -1;
int lastSteeringDuty = -1;
unsigned long lastSendTime = 0;

uint8_t mapJoystickToDuty(int value) {
    const int range = 512;
    value = constrain(value, -range, range);
    return static_cast<uint8_t>(map(value, -range, range, 0, 100));
}

void sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty) {
    protocol.sendThrottle(throttleDuty);
    protocol.sendSteering(steeringDuty);
    Serial.print("TX throttle=");
    Serial.print(throttleDuty);
    Serial.print(" steering=");
    Serial.println(steeringDuty);
}
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
    robot_joy.init_joystick();

    if (!comm.begin(nullptr, ENCRYPTION_KEY)) {
        Serial.println("Controller radio init failed");
        return;
    }
    protocol.setRemoteNodeId(ROBOT_NODE_ID);
    Serial.println("Controller radio started");
}

void loop() {
    int x = 0;
    int y = 0;
    robot_joy.update_joystick(x, y);

    const uint8_t steeringDuty = mapJoystickToDuty(x);
    const uint8_t throttleDuty = mapJoystickToDuty(y);

    const unsigned long now = millis();
    if (now - lastSendTime >= SEND_INTERVAL_MS) {
        if (steeringDuty != lastSteeringDuty || throttleDuty != lastThrottleDuty) {
            sendControlValues(throttleDuty, steeringDuty);
            lastSteeringDuty = steeringDuty;
            lastThrottleDuty = throttleDuty;
        }
        lastSendTime = now;
    }
}
