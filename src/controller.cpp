#include "ControllerHandler.hpp"
#include "radio.h"
#include "Arduino.h"
#include "DebugLog.hpp"

namespace {
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";

EventRadioComm comm(CONTROLLER_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, ROBOT_NODE_ID);
Joystick robot_joy(A3, A2, true);
ControllerHandler controller_handler(protocol, robot_joy, true);
}

void setup() {
    Serial.begin(115200);
    robot_joy.init_joystick();

    if (!comm.begin(nullptr, ENCRYPTION_KEY)) {
        Serial.println("Controller radio init failed");
        return;
    }

    protocol.setRemoteNodeId(ROBOT_NODE_ID);
    protocol.setHandler(&controller_handler);
    Serial.println("Controller radio started");
}

void loop() {
    protocol.process();
    controller_handler.update();
    DebugLog::flush();
}
