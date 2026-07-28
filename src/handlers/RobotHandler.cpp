#include "RobotHandler.hpp"

#include <stdio.h>
#include <string.h>

RobotHandler::RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, Sen66_Sensor &sensor, bool debug)
    : _scheduler(scheduler),
      _motor_driver(motor_driver),
      _sensor(sensor),
      _debug(debug) {}

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

void RobotHandler::processMotor(const ProtocolCommands::MotorPayload& payload){
    _motor_driver.set_PWM_1(payload.steer_duty);
    _motor_driver.set_PWM_2(payload.throttle_duty);
    _motor_driver.set_PWM_3(payload.steer_duty);
    _motor_driver.set_PWM_4(payload.steer_duty);
}

void RobotHandler::onSensorTrigger(){
    ProtocolCommands::SensorPayload sen66_payload = {};
    _sensor.readData(
        sen66_payload.co2, 
        sen66_payload.vocIndex, 
        sen66_payload.temperature, 
        sen66_payload.humidity, 
        sen66_payload.pm1p0, 
        sen66_payload.pm2p5, 
        sen66_payload.pm4p0,
        sen66_payload.pm10p0,
        sen66_payload.noxIndex
    );

    _scheduler.sendPacket(ProtocolCommands::NODE_CONTROLLER, ProtocolCommands::CMD_SENSORS, &sen66_payload, sizeof(sen66_payload));
}

void RobotHandler::processHeartbeat(const ProtocolCommands::HeartbeatPayload& payload) {
    if (_debug) {
        Serial.print("HB received at: ");
        Serial.println(payload.timestamp);  
    }
}