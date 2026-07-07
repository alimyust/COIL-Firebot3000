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

    scheduler.addPeriodicTask(100, EventPriority::PRIORITY_HIGH, ControllerHandler::onJoystickUpdate, &handler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_BATTERY, EventPriority::PRIORITY_LOW, ControllerHandler::onBatteryReceived, &handler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_HEARTBEAT, EventPriority::PRIORITY_LOW, ControllerHandler::onHeartbeatReceived, &handler);
}

void loop() {
    radio.update();
    scheduler.update();
}