#include "RobotHandler.hpp"

#include <stdio.h>
#include <string.h>

RobotHandler::RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, Sen66_Sensor &sensor, CoSensor &coSensor, bool debug)
    : _scheduler(scheduler),
      _motor_driver(motor_driver),
      _sensor(sensor),
      _coSensor(coSensor),
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
    _motor_driver.setSteerPWM(payload.steer_duty);
    _motor_driver.setThrottlePWM(payload.throttle_duty);
    _motor_driver.setTurretPan(payload.turret_x_duty);
    _motor_driver.setTurretTilt(payload.turret_y_duty);
    _motor_driver.setCameraMux(payload.camera_mux);
    digitalWrite(10, payload.light_mux);
    if(_debug) Serial.println("Light mux set to: " + String(payload.light_mux));
}

void RobotHandler::onSensorTrigger(){
    ProtocolCommands::SensorPayload sen66_payload = {};
    // _sensor.readData(
    //     sen66_payload.co2, 
    //     sen66_payload.vocIndex, 
    //     sen66_payload.temperature, 
    //     sen66_payload.humidity, 
    //     sen66_payload.pm1p0, 
    //     sen66_payload.pm2p5, 
    //     sen66_payload.pm4p0,
    //     sen66_payload.pm10p0,
    //     sen66_payload.noxIndex
    // );

    // sen66_payload.coRaw = _coSensor.readRaw();
        // Fake fixed sensor data for transmission testing without real sensor hardware
    sen66_payload.co2 = 415;           // ppm
    sen66_payload.vocIndex = 60;       // arbitrary index
    sen66_payload.temperature = 22.5f; // degrees Celsius
    sen66_payload.humidity = 45.0f;    // percent
    sen66_payload.pm1p0 = 8;           // ug/m3
    sen66_payload.pm2p5 = 12;          // ug/m3
    sen66_payload.pm4p0 = 18;          // ug/m3
    sen66_payload.pm10p0 = 25;         // ug/m3
    sen66_payload.noxIndex = 30;       // arbitrary index
    sen66_payload.coRaw = 1234;        // raw CO sensor value

    _scheduler.sendPacket(ProtocolCommands::NODE_CONTROLLER, ProtocolCommands::CMD_SENSORS, &sen66_payload, sizeof(sen66_payload));
}

void RobotHandler::processHeartbeat(const ProtocolCommands::HeartbeatPayload& payload) {
    if (_debug) {
        Serial.print("HB received at: ");
        Serial.println(payload.timestamp);  
    }
}