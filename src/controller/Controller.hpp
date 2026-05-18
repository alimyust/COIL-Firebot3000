#pragma once

#include <Arduino.h>
#include "RFComm.hpp"

class Controller {
public:
    Controller(uint8_t joyXPin = A1, uint8_t joyYPin = A2);

    void begin();
    void update();

private:
    enum CommandId : uint8_t {
        THROTTLE = 0x05,
        STEERING_DUTY = 0x06,
    };

    static constexpr uint8_t ROBOT_NODE_ID = 1;
    static constexpr uint8_t CONTROLLER_NODE_ID = 2;
    static constexpr float RF_FREQUENCY_MHZ = 868.0f;
    static constexpr uint16_t ANALOG_CENTER = 512;
    static constexpr uint16_t JOYSTICK_DEADZONE = 32;
    static constexpr uint32_t SEND_INTERVAL_MS = 50;

    RF69_Comm _comm;
    uint8_t _joyXPin;
    uint8_t _joyYPin;
    unsigned long _lastSendTime;
    uint8_t _lastThrottleDuty;
    uint8_t _lastSteeringDuty;
    bool _hasSentState;

    uint8_t mapAnalogToDuty(uint8_t analogPin, bool invert = false) const;
    bool sendDuty(uint8_t command, uint8_t duty);
};
