#include "ControllerHandler.hpp"
#include "Joystick.hpp"
#include <stdlib.h>

ControllerHandler::ControllerHandler(EventScheduler &scheduler, Joystick &joystick, bool debug)
    : _scheduler(scheduler), _joystick(joystick), _debug(debug), _lastThrottleDuty(0), _lastSteeringDuty(0) {}

void ControllerHandler::onJoystickTrigger() {
    int x = 0, y = 0;
    _joystick.update_joystick(x, y);

    _lastSteeringDuty = static_cast<uint8_t>(x);
    _lastThrottleDuty = static_cast<uint8_t>(y);

    sendControlValues(_lastThrottleDuty, _lastSteeringDuty);

    if (_debug) {
        Serial.print("joyX:");
        Serial.print(_lastSteeringDuty);
        Serial.print(" joyY:");
        Serial.println(_lastThrottleDuty);
    }
}

// OUTBOUND: itoa completely removes the need for separate serialize functions
void ControllerHandler::sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty) {
    char tBuf[4], sBuf[4];
    itoa(throttleDuty, tBuf, 10);
    itoa(steeringDuty, sBuf, 10);

    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_THROTTLE, tBuf);
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_STEERING, sBuf);
}

void ControllerHandler::onBatteryLevel(const ProtocolCommands::BatteryPayload& payload) {
    if (_debug) {
        Serial.print("battery:");
        Serial.println(payload.level);
    }
}

void ControllerHandler::onHeartbeat(const ProtocolCommands::HeartbeatPayload&) {
    if (_debug) {
        Serial.print("heartbeat:");
        Serial.println(millis());
    }
}