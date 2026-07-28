#ifndef ROBOT_HANDLER_HPP
#define ROBOT_HANDLER_HPP

#include "scheduler.h"
#include <MotorDriver.h>                // Pulled from generic lib/
#include <sensor.h>
#include "ProtocolCommands.h"

class RobotHandler {
public:
    RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, Sen66_Sensor &sensor, bool debug);

    // Business logic processing routines called by the scheduler bridges
    void processMotor(const ProtocolCommands::MotorPayload& payload);
    void handleDiagnostics();
    void processHeartbeat(const ProtocolCommands::HeartbeatPayload& payload);
    void onSensorTrigger();
    // ========================================================================
    // UNIVERSAL SCHEDULER STATIC ROUTING BRIDGES
    // ===============  =========================================================
    
    static void onSensorUpdate(void* context) {
        static_cast<RobotHandler*>(context)->onSensorTrigger();
    }

    static void onMotorReceived(const RadioComm::RF69_Packet& packet, void* context) {
        const ProtocolCommands::MotorPayload* payload = reinterpret_cast<const ProtocolCommands::MotorPayload*>(packet.payload);
        static_cast<RobotHandler*>(context)->processMotor(*payload);
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
    Sen66_Sensor &_sensor;
    bool _debug;

};

#endif // ROBOT_HANDLER_HPP