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
    };

    struct ThrottlePayload {
        uint8_t duty;
    };

    struct SteeringPayload {
        uint8_t duty;
    };

    struct SensorPayload {
        float pm1p0;
        float pm2p5;
        float pm4p0;
        float pm10p0;
        float humidity;
        float temperature;
        float vocIndex;
        float noxIndex;
    };

    inline bool deserializeNumericPayload(const RadioComm::RF69_Packet& packet, uint8_t& value) {
        if (packet.payload[0] == '\0') {
            return false;
        }

        char* end = nullptr;
        const unsigned long parsed = strtoul(packet.payload, &end, 10);
        if (end == packet.payload) {
            return false;
        }

        value = static_cast<uint8_t>(parsed);
        return true;
    }

    inline bool deserializeNumericPayload(const RadioComm::RF69_Packet& packet, float& value) {
        if (packet.payload[0] == '\0') {
            return false;
        }

        char* end = nullptr;
        value = strtof(packet.payload, &end);
        return end != packet.payload;
    }

    inline bool deserializeThrottlePayload(const RadioComm::RF69_Packet& packet, ThrottlePayload& payload) {
        return deserializeNumericPayload(packet, payload.duty);
    }

    inline bool deserializeSteeringPayload(const RadioComm::RF69_Packet& packet, SteeringPayload& payload) {
        return deserializeNumericPayload(packet, payload.duty);
    }

    inline bool deserializeSensorPayload(const RadioComm::RF69_Packet& packet, SensorPayload& payload) {
        if (!packet.payload) return false;

        char buffer[128];
        strncpy(buffer, packet.payload, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        char* end = nullptr;
        char* token = strtok(buffer, ",");
        if (!token) return false;
        payload.pm1p0 = strtof(token, &end);
        if (end == token) return false;

        token = strtok(nullptr, ",");
        if (!token) return false;
        payload.pm2p5 = strtof(token, &end);
        if (end == token) return false;

        token = strtok(nullptr, ",");
        if (!token) return false;
        payload.pm4p0 = strtof(token, &end);
        if (end == token) return false;

        token = strtok(nullptr, ",");
        if (!token) return false;
        payload.pm10p0 = strtof(token, &end);
        if (end == token) return false;

        token = strtok(nullptr, ",");
        if (!token) return false;
        payload.humidity = strtof(token, &end);
        if (end == token) return false;

        token = strtok(nullptr, ",");
        if (!token) return false;
        payload.temperature = strtof(token, &end);
        if (end == token) return false;

        token = strtok(nullptr, ",");
        if (!token) return false;
        payload.vocIndex = strtof(token, &end);
        if (end == token) return false;

        token = strtok(nullptr, ",");
        if (!token) return false;
        payload.noxIndex = strtof(token, &end);
        if (end == token) return false;

        return true;
    }

}

#endif
