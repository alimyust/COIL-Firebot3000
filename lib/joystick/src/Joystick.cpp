#include "Joystick.hpp"

void Joystick::init_joystick() {
    pinMode(joyXPin, INPUT);
    pinMode(joyYPin, INPUT);
}

void Joystick::update_joystick(int &outX, int &outY) {


    int x = analogRead(joyXPin) - JoystickConfig::CENTER;
    int y = analogRead(joyYPin) - JoystickConfig::CENTER;

    if (abs(x) < JoystickConfig::DEADZONE) x = 0;
    if (abs(y) < JoystickConfig::DEADZONE) y = 0;

    outX = x;
    outY = y;

    if (debug_enabled) {
        Serial.print("Joystick X=");
        Serial.print(outX);
        Serial.print(" Y=");
        Serial.println(outY);
    }
}
