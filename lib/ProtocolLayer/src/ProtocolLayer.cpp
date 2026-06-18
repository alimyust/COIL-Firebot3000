

#include "ProtocolLayer.hpp"
#include <string.h>

ProtocolLayer *ProtocolLayer::s_instance = nullptr;

ProtocolLayer::ProtocolLayer(RF69_Comm &comm, uint8_t remoteNodeId)
    : _comm(comm), _remote_node_id(remoteNodeId), _steering_cb(nullptr), _throttle_cb(nullptr) {
    s_instance = this;
    _comm.set_receive_handler(&ProtocolLayer::receiveCallback);
}

void ProtocolLayer::process() {
    _comm.update();
}

void ProtocolLayer::setRemoteNodeId(uint8_t remoteNodeId) {
    _remote_node_id = remoteNodeId;
}

bool ProtocolLayer::sendThrottle(uint8_t duty) {
    char buf[4];
    sprintf(buf, "%d", duty);
    return _comm.send(_remote_node_id, THROTTLE, buf);
}

bool ProtocolLayer::sendSteering(uint8_t duty) {
    char buf[4];
    sprintf(buf, "%d", duty);
    return _comm.send(_remote_node_id, STEERING_DUTY, buf);
}

bool ProtocolLayer::sendBatteryLevel(float level) {
    char buf[16];
    sprintf(buf, "%.2f", level);
    return _comm.send(_remote_node_id, BATTERY, buf);
}

bool ProtocolLayer::sendHeartbeat() {
    return _comm.send(_remote_node_id, HEARTBEAT, "HB");
}


void ProtocolLayer::setThrottleCallback(void (*cb)(uint8_t)) {
    _throttle_cb = cb;
}

void ProtocolLayer::setSteeringCallback(void (*cb)(uint8_t)) {
    _steering_cb = cb;
}

void ProtocolLayer::receiveCallback(RF69_Packet &packet) {
    if (s_instance) {
        s_instance->handlePacket(packet);
    }
}

void ProtocolLayer::handlePacket(RF69_Packet &packet) {
    switch (packet.command) {
        case STEERING:
            if (_steering_cb) {
                float duty = atof(packet.payload);
                _steering_cb(duty);
            }
            break;
        case THROTTLE:
            if (_throttle_cb) {
                float duty = atof(packet.payload);
                _throttle_cb(duty);
            }
            break;
        default:
            break;
    }
}


