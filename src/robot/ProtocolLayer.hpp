
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
    bool sendBatteryLevel(float level);
    bool sendHeartbeat();

    // callbacks to RobotHandler
    void setMotorCallback(void (*cb)(uint8_t));
    void setSteeringCallback(void (*cb)(uint8_t));

    enum CommandID : uint8_t {
        MOTOR_SPEED = 0x01,
        STEERING    = 0x02,
        BATTERY     = 0x03,
        HEARTBEAT   = 0x04
    };

private:
    static ProtocolLayer *s_instance;
    static void receiveCallback(RF69_Packet &packet);

    void handlePacket(RF69_Packet &packet);

    RF69_Comm &_comm;
    void (*_motor_cb)(uint8_t);
    void (*_steering_cb)(uint8_t);
};