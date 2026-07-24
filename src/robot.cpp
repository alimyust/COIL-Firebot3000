#include <Arduino.h>
#include "ProtocolCommands.h"

#include "handlers/scheduler.h"
#include "handlers/RobotHandler.hpp"
#include "handlers/AudioHandler.hpp"

#include "radio.h"
#include "MotorDriver.h"
#include "Microphone.hpp"
#include "Speaker.hpp"

RadioComm radio(1, 434.0, 8, 3, 4); // Node 1 (Robot)
EventScheduler scheduler(radio, true); 

MotorDriver motorDriver; 
RobotHandler robotHandler(scheduler, motorDriver, false);

Microphone mic;
Speaker speaker;
AudioHandler audioHandler(scheduler, mic, speaker, true); // Initialize AudioHandler with microphone
// connect to mic

void setup() {
    Serial.begin(115200);
    radio.begin(); 
    // motorDriver.init_motor();
    // mic.begin();
    speaker.begin();
    // 1. Map Over-The-Air Commands to their respective parsing handlers
    // scheduler.registerPacketHandler(ProtocolCommands::CMD_THROTTLE, EventPriority::PRIORITY_HIGH, RobotHandler::onThrottleReceived, &robotHandler);
    // scheduler.registerPacketHandler(ProtocolCommands::CMD_STEERING, EventPriority::PRIORITY_HIGH, RobotHandler::onSteeringReceived, &robotHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_AUDIO, EventPriority::PRIORITY_CRITICAL, AudioHandler::onAudioPacketReceived, &audioHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_HB, EventPriority::PRIORITY_MEDIUM, RobotHandler::onHeartbeatReceived, &robotHandler);

}
void loop() {
    radio.update();
    scheduler.update();

}