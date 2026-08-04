#ifndef CONTROLLER_HANDLER_HPP
#define CONTROLLER_HANDLER_HPP

#include <Arduino.h>
#include "scheduler.h"

#include "ProtocolCommands.h"
#include "Joystick.hpp"
#include "display.h"

class Joystick; // Forward declaration

class ControllerHandler {
public:
    // // Controller target definitions (Assuming Robot is Node 1)
    // static const uint8_t TARGET_ROBOT_NODE = ProtocolCommands::NODE_ROBOT;
    // static const uint8_t CMD_THROTTLE = ProtocolCommands::CMD_THROTTLE;
    // static const uint8_t CMD_STEERING = ProtocolCommands::CMD_STEERING;
    // static const uint8_t CMD_SENSORS = ProtocolCommands::CMD_SENSORS;
    // static const uint8_t CMD_AUDIO = ProtocolCommands::CMD_AUDIO;
    // static const uint8_t CMD_HB = ProtocolCommands::CMD_HB;

    ControllerHandler(EventScheduler &scheduler, Joystick &joystick_motor, Joystick &joystick_turret,
         DisplayOLED &_oled, bool debug_enabled);

    /**
     * @brief Triggered at a fixed time slice by the scheduler.
     * Evaluates hardware pins and directly transmits dependencies.
     */
    void onJoystickTrigger();
    void onHeartbeatTrigger();
    void processSensor(const ProtocolCommands::SensorPayload& payload);
    void onOLEDTrigger();
    // ========================================================================
    // SCHEDULER INTERFACE ROUTING STATIC BRIDGES
    // ========================================================================

  static void onSensorReceived(const RadioComm::RF69_Packet& packet, void* context) {
    if (context == nullptr) return;

    // Local stack variable guarantees proper 4-byte memory alignment
    ProtocolCommands::SensorPayload payload;
    
    // Copy safely out of the RF buffer byte array
    memcpy(&payload, packet.payload, sizeof(payload));

    static_cast<ControllerHandler*>(context)->processSensor(payload);
}

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
    Joystick &_joystick_motor;
    Joystick &_joystick_turret;
    DisplayOLED &_oled;
    bool _debug_enabled;

    uint8_t _lastThrottleDuty;
    uint8_t _lastSteeringDuty;

    float _last_pm1p0       = 0.0f;
    float _last_pm2p5       = 0.0f;
    float _last_pm4p0       = 0.0f;
    float _last_pm10p0      = 0.0f;
    float _last_humidity    = 0.0f;
    float _last_temperature = 0.0f;
    float _last_vocIndex    = 0.0f;
    float _last_noxIndex    = 0.0f;
    uint16_t _last_co2      = 0;
    uint16_t _last_coRaw    = 0;
};

#endif // CONTROLLER_HANDLER_HPP