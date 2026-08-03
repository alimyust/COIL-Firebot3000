#include <Arduino.h>
#include "ProtocolCommands.h"

#include "radio.h"
#include "MotorDriver.h"
#include "Microphone.hpp"
#include "Speaker.hpp"
#include "sensor.h"

#include "handlers/scheduler.h"
#include "handlers/RobotHandler.hpp"
#include "handlers/AudioHandler.hpp"


RadioComm radio(ProtocolCommands::NODE_ROBOT, 915.0, 8, 3, 4); // Node 1 (Robot)
EventScheduler scheduler(radio, true); 

MotorDriver motorDriver; 
Sen66_Sensor sensor;
RobotHandler robotHandler(scheduler, motorDriver, sensor, true);

Microphone mic;
Speaker speaker;
AudioHandler audioHandler(scheduler, mic, speaker, true); // Initialize AudioHandler with microphone
// connect to mic

void setup() {
    Serial.begin(115200);
    // while(!Serial);

    radio.begin(); 
    // motorDriver.init_motor();
    // sensor.begin();
    // audioHandler.beginTimer();
    speaker.begin();
    // scheduler.registerPacketHandler(ProtocolCommands::CMD_MOTOR, EventPriority::PRIORITY_HIGH, RobotHandler::onMotorReceived, &robotHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_AUDIO, EventPriority::PRIORITY_CRITICAL, AudioHandler::onAudioPacketReceived, &audioHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_HB, EventPriority::PRIORITY_LOW, RobotHandler::onHeartbeatReceived, &robotHandler);
    
    // scheduler.addPeriodicTask(1000, EventPriority::PRIORITY_MEDIUM, RobotHandler::onSensorUpdate, &robotHandler);
    Serial.println("Robot Setup Complete");
}
void loop() {
    radio.update();
    scheduler.update();
    // oled.update();      

    // static unsigned long lastPrintTime = 0;
    // constexpr unsigned long PRINT_INTERVAL = 1000;

    // unsigned long currentTime = millis();

    // if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
    //     lastPrintTime = currentTime;
    //     Serial.println("Robot Loop Alive");
    // }

}