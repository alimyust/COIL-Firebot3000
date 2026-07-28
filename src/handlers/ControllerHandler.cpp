#include "ControllerHandler.hpp"
#include <stdlib.h>

ControllerHandler::ControllerHandler(EventScheduler &scheduler, Joystick &joystick_motor, Joystick &joystick_turret, DisplayOLED &oled, bool debug_enabled)
    : _scheduler(scheduler), _joystick_motor(joystick_motor),_joystick_turret(joystick_turret),_oled(oled), _debug_enabled(debug_enabled), _lastThrottleDuty(0), _lastSteeringDuty(0) {}

void ControllerHandler::onJoystickTrigger() {
    int steer, throttle, turret_x, turret_y;
    _joystick_motor.update_joystick(steer, throttle);
    _joystick_turret.update_joystick(turret_x, turret_y);

    ProtocolCommands::MotorPayload motor_payload = {steer, throttle, turret_x, turret_y};
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

void ControllerHandler::onHeartbeatTrigger() {
    uint32_t timestamp = millis();
    _scheduler.sendPacket(ProtocolCommands::NODE_ROBOT, ProtocolCommands::CMD_HB, &timestamp, sizeof(timestamp));
    _oled.display.clearDisplay();
    // 2. Configure font settings (required for clean rendering)
    _oled.display.setTextSize(1);              // Normal 1:1 pixel scale (6x8 px per char)
    // _oled.display.setTextColor(SH110X_WHITE);  // Draw white text on black background
    _oled.display.setTextWrap(false);          // Prevent unintended wrapping
    _oled.display.setCursor(0,0);
    _oled.display.println("Time: ");
    _oled.display.println(timestamp);
    // int bufferSize = 1024; 
    // for (int i = 0; i < bufferSize; i++) {
    //     Serial.print(_oled.display.getBuffer()[i]);
    //     Serial.print(" "); // Adds a space between elements for readability
    // }

    if (_debug_enabled) {
        // Serial.print("Heartbeat sent at: ");
        // Serial.println(timestamp);
    }
}
