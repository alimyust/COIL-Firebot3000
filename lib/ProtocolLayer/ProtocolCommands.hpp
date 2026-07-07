#ifndef PROTOCOL_COMMANDS_HPP
#define PROTOCOL_COMMANDS_HPP

#include <Arduino.h>
#include "radio.h"
#include <stdlib.h>

namespace ProtocolCommands {
    enum NodeId : uint8_t {
        NODE_CONTROLLER = 2,
        NODE_ROBOT = 1
    };

    enum CommandId : uint8_t {
        CMD_THROTTLE = 0x01,
        CMD_STEERING = 0x02,
        CMD_BATTERY = 0x03,
        CMD_HEARTBEAT = 0x04,
        CMD_MESSAGE = 0x05
    };

    struct ThrottlePayload {
        uint8_t duty;
    };

    struct SteeringPayload {
        uint8_t duty;
    };

    struct BatteryPayload {
        float level;
    };

    struct HeartbeatPayload {
    };

    inline bool deserializeThrottlePayload(const RadioComm::RF69_Packet& packet, ThrottlePayload& payload) {
        if (packet.payload[0] == '\0') {
            return false;
        }

        char* end = nullptr;
        const unsigned long value = strtoul(packet.payload, &end, 10);
        if (end == packet.payload) {
            return false;
        }

        payload.duty = static_cast<uint8_t>(value);
        return true;
    }

    inline bool deserializeSteeringPayload(const RadioComm::RF69_Packet& packet, SteeringPayload& payload) {
        Serial.print(packet.payload[0]);
        if (packet.payload[0] == '\0') {
            return false;
        }

        char* end = nullptr;
        const unsigned long value = strtoul(packet.payload, &end, 10);
        if (end == packet.payload) {
            return false;
        }

        payload.duty = static_cast<uint8_t>(value);
        return true;
    }

    inline bool deserializeBatteryPayload(const RadioComm::RF69_Packet& packet, BatteryPayload& payload) {
        if (packet.payload[0] == '\0') {
            return false;
        }

        char* end = nullptr;
        payload.level = strtof(packet.payload, &end);
        return end != packet.payload;
    }

    inline bool deserializeHeartbeatPayload(const RadioComm::RF69_Packet&, HeartbeatPayload&) {
        return true;
    }
}

#endif
