#pragma once

#include <Arduino.h>
#include "../../lib/tagtronics/RFComm.hpp"
#include "../robot/ProtocolLayer.hpp"

// Controller class for reading joystick and sending commands via ProtocolLayer

class Controller {
public:
    Controller(uint8_t joyXPin = A1, uint8_t joyYPin = A2);

    void begin();
    void update();

private:
    RF69_Comm _comm;
    ProtocolLayer _protocol;
    uint8_t _joyXPin;
    uint8_t _joyYPin;

    const uint16_t JOYSTICK_DEADZONE = 32;
    const uint32_t SEND_INTERVAL_MS = 50;
    unsigned long _lastSendTime = 0;

    uint8_t mapJoystickToDuty(uint8_t analogPin);
};