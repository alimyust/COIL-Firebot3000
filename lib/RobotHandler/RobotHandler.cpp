#include "RobotHandler.hpp"
#include "Arduino.h"
#include "DebugLog.hpp"
#include <string.h>

static float mapAroundNeutral(uint8_t value,
                              uint8_t in_min, uint8_t in_center, uint8_t in_max,
                              float out_min, float out_neutral, float out_max) {
    if (value <= in_center) {
        float t = float(value - in_min) / float(in_center - in_min);
        return out_min + t * (out_neutral - out_min);
    } else {
        float t = float(value - in_center) / float(in_max - in_center);
        return out_neutral + t * (out_max - out_neutral);
    }
}

void RobotHandler::update() {
    if (!_hasData) {
        return;
    }

    if (_lastMessage[0] != '\0') {
        DebugLog::appendField("msg", _lastMessage);
    }

    DebugLog::appendField("T", _lastThrottleDuty);
    DebugLog::appendField("TM", _lastThrottleMap);
    DebugLog::appendField("S", _lastSteeringDuty);
    DebugLog::appendField("SM", _lastSteeringMap);

    DebugLog::flush();
}

void RobotHandler::onThrottle(uint8_t duty) {
    _motor_driver.setThrottle(duty);
    _lastThrottleDuty = duty;
    _lastThrottleMap = mapAroundNeutral(duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::THROTTLE_MIN,
        MotorConfig::NEUTRAL_THROTTLE,
        MotorConfig::THROTTLE_MAX);
    _hasData = true;
}

void RobotHandler::onSteering(uint8_t duty) {
    _motor_driver.setSteeringDuty(duty);
    _lastSteeringDuty = duty;
    _lastSteeringMap = mapAroundNeutral(duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::STEERING_MIN,
        MotorConfig::NEUTRAL_STEERING,
        MotorConfig::STEERING_MAX);
    _hasData = true;
}

void RobotHandler::onMessage(const char *message) {
    if (message != nullptr) {
        strncpy(_lastMessage, message, sizeof(_lastMessage) - 1);
        _lastMessage[sizeof(_lastMessage) - 1] = '\0';
        _hasData = true;
    }
}
  