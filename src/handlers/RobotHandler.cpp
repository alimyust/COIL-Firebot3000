#include "RobotHandler.hpp"

#include <stdio.h>
#include <string.h>

RobotHandler::RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, Sen66_Sensor &sensor, CoSensor &coSensor, bool debug,
                           uint8_t light_mux_pin, uint8_t walkie_mux_pin, uint8_t camera_mux_pin,  bool last_light_mux_state, bool last_walkie_mux_state, bool last_camera_mux_state)
    : _scheduler(scheduler),
      _motor_driver(motor_driver),
      _sensor(sensor),
      _coSensor(coSensor),
      _debug(debug),
      _light_mux_pin(light_mux_pin),
      _walkie_mux_pin(walkie_mux_pin),
      _camera_mux_pin(camera_mux_pin){}

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
    _motor_driver.setSteerPWM(payload.throttle_duty);
    _motor_driver.setThrottlePWM(payload.steer_duty);
    _motor_driver.setTurretPan(payload.turret_x_duty);
    _motor_driver.setTurretTilt(payload.turret_y_duty);
    Serial.println("Process motor");

}
void RobotHandler::processMux(const ProtocolCommands::MuxPayload& payload) {
    // --- LIGHT MUX TOGGLE ---
    // Detect rising edge: button was off previously, now pressed
    if (payload.light_mux && !_prev_light_button) {
        _light_toggle_state = !_light_toggle_state; // Flip toggle state
        digitalWrite(_light_mux_pin, _light_toggle_state ? HIGH : LOW);
    }
    _prev_light_button = payload.light_mux;

    // --- WALKIE MUX TOGGLE ---
    if (payload.walkie_mux != _prev_walkie_button) {
        digitalWrite(_walkie_mux_pin, payload.walkie_mux ? HIGH : LOW);
        Serial.println(payload.walkie_mux ? "Walkie Mux ON" : "Walkie Mux OFF");
    }
    _prev_walkie_button = payload.walkie_mux;

    // --- CAMERA MUX TOGGLE ---
    if (payload.camera_mux && !_prev_camera_button) {
        _camera_toggle_state = !_camera_toggle_state;
        _motor_driver.setCameraMux(_camera_toggle_state);
    }
    _prev_camera_button = payload.camera_mux;

    // --- DEBUG LOGGING ---
    if (_debug) {
        Serial.print("Light mux toggle: ");
        Serial.println(_light_toggle_state);
        Serial.print("Walkie mux toggle: ");
        Serial.println(payload.walkie_mux ? HIGH : LOW);
        Serial.print("Camera mux toggle: ");
        Serial.println(_camera_toggle_state);
    }
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

    // sen66_payload.coRaw = _coSensor.readRaw();
        // Fake fixed sensor data for transmission testing without real sensor hardware
    // sen66_payload.co2 = 415;           // ppm
    // sen66_payload.vocIndex = 60;       // arbitrary index
    // sen66_payload.temperature = 22.5f; // degrees Celsius
    // sen66_payload.humidity = 45.0f;    // percent
    // sen66_payload.pm1p0 = 8;           // ug/m3
    // sen66_payload.pm2p5 = 12;          // ug/m3
    // sen66_payload.pm4p0 = 18;          // ug/m3
    // sen66_payload.pm10p0 = 25;         // ug/m3
    // sen66_payload.noxIndex = 30;       // arbitrary index
    // sen66_payload.coRaw = 1234;        // raw CO sensor value

    _scheduler.sendPacket(ProtocolCommands::NODE_CONTROLLER, ProtocolCommands::CMD_SENSORS, &sen66_payload, sizeof(sen66_payload));
}

void RobotHandler::processHeartbeat(const ProtocolCommands::HeartbeatPayload& payload) {
    if (_debug) {
        Serial.print("HB received at: ");
        Serial.println(payload.timestamp);  
    }
}