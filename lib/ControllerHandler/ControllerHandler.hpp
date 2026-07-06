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
    void handlePeriodicUpdate();

    // Inbound Callback Targets called by the scheduler
    void onBatteryLevel(const RadioComm::RF69_Packet& packet);
    void onHeartbeat(const RadioComm::RF69_Packet& packet);

    // ========================================================================
    // SCHEDULER INTERFACE ROUTING STATIC BRIDGES
    // ========================================================================
    static void onTimerTick(void* context) {
        static_cast<ControllerHandler*>(context)->handlePeriodicUpdate();
    }

    static void onBatteryReceived(const RadioComm::RF69_Packet& packet, void* context) {
        static_cast<ControllerHandler*>(context)->onBatteryLevel(packet);
    }

    static void onHeartbeatReceived(const RadioComm::RF69_Packet& packet, void* context) {
        static_cast<ControllerHandler*>(context)->onHeartbeat(packet);
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