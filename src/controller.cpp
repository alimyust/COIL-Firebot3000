#include <Arduino.h>

#include "scheduler.h"
#include "ControllerHandler.hpp"

#include "radio.h"

#include "Joystick.hpp"
#include "ProtocolCommands.hpp"

#include "Microphone.hpp"
#include "AudioHandler.hpp"
#include "Speaker.hpp"

RadioComm radio(2, 434.0, 8, 3, 4); // Node 2 (Controller)
EventScheduler scheduler(radio, false);
Joystick joystick;
Microphone mic;
Speaker speaker;
ControllerHandler handler(scheduler, joystick, false);
AudioHandler audioHandler(scheduler, mic,speaker, true);

void setup() {

    Serial.begin(115200);
    radio.begin();
    mic.begin();
    // speaker.begin();
    Serial.println("Controller Setup Complete");
    // scheduler.addPeriodicTask(20, EventPriority::PRIORITY_MEDIUM, ControllerHandler::onJoystickUpdate, &handler);
    scheduler.addPeriodicTask(1, EventPriority::PRIORITY_CRITICAL, AudioHandler::onAudioUpdate, &audioHandler); 
    scheduler.addPeriodicTask(1000, EventPriority::PRIORITY_CRITICAL, ControllerHandler::onHeartbeat, &handler);
}
void loop() {
    radio.update();
    scheduler.update();
    // Serial.println("Controller Loop Running");
}