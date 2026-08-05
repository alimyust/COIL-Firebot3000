#include "Joystick.hpp"

void Joystick::init_joystick() {
    pinMode(joyXPin, INPUT);
    pinMode(joyYPin, INPUT);
    pinMode(joySwitchPin, INPUT);
    Serial.print("Init Joy-> ");
    Serial.print(" X: "); Serial.print(joyXPin);
    Serial.print(" Y: "); Serial.print(joyYPin);
    Serial.print(" Switch: "); Serial.println(joySwitchPin);
}
// outX JoyX, outY JoyY
void Joystick::update_joystick(int &outX, int &outY, bool &outZ) {


    int x = analogRead(joyXPin);
    int y = analogRead(joyYPin);
    bool z = digitalRead(joySwitchPin);

    if (abs(x - JoystickConfig::JOY_CENTER) < JoystickConfig::DEADZONE) x = 0;
    if (abs(y - JoystickConfig::JOY_CENTER) < JoystickConfig::DEADZONE) y = 0;

    outX = map(x, 0, 1024, JoystickConfig::JOY_MIN, JoystickConfig::JOY_MAX);
    outY = map(y, 0, 1024, JoystickConfig::JOY_MIN, JoystickConfig::JOY_MAX);
    outZ = z;
}
