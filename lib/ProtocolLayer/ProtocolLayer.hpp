
#pragma once

#include "radio.h"

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
    class ProtocolHandler {
    public:
        virtual ~ProtocolHandler() = default;
        virtual void onThrottle(uint8_t duty) {}
        virtual void onSteering(uint8_t duty) {}
        virtual void onBatteryLevel(float level) {}
        virtual void onMessage(const char *message) {}
        virtual void onHeartbeat() {}
    };

    ProtocolLayer(EventRadioComm &comm, uint8_t remoteNodeId = 1);

    void process();  // handles RX and drains event queue

    // outgoing commands
    bool sendThrottle(uint8_t duty);
    bool sendSteering(uint8_t duty);
    bool sendBatteryLevel(float level);
    bool sendHeartbeat();

    void setRemoteNodeId(uint8_t remoteNodeId);
    void setHandler(ProtocolHandler *handler);

    // callback to link to motor control logic
    void setThrottleCallback(void (*cb)(uint8_t));
    void setSteeringCallback(void (*cb)(uint8_t));

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

    void handleEvent(const RadioEvent &event);
    void handlePacket(const RF69_Packet &packet);

    EventRadioComm &_comm;
    uint8_t _remote_node_id;
    ProtocolHandler *_handler;
    void (*_steering_cb)(uint8_t);
    void (*_throttle_cb)(uint8_t);
};