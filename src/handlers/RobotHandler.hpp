#ifndef ROBOT_HANDLER_HPP
#define ROBOT_HANDLER_HPP

#include "scheduler.h"
#include <MotorDriver.h>                // Pulled from generic lib/
#include "ProtocolCommands.h"

class RobotHandler {
public:
    RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, bool debug);

    // Business logic processing routines called by the scheduler bridges
    void processThrottle(const ProtocolCommands::ThrottlePayload& payload);
    void processSteering(const ProtocolCommands::SteeringPayload& payload);
    void handleDiagnostics();
    void processHeartbeat(const ProtocolCommands::HeartbeatPayload& payload);

    // ========================================================================
    // UNIVERSAL SCHEDULER STATIC ROUTING BRIDGES
    // ========================================================================
    static void onThrottleReceived(const RadioComm::RF69_Packet& packet, void* context) {
        ProtocolCommands::ThrottlePayload payload;
        static_cast<RobotHandler*>(context)->processThrottle(payload);
    }

    static void onSteeringReceived(const RadioComm::RF69_Packet& packet, void* context) {
        ProtocolCommands::SteeringPayload payload;
        static_cast<RobotHandler*>(context)->processSteering(payload);
    }

    static void onHeartbeatReceived(const RadioComm::RF69_Packet& packet, void* context) {
        ProtocolCommands::HeartbeatPayload payload;
        static_cast<RobotHandler*>(context)->processHeartbeat(payload);
    }
private:
    // Preserved exact original mathematical mapping function
    static float mapAroundNeutral(uint8_t value,
                                  uint8_t in_min, uint8_t in_center, uint8_t in_max,
                                  float out_min, float out_neutral, float out_max);

    EventScheduler &_scheduler;
    MotorDriver &_motor_driver;
    bool _debug;

    // Fully encapsulated internal system states
    float _lastThrottleMap;
    float _lastSteeringMap;
};

#endif // ROBOT_HANDLER_HPP