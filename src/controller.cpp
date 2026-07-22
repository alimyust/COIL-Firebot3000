#include <Arduino.h>

#include "scheduler.h"
#include "ControllerHandler.hpp"

#include "radio.h"

#include "Joystick.hpp"
#include "ProtocolCommands.hpp"
#include <iostream>  // For C++ style cout

#include "Microphone.hpp"
#include "AudioHandler.hpp"
#include "Speaker.hpp"

#include "display.h"

RadioComm radio(2, 434.0, 8, 3, 4); // Node 2 (Controller)
EventScheduler scheduler(radio, false);
Joystick joystick;
Microphone mic;
Speaker speaker;
DisplayOLED oled(true);

ControllerHandler handler(scheduler, joystick, oled, true);
AudioHandler audioHandler(scheduler, mic,speaker, true);

void setup() {

    Serial.begin(115200);
    while(!Serial);

    radio.begin();
    // mic.begin();
    oled.begin();



  // Clear the buffer.
    // oled.display.display();

    // scheduler.addPeriodicTask(20, EventPriority::PRIORITY_MEDIUM, ControllerHandler::onJoystickUpdate, &handler);
    // scheduler.addPeriodicTask(1, EventPriority::PRIORITY_HIGH, AudioHandler::onAudioUpdate, &audioHandler); 
    scheduler.addPeriodicTask(100, EventPriority::PRIORITY_HIGH,
         ControllerHandler::onHeartbeat, &handler);
    scheduler.addPeriodicTask(100, EventPriority::PRIORITY_HIGH,
         ControllerHandler::onOLED, &handler);
        // int bufferSize = 1024; 
        // for (int i = 0; i < bufferSize; i++) {
        //     Serial.print(oled.display.getBuffer()[i]);
        //     Serial.print(" "); // Adds a space between elements for readability
        // }
    Serial.println("Controller Setup Complete");

}
void loop() {
    radio.update();
    scheduler.update();
    // oled.display.clearDisplay();
    // oled.display.setCursor(0,2);
    // oled.display.println("Please wors");
    // oled.pushFrame();
    // oled.update();
    // oled.display.display();

    // oled.display.println("weeahiods");
    // oled.pushFrame();
    // oled.update()
    // oled.pushFrame();
    // oled.update();

    // int bufferSize = 1024; 
    // for (int i = 0; i < bufferSize; i++) {
    //     Serial.print(oled.display.getBuffer()[i]);
    //     Serial.print(" "); // Adds a space between elements for readability
    // }    
    // Serial.println("Controller Loop Running");
}