
#include "CommandHandler.hpp"

CommandHandler::CommandHandler(DualHardwarePWM &motor_pwm)
    : _motor_pwm(motor_pwm) {}

void CommandHandler::initialize(uint32_t frequency) {
    _motor_pwm.begin(frequency);
}

void CommandHandler::setMotorSpeed(uint8_t speed) {
    _motor_pwm.setDutyCycle1(speed);
    Serial.print("Motor speed set to: ");
    Serial.println(speed);
}

void CommandHandler::setSteeringAngle(uint8_t angle) {
    _motor_pwm.setDutyCycle2(angle);
    Serial.print("Steering angle set to: ");
    Serial.println(angle);
}
