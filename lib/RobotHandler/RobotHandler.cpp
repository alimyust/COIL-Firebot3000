#include "RobotHandler.hpp"
#include "DebugLog.hpp"
#include "MotorDriver.h" 
#include "DebugLog.hpp"


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

void RobotHandler::processThrottle(const RadioComm::RF69_Packet& packet) {
    // Stage 3 converts raw incoming string payload back to programmatic type
    uint8_t duty = static_cast<uint8_t>(atoi(packet.payload));

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

void RobotHandler::processSteering(const RadioComm::RF69_Packet& packet) {
    // Stage 3 converts raw incoming string payload back to programmatic type
    uint8_t duty = static_cast<uint8_t>(atoi(packet.payload));

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

void RobotHandler::processMessage(const RadioComm::RF69_Packet& packet) {
    if (packet.payload != nullptr) {
        strncpy(_lastMessage, packet.payload, sizeof(_lastMessage) - 1);
        _lastMessage[sizeof(_lastMessage) - 1] = '\0';
        _hasData = true;
    }
}

/**
 * @brief Replaces the old continuous update() loop. 
 * Executed at a fixed cadence configured via the Universal Scheduler.
 */
void RobotHandler::handleDiagnostics() {
    if (!_hasData || !_debug) {
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