
#include "CommandHandler.hpp"

CommandHandler::CommandHandler(DualHardwarePWM &motor_pwm): _motor_pwm(motor_pwm) {}

void CommandHandler::initialize(uint32_t frequency) {
    _motor_pwm.begin(frequency);
}


void CommandHandler::setThrottleDuty(uint8_t duty) {
    _motor_pwm.setDutyCycle1(duty);
    Serial.print("Throttle duty set to: ");
    Serial.println(duty);
}

void CommandHandler::setSteeringDuty(uint8_t duty) {
    _motor_pwm.setDutyCycle2(duty);
    Serial.print("Steering duty set to: ");
    Serial.println(duty);
}
