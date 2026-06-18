
#pragma once

#include "RFComm.hpp"

// A class to handle all tx/rx radio comms, separating robot
// operation logic from communication layer.

//defined packet structures

struct motor_packet{
    uint8_t throttle_duty;
    uint8_t steering_duty;
};

//PM, RH&T, VOC, NOx, CO2/HCHO
struct sen66_packet{
    float pm1_0;
    float pm2_5;
    float pm10;
    float rh;
    float temp;
    float voc;
    float nox;
    float co2_hcho;
};

struct telemetry_packet{
    float battery_level;
    float uptime;
};

class ProtocolLayer {
public:
    ProtocolLayer(RF69_Comm &comm, uint8_t remoteNodeId = 1);

    void process();  // handles RX

    // outgoing commands

    bool sendThrottle(uint8_t duty);
    bool sendSteering(uint8_t duty);
    bool sendBatteryLevel(float level);
    bool sendHeartbeat();

    void setRemoteNodeId(uint8_t remoteNodeId);

    // callbacks to RobotHandler
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
    uint8_t _remote_node_id;
    void (*_motor_cb)(uint8_t);
    void (*_steering_cb)(uint8_t);
    void (*_throttle_cb)(uint8_t);
    void (*_steering_duty_cb)(uint8_t);
};