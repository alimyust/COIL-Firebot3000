#include <Arduino.h>
#include "ProtocolCommands.h"

#include "radio.h"
#include "MotorDriver.h"
#include "Microphone.hpp"
#include "Speaker.hpp"
#include "sensor.h"
#include "CoSensor.h"

#include "handlers/scheduler.h"
#include "handlers/RobotHandler.hpp"
#include "handlers/AudioHandler.hpp"


RadioComm radio(1, 434.0, 8, 3, 4); // Node 1 (Robot)
EventScheduler scheduler(radio, false); 

MotorDriver motorDriver; 
Sen66_Sensor sensor;
CoSensor coSensor;
RobotHandler robotHandler(scheduler, motorDriver, sensor, false);

Microphone mic;
Speaker speaker;
AudioHandler audioHandler(scheduler, mic, speaker, false); // Initialize AudioHandler with microphone
// connect to mic

void setup() {
    Serial.begin(115200);
    radio.begin(); 
    motorDriver.init_motor();
    sensor.begin();
    speaker.begin();
    audioHandler.beginTimer();
    // scheduler.registerPacketHandler(ProtocolCommands::CMD_MOTOR, EventPriority::PRIORITY_HIGH, RobotHandler::onMotorReceived, &robotHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_AUDIO, EventPriority::PRIORITY_CRITICAL, AudioHandler::onAudioPacketReceived, &audioHandler);
    // scheduler.registerPacketHandler(ProtocolCommands::CMD_HB, EventPriority::PRIORITY_LOW, RobotHandler::onHeartbeatReceived, &robotHandler);
    // scheduler.addPeriodicTask(1000, EventPriority::PRIORITY_MEDIUM, RobotHandler::onSensorUpdate, &robotHandler);

}
void loop() {
    radio.update();
    scheduler.update();

}