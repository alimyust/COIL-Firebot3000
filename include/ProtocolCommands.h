#ifndef PROTOCOL_COMMANDS_HPP
#define PROTOCOL_COMMANDS_HPP

#include <Arduino.h>

namespace ProtocolCommands {
    enum NodeId : uint8_t {
        NODE_CONTROLLER = 2,
        NODE_ROBOT = 1
    };

    enum CommandId : uint8_t {
        CMD_THROTTLE = 0x01,
        CMD_STEERING = 0x02,
        CMD_SENSORS = 0x03,
        CMD_AUDIO = 0x04,
        CMD_HB = 0x05,
        CMD_DISPLAY = 0x06
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
