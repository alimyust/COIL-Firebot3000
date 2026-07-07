#include "RobotHandler.hpp"
#include "MotorDriver.h"

#include <stdio.h>
#include <string.h>

RobotHandler::RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, bool debug)
    : _scheduler(scheduler),
      _motor_driver(motor_driver),
      _debug(debug),
      _lastThrottleDuty(0),
      _lastThrottleMap(0.0f),
      _lastSteeringDuty(0),
      _lastSteeringMap(0.0f),
      _hasData(false) {
    _lastMessage[0] = '\0';
}

float RobotHandler::mapAroundNeutral(uint8_t value,
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

void RobotHandler::updateDebugMessage(const char* payload) {
    if (payload != nullptr) {
        strncpy(_lastMessage, payload, sizeof(_lastMessage) - 1);
        _lastMessage[sizeof(_lastMessage) - 1] = '\0';
        _hasData = true;
    }
}

void RobotHandler::processThrottle(const ProtocolCommands::ThrottlePayload& payload) {
    char debugBuffer[8];
    itoa(payload.duty, debugBuffer, 10);
    updateDebugMessage(debugBuffer);

    _motor_driver.setThrottle(payload.duty);
    _lastThrottleDuty = payload.duty;
    _lastThrottleMap = mapAroundNeutral(payload.duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::THROTTLE_MIN,
        MotorConfig::NEUTRAL_THROTTLE,
        MotorConfig::THROTTLE_MAX);
    _hasData = true;
}

void RobotHandler::processSteering(const ProtocolCommands::SteeringPayload& payload) {
    char debugBuffer[8];
    itoa(payload.duty, debugBuffer, 10);
    updateDebugMessage(debugBuffer);

    _motor_driver.setSteeringDuty(payload.duty);
    _lastSteeringDuty = payload.duty;
    _lastSteeringMap = mapAroundNeutral(payload.duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::STEERING_MIN,
        MotorConfig::NEUTRAL_STEERING,
        MotorConfig::STEERING_MAX);
    _hasData = true;
}

/**
 * @brief Replaces the old continuous update() loop.
 * Executed at a fixed cadence configured via the Universal Scheduler.
 */
void RobotHandler::handleDiagnostics() {
    if (!_hasData || !_debug) {
        return;
    }

    Serial.print("throttle:");
    Serial.print(_lastThrottleDuty);
    Serial.print(" ");
    Serial.print(_lastThrottleMap);
    Serial.print(" steer:");
    Serial.print(_lastSteeringDuty);
    Serial.print(" ");
    Serial.println(_lastSteeringMap);
}