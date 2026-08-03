#ifndef PROTOCOL_COMMANDS_HPP
#define PROTOCOL_COMMANDS_HPP

#include <Arduino.h>

namespace ProtocolCommands {
    enum NodeId : uint8_t {
        NODE_CONTROLLER = 2,
        NODE_ROBOT = 1
    };

    enum CommandId : uint8_t {
        CMD_MOTOR = 0x01, // encapsulates throttle, steering, turretX, turretY
        CMD_SENSORS = 0x02, // All sensor payloads and CO sensor (not implemented yet)
        CMD_AUDIO = 0x03,  // All audio packets
        CMD_HB = 0x04,  // Basic heartbeat (shouold be all telemetry)
    };

    struct MotorPayload {
        uint8_t throttle_duty;
        uint8_t steer_duty;
        uint8_t turret_x_duty;
        uint8_t turret_y_duty;
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
        float co2;
    };

// G.711 u-law Audio Packet Structure
    struct __attribute__((packed)) RadioAudioPacket {
        uint16_t sequence;        // 2 bytes: sequence counter for loss tracking
        uint8_t data[32];         // 35 bytes: 35 u-law encoded audio samples (1 byte/sample)
    };

    struct HeartbeatPayload {
        uint32_t timestamp;
    };
}
#endif
