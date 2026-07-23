#include "ControllerHandler.hpp"
#include <stdlib.h>

ControllerHandler::ControllerHandler(EventScheduler &scheduler, Joystick &joystick, DisplayOLED &oled, bool debug_enabled)
    : _scheduler(scheduler), _joystick(joystick),_oled(oled), _debug_enabled(debug_enabled), _lastThrottleDuty(0), _lastSteeringDuty(0) {}

void ControllerHandler::onJoystickTrigger() {
    int x = 0, y = 0;
    _joystick.update_joystick(x, y);

    _lastSteeringDuty = static_cast<uint8_t>(x);
    _lastThrottleDuty = static_cast<uint8_t>(y);

    sendControlValues(_lastThrottleDuty, _lastSteeringDuty);

    if (_debug_enabled) {
        Serial.print("joyX:");
        Serial.print(_lastSteeringDuty);
        Serial.print(" joyY:");
        Serial.println(_lastThrottleDuty);
    }
}

void ControllerHandler::sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty) {

    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_THROTTLE, &throttleDuty, sizeof(throttleDuty));
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_STEERING, &steeringDuty, sizeof(steeringDuty));
}

void ControllerHandler::onOLEDTrigger(){
    // Serial.println("OLED triggered");
    _oled.update();
}

void ControllerHandler::onHeartbeatTrigger() {
    uint32_t timestamp = millis();
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_HB, &timestamp, sizeof(timestamp));
    _oled.display.clearDisplay();
    // 2. Configure font settings (required for clean rendering)
    _oled.display.setTextSize(1);              // Normal 1:1 pixel scale (6x8 px per char)
    // _oled.display.setTextColor(SH110X_WHITE);  // Draw white text on black background
    _oled.display.setTextWrap(false);          // Prevent unintended wrapping
    _oled.display.setCursor(0,0);
    _oled.display.println("Time: ");
    _oled.display.println(timestamp);
    // int bufferSize = 1024; 
    // for (int i = 0; i < bufferSize; i++) {
    //     Serial.print(_oled.display.getBuffer()[i]);
    //     Serial.print(" "); // Adds a space between elements for readability
    // }

    if (_debug_enabled) {
        // Serial.print("Heartbeat sent at: ");
        // Serial.println(timestamp);
    }
}
