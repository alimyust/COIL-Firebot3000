#include "RobotHandler.hpp"
#include "MotorDriver.h"

#include <stdio.h>
#include <string.h>

RobotHandler::RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, bool debug)
    : _scheduler(scheduler),
      _motor_driver(motor_driver),
      _debug(debug),
      _lastThrottleMap(0.0f),
      _lastSteeringMap(0.0f) {}

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

void RobotHandler::processThrottle(const ProtocolCommands::ThrottlePayload& payload) {
    
    _lastThrottleMap = mapAroundNeutral(payload.duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::THROTTLE_MIN,
        MotorConfig::NEUTRAL_THROTTLE,
        MotorConfig::THROTTLE_MAX);

    _motor_driver.setThrottle(_lastThrottleMap);
}

void RobotHandler::processSteering(const ProtocolCommands::SteeringPayload& payload) {

    _lastSteeringMap = mapAroundNeutral(payload.duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::STEERING_MIN,
        MotorConfig::NEUTRAL_STEERING,
        MotorConfig::STEERING_MAX);

    _motor_driver.setSteeringDuty(_lastSteeringMap);

}

void RobotHandler::processHeartbeat(const ProtocolCommands::HeartbeatPayload& payload) {
    if (_debug) {
        Serial.print("HB received at: ");
        Serial.println(payload.timestamp);  
    }
}