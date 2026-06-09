
#include <Arduino.h>
#include "Joystick.hpp"


class Joystick {
public:
    Joystick()
    void init_joystick();
    void update_joystick();


private:

    uint8_t joyXPin;
    uint8_t joyYPin;
}
