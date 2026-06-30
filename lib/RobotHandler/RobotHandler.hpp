#pragma once

#include "ProtocolLayer.hpp"
#include "MotorDriver.h"

class RobotHandler : public ProtocolLayer::ProtocolHandler {
public:
    explicit RobotHandler(MotorDriver &motor_driver)
        : _motor_driver(motor_driver),
          _lastHeartbeatMs(0),
          _lastHeartbeatSentMs(0) {}

    void update();
    void onThrottle(uint8_t duty) override;
    void onSteering(uint8_t duty) override;
    void onHeartbeat() override;

private:
    MotorDriver &_motor_driver;
    unsigned long _lastHeartbeatMs;
    unsigned long _lastHeartbeatSentMs;
};
