

#include "ProtocolLayer.hpp"
#include <stdlib.h>
#include <string.h>

ProtocolLayer *ProtocolLayer::s_instance = nullptr;

ProtocolLayer::ProtocolLayer(EventRadioComm &comm, uint8_t remoteNodeId)
    : _comm(comm), _remote_node_id(remoteNodeId), _handler(nullptr),
      _steering_cb(nullptr), _throttle_cb(nullptr) {
    s_instance = this;
}

void ProtocolLayer::process() {
    _comm.update();

    RadioEvent event;
    while (_comm.pollEvent(event)) {
        handleEvent(event);
    }
}

void ProtocolLayer::setRemoteNodeId(uint8_t remoteNodeId) {
    _remote_node_id = remoteNodeId;
}

void ProtocolLayer::setHandler(ProtocolHandler *handler) {
    _handler = handler;
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

void ProtocolLayer::handleEvent(const RadioEvent &event) {
    switch (event.type) {
        case RadioEventType::PacketReceived:
            handlePacket(event.packet);
            break;
        case RadioEventType::TelemetryTick:
            if (_handler) {
                _handler->onHeartbeat();
            }
            sendHeartbeat();
            break;
        default:
            break;
    }
}

void ProtocolLayer::handlePacket(const RF69_Packet &packet) {
    if (_handler) {
        _handler->onMessage(packet.payload);
    }

    switch (packet.command) {
        case STEERING_DUTY: {
            uint8_t duty = static_cast<uint8_t>(atoi(packet.payload));
            if (_handler) {
                _handler->onSteering(duty);
            } else if (_steering_cb) {
                _steering_cb(duty);
            }
            break;
        }
        case THROTTLE: {
            uint8_t duty = static_cast<uint8_t>(atoi(packet.payload));
            if (_handler) {
                _handler->onThrottle(duty);
            } else if (_throttle_cb) {
                _throttle_cb(duty);
            }
            break;
        }
        case BATTERY:
            if (_handler) {
                _handler->onBatteryLevel(atof(packet.payload));
            }
            break;
        default:
            Serial.print("Unknown command received: ");
            Serial.println(packet.command);
            break;
    }
}


