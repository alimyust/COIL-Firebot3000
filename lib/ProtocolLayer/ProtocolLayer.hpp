
// #pragma once

// #include "radio.h"

// // A class to handle all tx/rx radio comms, separating robot
// // operation logic from communication layer.

// class ProtocolLayer {
// public:
//     class ProtocolHandler {
//     public:
//         virtual ~ProtocolHandler() = default;
//         virtual void onThrottle(uint8_t duty) {}
//         virtual void onSteering(uint8_t duty) {}
//         virtual void onBatteryLevel(float level) {}
//         virtual void onMessage(const char *message) {}
//         virtual void onHeartbeat() {}
//     };

//     ProtocolLayer(EventRadioComm &comm, uint8_t remoteNodeId = 1);

//     void process();  // handles RX and drains event queue

//     bool enqueueThrottle(uint8_t duty);
//     bool enqueueSteering(uint8_t duty);
//     bool enqueueBatteryLevel(float level);
//     bool enqueueHeartbeat();

//     void setRemoteNodeId(uint8_t remoteNodeId);
//     void setHandler(ProtocolHandler *handler);

//     enum CommandID : uint8_t {
//         MOTOR_SPEED = 0x01,
//         STEERING    = 0x02,
//         BATTERY     = 0x03,
//         HEARTBEAT   = 0x04,
//         THROTTLE    = 0x05,
//         STEERING_DUTY = 0x06
//     };

// private:
//     void handleEvent(const RadioEvent &event);
//     void handlePacket(const RF69_Packet &packet);

//     EventRadioComm &_comm;
//     uint8_t _remote_node_id;
//     ProtocolHandler *_handler;
// };