#ifndef ROBOT_HANDLER_HPP
#define ROBOT_HANDLER_HPP

#include "scheduler.h"
#include "MotorDriver.h"
#include "ProtocolCommands.hpp"

class RobotHandler {
public:
    RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, bool debug);

    // Business logic processing routines called by the scheduler bridges
    void processThrottle(const ProtocolCommands::ThrottlePayload& payload);
    void processSteering(const ProtocolCommands::SteeringPayload& payload);
    void handleDiagnostics();

    // ========================================================================
    // UNIVERSAL SCHEDULER STATIC ROUTING BRIDGES
    // ========================================================================
    static void onThrottleReceived(const RadioComm::RF69_Packet& packet, void* context) {
        ProtocolCommands::ThrottlePayload payload;
        if (ProtocolCommands::deserializeThrottlePayload(packet, payload)) {
            static_cast<RobotHandler*>(context)->processThrottle(payload);
        }
    }

    static void onSteeringReceived(const RadioComm::RF69_Packet& packet, void* context) {
        ProtocolCommands::SteeringPayload payload;
        if (ProtocolCommands::deserializeSteeringPayload(packet, payload)) {
            static_cast<RobotHandler*>(context)->processSteering(payload);
        }
    }

    static void onDiagnosticTimerTick(void* context) {
        static_cast<RobotHandler*>(context)->handleDiagnostics();
    }

private:
    // Preserved exact original mathematical mapping function
    static float mapAroundNeutral(uint8_t value,
                                  uint8_t in_min, uint8_t in_center, uint8_t in_max,
                                  float out_min, float out_neutral, float out_max);

    void updateDebugMessage(const char* payload);

    EventScheduler &_scheduler;
    MotorDriver &_motor_driver;
    bool _debug;

    // Fully encapsulated internal system states
    uint8_t _lastThrottleDuty;
    float _lastThrottleMap;
    uint8_t _lastSteeringDuty;
    float _lastSteeringMap;

    char _lastMessage[32]; // Adjusted boundary size for string safety
    bool _hasData;
};

#endif // ROBOT_HANDLER_HPP