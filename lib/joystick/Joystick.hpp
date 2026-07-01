#pragma once

#include <Arduino.h>


namespace JoystickConfig {
    static constexpr uint8_t JOY_MIN = 0;
    static constexpr uint8_t JOY_MAX = 255;
    static constexpr uint8_t JOY_CENTER = 128;
    static constexpr int DEADZONE = 0;
};

class Joystick {
public:
    Joystick(uint8_t joyXPin = A3, uint8_t joyYPin = A2, bool debug = false)
        : joyXPin(joyXPin),
          joyYPin(joyYPin),
          debug_enabled(debug) {}

    void init_joystick();
    void update_joystick(int &outX, int &outY);

private:
    uint8_t joyXPin;
    uint8_t joyYPin;
    bool debug_enabled;
};
