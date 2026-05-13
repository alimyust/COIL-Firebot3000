
#pragma once

#include <Arduino.h>
#include "DualHWPwm.hpp"

// How the robot actually does things

class CommandHandler {
public:
    explicit CommandHandler(DualHardwarePWM &motor_pwm);

    void initialize(uint32_t frequency = 60);
    void setMotorSpeed(uint8_t speed);
    void setSteeringAngle(uint8_t angle);

private:
    DualHardwarePWM &_motor_pwm;
};