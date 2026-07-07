#include <Arduino.h>
#include "radio.h"
#include "scheduler.h"
#include "RobotHandler.hpp"
#include "MotorDriver.h"
#include "ProtocolCommands.hpp"

RadioComm radio(1, 434.0, 8, 3, 4); // Node 1 (Robot)
EventScheduler scheduler(radio);

MotorDriver motorDriver; 
RobotHandler robotHandler(scheduler, motorDriver, false);

void setup() {
    Serial.begin(115200);
    radio.begin();
    motorDriver.init_motor();

    // 1. Map Over-The-Air Commands to their respective parsing handlers
    scheduler.registerPacketHandler(ProtocolCommands::CMD_THROTTLE, EventPriority::PRIORITY_HIGH, RobotHandler::onThrottleReceived, &robotHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_STEERING, EventPriority::PRIORITY_HIGH, RobotHandler::onSteeringReceived, &robotHandler);

    // 2. Schedule the diagnostic logging block to update cleanly every 50ms 
    scheduler.addPeriodicTask(50, EventPriority::PRIORITY_LOW, RobotHandler::onDiagnosticTimerTick, &robotHandler);
}

void loop() {
    radio.update();
    scheduler.update();
}