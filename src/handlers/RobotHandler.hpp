#ifndef ROBOT_HANDLER_HPP
#define ROBOT_HANDLER_HPP

#include "scheduler.h"
#include <MotorDriver.h>                // Pulled from generic lib/
#include <sensor.h>
#include <CoSensor.h>
#include "ProtocolCommands.h"

class RobotHandler {
public:
    RobotHandler(EventScheduler &scheduler, MotorDriver &motor_driver, Sen66_Sensor &sensor, CoSensor &coSensor, bool debug,
                 uint8_t light_mux_pin, uint8_t walkie_mux_pin, uint8_t camera_mux_pin, 
                 bool last_light_mux_state, bool last_walkie_mux_state, bool last_camera_mux_state);

    // Business logic processing routines called by the scheduler bridges
    void processMotor(const ProtocolCommands::MotorPayload& payload);
    void processMux(const ProtocolCommands::MuxPayload& payload);
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

    static void onMuxReceived(const RadioComm::RF69_Packet& packet, void* context) {
        const ProtocolCommands::MuxPayload* payload = reinterpret_cast<const ProtocolCommands::MuxPayload*>(packet.payload);
        static_cast<RobotHandler*>(context)->processMux(*payload);
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
    CoSensor &_coSensor;
    bool _debug;
    uint8_t _light_mux_pin;
    uint8_t _walkie_mux_pin;
    uint8_t _camera_mux_pin;
    // Tracks current toggled output states
    bool _light_toggle_state = false;
    bool _walkie_toggle_state = false;
    bool _camera_toggle_state = false;

    // Tracks previous button states for edge detection
    bool _prev_light_button = false;
    bool _prev_walkie_button = false;
    bool _prev_camera_button = false;

};

#endif // ROBOT_HANDLER_HPP