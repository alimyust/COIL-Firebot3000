#include <Arduino.h>
#include "radio.h"
#include "scheduler.h"
#include "RobotHandler.hpp"
#include "MotorDriver.h"
#include "ProtocolCommands.hpp"


#include "Microphone.hpp"
#include "Speaker.hpp"
#include "AudioHandler.hpp"

RadioComm radio(1, 434.0, 8, 3, 4); // Node 1 (Robot)
EventScheduler scheduler(radio, false); 

MotorDriver motorDriver; 
RobotHandler robotHandler(scheduler, motorDriver, false);

Microphone mic;
Speaker speaker;
AudioHandler audioHandler(scheduler, mic, speaker, true); // Initialize AudioHandler with microphone

void setup() {
    Serial.begin(115200);
    radio.begin();
    motorDriver.init_motor();
    
    // 1. Map Over-The-Air Commands to their respective parsing handlers
    scheduler.registerPacketHandler(ProtocolCommands::CMD_THROTTLE, EventPriority::PRIORITY_HIGH, RobotHandler::onThrottleReceived, &robotHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_STEERING, EventPriority::PRIORITY_HIGH, RobotHandler::onSteeringReceived, &robotHandler);
    scheduler.addPeriodicTask(1, EventPriority::PRIORITY_CRITICAL, AudioHandler::onAudioUpdate, &audioHandler);}   
    //Sending side (wire mic to robot)

void loop() {
    radio.update();
    scheduler.update();
}