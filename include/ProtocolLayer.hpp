
#pragma once

#include "RFComm.hpp"

// A class to handle all tx/rx radio comms, separating robot
// operation logic from communication layer.

class ProtocolLayer {
public:
    ProtocolLayer(RF69_Comm &comm);

    void process();  // handles RX

    // outgoing commands

    bool sendMotorSpeed(uint8_t speed);
    bool sendSteeringAngle(uint8_t angle);
    bool sendThrottle(uint8_t duty);
    bool sendSteering(uint8_t duty);
    bool sendBatteryLevel(float level);
    bool sendHeartbeat();

    // callbacks to RobotHandler
    void setMotorCallback(void (*cb)(uint8_t));
    void setSteeringCallback(void (*cb)(uint8_t));
    void setThrottleCallback(void (*cb)(uint8_t));
    void setSteeringDutyCallback(void (*cb)(uint8_t));

    enum CommandID : uint8_t {
        MOTOR_SPEED = 0x01,
        STEERING    = 0x02,
        BATTERY     = 0x03,
        HEARTBEAT   = 0x04,
        THROTTLE    = 0x05,
        STEERING_DUTY = 0x06
    };

private:
    static ProtocolLayer *s_instance;
    static void receiveCallback(RF69_Packet &packet);

    void handlePacket(RF69_Packet &packet);

    RF69_Comm &_comm;
    void (*_motor_cb)(uint8_t);
    void (*_steering_cb)(uint8_t);
    void (*_throttle_cb)(uint8_t);
    void (*_steering_duty_cb)(uint8_t);
};