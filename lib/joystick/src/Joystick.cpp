#include "Joystick.hpp"

void Joystick::init_joystick() {
    pinMode(joyXPin, INPUT);
    pinMode(joyYPin, INPUT);
}

void Joystick::update_joystick(int &outX, int &outY) {
    constexpr int CENTER = 0; // keep range (0-1024)
    constexpr int DEADZONE = 50;

    int x = analogRead(joyXPin) - CENTER;
    int y = analogRead(joyYPin) - CENTER;

    if (abs(x) < DEADZONE) x = 0;
    if (abs(y) < DEADZONE) y = 0;

    outX = x;
    outY = y;

    if (debug_enabled) {
        Serial.print("Joystick X=");
        Serial.print(outX);
        Serial.print(" Y=");
        Serial.println(outY);
    }
}
