#ifndef CONTROLLER_HANDLER_HPP
#define CONTROLLER_HANDLER_HPP

#include <Arduino.h>
#include "scheduler.h"

#include "ProtocolCommands.hpp"
#include "Joystick.hpp"
#include "display.h"

class Joystick; // Forward declaration

class ControllerHandler {
public:
    // Controller target definitions (Assuming Robot is Node 1)
    static const uint8_t TARGET_ROBOT_NODE = ProtocolCommands::NODE_ROBOT;
    static const uint8_t CMD_THROTTLE = ProtocolCommands::CMD_THROTTLE;
    static const uint8_t CMD_STEERING = ProtocolCommands::CMD_STEERING;
    static const uint8_t CMD_SENSORS = ProtocolCommands::CMD_SENSORS;
    static const uint8_t CMD_AUDIO = ProtocolCommands::CMD_AUDIO;
    static const uint8_t CMD_HB = ProtocolCommands::CMD_HB;

    ControllerHandler(EventScheduler &scheduler, Joystick &joystick, DisplayOLED &_oled, bool debug_enabled);

    /**
     * @brief Triggered at a fixed time slice by the scheduler.
     * Evaluates hardware pins and directly transmits dependencies.
     */
    void onJoystickTrigger();
    void onHeartbeatTrigger();
    void onOLEDTrigger();
    // ========================================================================
    // SCHEDULER INTERFACE ROUTING STATIC BRIDGES
    // ========================================================================
    static void onJoystickUpdate(void* context) {
        static_cast<ControllerHandler*>(context)->onJoystickTrigger();
    }

    static void onHeartbeat(void* context) {
        static_cast<ControllerHandler*>(context)->onHeartbeatTrigger();
    }

    static void onOLED(void* context){
        static_cast<ControllerHandler*>(context)->onOLEDTrigger();
    }

private:
    void sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty);

    EventScheduler &_scheduler;
    Joystick &_joystick;
    DisplayOLED &_oled;
    bool _debug_enabled;

    uint8_t _lastThrottleDuty;
    uint8_t _lastSteeringDuty;
};

#endif // CONTROLLER_HANDLER_HPP