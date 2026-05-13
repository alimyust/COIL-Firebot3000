

#include "ProtocolLayer.hpp"
#include <string.h>

ProtocolLayer *ProtocolLayer::s_instance = nullptr;

ProtocolLayer::ProtocolLayer(RF69_Comm &comm)
    : _comm(comm), _motor_cb(nullptr), _steering_cb(nullptr) {
    s_instance = this;
    _comm.set_receive_handler(&ProtocolLayer::receiveCallback);
}

void ProtocolLayer::process() {
    _comm.update();
}

bool ProtocolLayer::sendMotorSpeed(uint8_t speed) {
    char buf[4];
    sprintf(buf, "%d", speed);
    return _comm.send(1, MOTOR_SPEED, buf);
}

bool ProtocolLayer::sendSteeringAngle(uint8_t angle) {
    char buf[4];
    sprintf(buf, "%d", angle);
    return _comm.send(1, STEERING, buf);
}

bool ProtocolLayer::sendBatteryLevel(float level) {
    char buf[16];
    sprintf(buf, "%.2f", level);
    return _comm.send(1, BATTERY, buf);
}

bool ProtocolLayer::sendHeartbeat() {
    return _comm.send(1, HEARTBEAT, "HB");
}

void ProtocolLayer::setMotorCallback(void (*cb)(uint8_t)) {
    _motor_cb = cb;
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
        case MOTOR_SPEED:
            if (_motor_cb) {
                uint8_t speed = atoi(packet.payload);
                _motor_cb(speed);
            }
            break;
        case STEERING:
            if (_steering_cb) {
                uint8_t angle = atoi(packet.payload);
                _steering_cb(angle);
            }
            break;
        default:
            break;
    }
}


