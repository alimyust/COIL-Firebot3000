#include <Arduino.h>

#include "handlers/scheduler.h"
#include "handlers/ControllerHandler.hpp"
#include "handlers/AudioHandler.hpp"

#include "radio.h"

#include "Joystick.hpp"
#include "ProtocolCommands.h"
#include "Microphone.hpp"
#include "Speaker.hpp"

#include "display.h"

RadioComm radio(2, 434.0, 8, 3, 4); // Node 2 (Controller)
EventScheduler scheduler(radio, true);
Joystick joystick_motor;
Joystick joystick_turret;
Microphone mic;
Speaker speaker;
DisplayOLED oled(false);

ControllerHandler controller_handler(scheduler, joystick_motor, joystick_turret, oled, false);
AudioHandler audioHandler(scheduler, mic,speaker, false);

void setup() {

    Serial.begin(115200);
    while(!Serial);

    radio.begin();
    // joystick_motor.init_joystick();
    // joystick_turret.init_joystick();
    // oled.begin();
    mic.begin();

    // scheduler.addPeriodicTask(500, EventPriority::PRIORITY_MEDIUM, ControllerHandler::onJoystickUpdate, &controller_handler);
    // scheduler.addPeriodicTask(1, EventPriority::PRIORITY_HIGH, AudioHandler::onAudioUpdate, &audioHandler); 
    // scheduler.addPeriodicTask(1000, EventPriority::PRIORITY_LOW, ControllerHandler::onHeartbeat, &controller_handler);

    // scheduler.registerPacketHandler(ProtocolCommands::CMD_SENSORS, EventPriority::PRIORITY_MEDIUM, ControllerHandler::onSensorReceived, &controller_handler);
    Serial.println("Controller Setup Complete");

}
void loop() {
    radio.update();
    audioHandler.onAudioTrigger();
    scheduler.update();
    // static unsigned long lastPrintTime = 0;
    // constexpr unsigned long PRINT_INTERVAL = 1000;

    // unsigned long currentTime = millis();

    // if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
    //     lastPrintTime = currentTime;
    //     Serial.println("Controller Loop Alive");
    // }

    // oled.update();      
}