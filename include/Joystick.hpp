#pragma once

#include <Arduino.h>


class Joystick {
public:
    Joystick(uint8_t joyXPin = A3, uint8_t joyYPin = A2, bool debug = false)
        : joyXPin(joyXPin),
          joyYPin(joyYPin),
          debug_enabled(debug) {}

    void init_joystick();
    void update_joystick(int &outX, int &outY);

    void enableDebug(bool enabled) { debug_enabled = enabled; }

private:
    uint8_t joyXPin;
    uint8_t joyYPin;
    bool debug_enabled;
};
