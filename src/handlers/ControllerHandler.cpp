#include "ControllerHandler.hpp"
#include <stdlib.h>

ControllerHandler::ControllerHandler(EventScheduler &scheduler, Joystick &joystick_motor, Joystick &joystick_turret, DisplayOLED &oled,
     bool debug_enabled, uint8_t walkie_mux_pin, uint8_t walkie_state_pin)
    : _scheduler(scheduler), _joystick_motor(joystick_motor),_joystick_turret(joystick_turret),_oled(oled), _debug_enabled(debug_enabled),
     _lastThrottleDuty(0), _lastSteeringDuty(0), _walkie_mux_pin(walkie_mux_pin), _walkie_state_pin(walkie_state_pin) {}

void ControllerHandler::onJoystickTrigger() {
    int steer, throttle, turret_x, turret_y;
    _joystick_motor.update_joystick(steer, throttle);
    _joystick_turret.update_joystick(turret_x, turret_y);

    ProtocolCommands::MotorPayload motor_payload = {throttle, steer, turret_x, turret_y};
    _scheduler.sendPacket(ProtocolCommands::NODE_ROBOT, ProtocolCommands::CMD_MOTOR, &motor_payload, sizeof(motor_payload));
}

void ControllerHandler::onOLEDTrigger(){
    // Serial.println("OLED triggered");
    _oled.update();
}

void ControllerHandler::processSensor(const ProtocolCommands::SensorPayload& payload) {
    // 1. Update internal state only if incoming values are non-zero
    if (payload.co2 > 0)           _last_co2         = payload.co2;
    if (payload.temperature != 0) _last_temperature = payload.temperature;
    if (payload.humidity != 0)    _last_humidity    = payload.humidity;
    if (payload.vocIndex != 0)    _last_vocIndex    = payload.vocIndex;
    if (payload.noxIndex != 0)    _last_noxIndex    = payload.noxIndex;
    if (payload.pm1p0 != 0)       _last_pm1p0       = payload.pm1p0;
    if (payload.pm2p5 != 0)       _last_pm2p5       = payload.pm2p5;
    if (payload.pm4p0 != 0)       _last_pm4p0       = payload.pm4p0;
    if (payload.pm10p0 != 0)      _last_pm10p0      = payload.pm10p0;
    if (payload.coRaw > 0)         _last_coRaw       = payload.coRaw;
    Serial.println("Sensor data updated");
    // 2. Clear display and set styling
    _oled.display.clearDisplay();
    _oled.display.setTextSize(1);       // 6x8 pixels per character
    _oled.display.setTextWrap(false);   // Keep layout aligned

    // Column X positions for a 2-column layout
    const int col1 = 0;
    const int col2 = 64;

    // Line 0: Header
    _oled.display.setCursor(col1, 0);
    _oled.display.print("--- SEN66 DATA ---");

    // Line 1: CO2 & Temp
    _oled.display.setCursor(col1, 10);
    _oled.display.print("CO2:");
    _oled.display.print(_last_co2);

    _oled.display.setCursor(col2, 10);
    _oled.display.print("T:");
    _oled.display.print(_last_temperature, 1);
    _oled.display.print("C");

    // Line 2: Humidity & VOC Index
    _oled.display.setCursor(col1, 20);
    _oled.display.print("RH :");
    _oled.display.print(_last_humidity, 1);
    _oled.display.print("%");

    _oled.display.setCursor(col2, 20);
    _oled.display.print("VOC:");
    _oled.display.print(_last_vocIndex, 0);

    // Line 3: NOx Index & PM1.0
    _oled.display.setCursor(col1, 30);
    _oled.display.print("NOx:");
    _oled.display.print(_last_noxIndex, 0);

    _oled.display.setCursor(col2, 30);
    _oled.display.print("P1.0:");
    _oled.display.print(_last_pm1p0, 1);

    // Line 4: PM2.5 & PM4.0
    _oled.display.setCursor(col1, 40);
    _oled.display.print("P2.5:");
    _oled.display.print(_last_pm2p5, 1);

    _oled.display.setCursor(col2, 40);
    _oled.display.print("P4.0:");
    _oled.display.print(_last_pm4p0, 1);

    // Line 5: PM10.0 & CO Raw
    _oled.display.setCursor(col1, 50);
    _oled.display.print("P10 :");
    _oled.display.print(_last_pm10p0, 1);

    _oled.display.setCursor(col2, 50);
    _oled.display.print("CO :");
    _oled.display.print(_last_coRaw, 1);

    // Push frame buffer to display
    _oled.pushFrame();
}

void ControllerHandler::onMuxTrigger() {
    bool light_mux, walkie_mux, camera_mux;
    _joystick_motor.update_switch(camera_mux);
    _joystick_turret.update_switch(light_mux);
    walkie_mux = (digitalRead(_walkie_state_pin) == LOW);

    digitalWrite(_walkie_mux_pin, walkie_mux ? HIGH : LOW ); // Control the transistor for walkie mux

    ProtocolCommands::MuxPayload mux_payload = {light_mux, walkie_mux, camera_mux};

    _scheduler.sendPacket(ProtocolCommands::NODE_ROBOT, ProtocolCommands::CMD_MUX, &mux_payload, sizeof(mux_payload));

    if (_debug_enabled) {
        Serial.print("Mux Payload sent - Light: ");
        Serial.print(light_mux);
        Serial.print(", Camera: ");
        Serial.println(camera_mux);
        Serial.print("Walkie: ");
        Serial.println(walkie_mux);
    }
}

void ControllerHandler::onHeartbeatTrigger() {
    uint32_t timestamp = millis();
    _scheduler.sendPacket(ProtocolCommands::NODE_ROBOT, ProtocolCommands::CMD_HB, &timestamp, sizeof(timestamp));

    if (_debug_enabled) {
        Serial.print("Heartbeat sent at: ");
        Serial.println(timestamp);
    }
}
