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


RadioComm radio(1, 915.0, 8, 3, 4); // Node 1 (Robot)
EventScheduler scheduler(radio, true); 
MotorDriver motorDriver; 
Sen66_Sensor sensor;
CoSensor coSensor;
RobotHandler robotHandler(scheduler, motorDriver, sensor, coSensor, true);

Microphone mic;
Speaker speaker;
AudioHandler audioHandler(scheduler, mic, speaker, false); // Initialize AudioHandler with microphone
// connect to mic

void setup() {
    Serial.begin(115200);
    // while(!Serial);
    pinMode(10, OUTPUT);
    radio.begin(); 
    motorDriver.init_motor();
    // sensor.begin();
    // coSensor.begin();

    scheduler.registerPacketHandler(
        ProtocolCommands::CMD_MOTOR,
        EventPriority::PRIORITY_HIGH,
        RobotHandler::onMotorReceived,
        &robotHandler
    );
    
    scheduler.registerPacketHandler(
        ProtocolCommands::CMD_HB,
        EventPriority::PRIORITY_LOW,
        RobotHandler::onHeartbeatReceived,
        &robotHandler
    );

    scheduler.addPeriodicTask(
        1000,
        EventPriority::PRIORITY_MEDIUM,
        RobotHandler::onSensorUpdate,
        &robotHandler);

}
void loop() {
    radio.update();
    scheduler.update();
}