#include "ControllerHandler.hpp"
#include "Joystick.hpp"
#include <stdlib.h>

ControllerHandler::ControllerHandler(EventScheduler &scheduler, Joystick &joystick, bool debug_enabled)
    : _scheduler(scheduler), _joystick(joystick), _debug_enabled(debug_enabled), _lastThrottleDuty(0), _lastSteeringDuty(0) {}

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
