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
        float coRaw;
    };

    // ADPCM Packet Structure (Matches Transmitter & Receiver)
    struct __attribute__((packed))RadioAudioPacket {
        uint16_t sequence;        // 2 bytes
        int16_t init_predicted;   // 2 bytes
        int8_t init_step_index;   // 1 byte
        uint8_t data[32];         // 32 bytes (holds 32 compressed samples)
    };  

    struct HeartbeatPayload {
        uint32_t timestamp;
    };
}
#endif
