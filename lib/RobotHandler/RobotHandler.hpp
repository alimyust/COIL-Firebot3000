#pragma once

#include "ProtocolLayer.hpp"
#include "MotorDriver.h"

class RobotHandler : public ProtocolLayer::ProtocolHandler {
public:
    explicit RobotHandler(MotorDriver &motor_driver)
        : _motor_driver(motor_driver),
          _lastThrottleDuty(0),
          _lastThrottleMap(0.0f),
          _lastSteeringDuty(0),
          _lastSteeringMap(0.0f),
          _hasData(false) {
        _lastMessage[0] = '\0';
    }

    void update();
    void onThrottle(uint8_t duty) override;
    void onSteering(uint8_t duty) override;
    void onMessage(const char *message) override;

private:
    MotorDriver &_motor_driver;
    uint8_t _lastThrottleDuty;
    float _lastThrottleMap;
    uint8_t _lastSteeringDuty;
    float _lastSteeringMap;
    char _lastMessage[32];
    bool _hasData;
};
