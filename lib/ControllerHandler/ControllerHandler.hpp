#ifndef CONTROLLER_HANDLER_HPP
#define CONTROLLER_HANDLER_HPP

#include <Arduino.h>
#include "scheduler.h"
#include "ProtocolCommands.hpp"

class Joystick; // Forward declaration

class ControllerHandler {
public:
    // Controller target definitions (Assuming Robot is Node 1)
    static const uint8_t TARGET_ROBOT_NODE = ProtocolCommands::NODE_ROBOT;
    static const uint8_t CMD_THROTTLE = ProtocolCommands::CMD_THROTTLE;
    static const uint8_t CMD_STEERING = ProtocolCommands::CMD_STEERING;

    ControllerHandler(EventScheduler &scheduler, Joystick &joystick, bool debug);

    /**
     * @brief Triggered at a fixed time slice by the scheduler.
     * Evaluates hardware pins and directly transmits dependencies.
     */
    void onJoystickTrigger();

    // Inbound callback targets called by the scheduler bridges
    void onBatteryLevel(const ProtocolCommands::BatteryPayload& payload);
    void onHeartbeat(const ProtocolCommands::HeartbeatPayload& payload);

    // ========================================================================
    // SCHEDULER INTERFACE ROUTING STATIC BRIDGES
    // ========================================================================
    static void onJoystickUpdate(void* context) {
        static_cast<ControllerHandler*>(context)->onJoystickTrigger();
    }

    static void onBatteryReceived(const RadioComm::RF69_Packet& packet, void* context) {
        ProtocolCommands::BatteryPayload payload;
        if (ProtocolCommands::deserializeBatteryPayload(packet, payload)) {
            static_cast<ControllerHandler*>(context)->onBatteryLevel(payload);
        }
    }

    static void onHeartbeatReceived(const RadioComm::RF69_Packet& packet, void* context) {
        ProtocolCommands::HeartbeatPayload payload;
        if (ProtocolCommands::deserializeHeartbeatPayload(packet, payload)) {
            static_cast<ControllerHandler*>(context)->onHeartbeat(payload);
        }
    }

private:
    void sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty);

    EventScheduler &_scheduler;
    Joystick &_joystick;
    bool _debug;

    uint8_t _lastThrottleDuty;
    uint8_t _lastSteeringDuty;
};

#endif // CONTROLLER_HANDLER_HPP