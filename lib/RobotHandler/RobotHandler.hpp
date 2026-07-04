#ifndef ROBOT_HANDLER_HPP
#define ROBOT_HANDLER_HPP

#include "scheduler.h"

// Forward declaration of your specific motor driver class
class MotorDriver; 

class RobotHandler {
public:
    RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, bool debug);

    // Business logic processing routines called by the scheduler bridges
    void processThrottle(const RadioComm::RF69_Packet& packet);
    void processSteering(const RadioComm::RF69_Packet& packet);
    void processMessage(const RadioComm::RF69_Packet& packet);
    void handleDiagnostics();

    // ========================================================================
    // UNIVERSAL SCHEDULER STATIC ROUTING BRIDGES
    // ========================================================================
    static void onThrottleReceived(const RadioComm::RF69_Packet& packet, void* context) {
        static_cast<RobotHandler*>(context)->processThrottle(packet);
    }

    static void onSteeringReceived(const RadioComm::RF69_Packet& packet, void* context) {
        static_cast<RobotHandler*>(context)->processSteering(packet);
    }

    static void onMessageReceived(const RadioComm::RF69_Packet& packet, void* context) {
        static_cast<RobotHandler*>(context)->processMessage(packet);
    }

    static void onDiagnosticTimerTick(void* context) {
        static_cast<RobotHandler*>(context)->handleDiagnostics();
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
    uint8_t _lastThrottleDuty;
    float _lastThrottleMap;
    uint8_t _lastSteeringDuty;
    float _lastSteeringMap;
    char _lastMessage[32]; // Adjusted boundary size for string safety
    bool _hasData;
};

#endif // ROBOT_HANDLER_HPP