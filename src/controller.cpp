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

RadioComm radio(2, 915.0, 8, 3, 4); // Node 2 (Controller)
EventScheduler scheduler(radio, true);
Joystick joystick_motor(A2, A3);
Joystick joystick_turret(A4, A5);
Microphone mic;
Speaker speaker;
DisplayOLED oled(true);

ControllerHandler controller_handler(scheduler, joystick_motor, joystick_turret, oled, true);
AudioHandler audioHandler(scheduler, mic,speaker, false);

void setup() {

    Serial.begin(115200);
    while(!Serial);

    radio.begin();
    joystick_motor.init_joystick();
    joystick_turret.init_joystick();
    // oled.begin();
    // mic.begin();

    scheduler.addPeriodicTask(500, EventPriority::PRIORITY_MEDIUM, ControllerHandler::onJoystickUpdate, &controller_handler);
    // scheduler.addPeriodicTask(3, EventPriority::PRIORITY_HIGH, AudioHandler::onAudioUpdate, &audioHandler); 
    // scheduler.addPeriodicTask(1000, EventPriority::PRIORITY_LOW, ControllerHandler::onHeartbeat, &controller_handler);

    // scheduler.registerPacketHandler(ProtocolCommands::CMD_SENSORS, EventPriority::PRIORITY_MEDIUM, ControllerHandler::onSensorReceived, &controller_handler);
    Serial.println("Controller Setup Complete");

}
void loop() {
    radio.update();
    scheduler.update();
    // oled.update();      
}