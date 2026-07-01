#include "ControllerHandler.hpp"
#include "Arduino.h"
#include "DebugLog.hpp"

namespace {
constexpr unsigned long SEND_INTERVAL_MS = 20;
}

ControllerHandler::ControllerHandler(ProtocolLayer &protocol, Joystick &joystick, bool debug)
    : _protocol(protocol),
      _joystick(joystick),
      _debug(debug),
      _lastThrottleDuty(-1),
      _lastSteeringDuty(-1),
      _lastSendTime(0) {}

void ControllerHandler::update() {
    int x = 0;
    int y = 0;
    _joystick.update_joystick(x, y);

    const uint8_t steeringDuty = static_cast<uint8_t>(x);
    const uint8_t throttleDuty = static_cast<uint8_t>(y);

    const unsigned long now = millis();
    if (now - _lastSendTime >= SEND_INTERVAL_MS) {
        sendControlValues(throttleDuty, steeringDuty);
        _lastSteeringDuty = steeringDuty;
        _lastThrottleDuty = throttleDuty;
        _lastSendTime = now;
    }

    if (_debug) {
        DebugLog::appendField("ctrlT", _lastThrottleDuty);
        DebugLog::appendField("ctrlS", _lastSteeringDuty);
    }
}

void ControllerHandler::sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty) {
    _protocol.enqueueThrottle(throttleDuty);
    _protocol.enqueueSteering(steeringDuty);
}

void ControllerHandler::onBatteryLevel(float level) {
    if (_debug) {
        DebugLog::appendField("bat", level);
    }
}

void ControllerHandler::onHeartbeat() {
    if (_debug) {
        DebugLog::appendField("ctrlHB", millis());
    }
}
