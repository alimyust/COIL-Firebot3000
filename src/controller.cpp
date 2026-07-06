#include <Arduino.h>
#include "radio.h"
#include "scheduler.h"
#include "ControllerHandler.hpp"
#include "Joystick.hpp"
#include "ProtocolCommands.hpp"

RadioComm radio(2, 434.0, 8, 3, 4); // Node 2 (Controller)
EventScheduler scheduler(radio);
Joystick joystick;

ControllerHandler handler(scheduler, joystick, true);

void setup() {
    Serial.begin(115200);
    radio.begin();

    // 1. Assign 20ms execution loops to the handler without internal millis checks
    scheduler.addPeriodicTask(100, EventPriority::PRIORITY_HIGH, ControllerHandler::onTimerTick, &handler);

    // 2. Map incoming payload commands (e.g., Battery = 0x04, Heartbeat = 0x05)
    scheduler.registerPacketHandler(ProtocolCommands::CMD_HEARTBEAT, EventPriority::PRIORITY_LOW, ControllerHandler::onHeartbeatReceived, &handler);
}

void loop() {
    radio.update();
    scheduler.update();
}