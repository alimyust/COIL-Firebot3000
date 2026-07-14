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
        CMD_SENSORS = 0x03
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
}
#endif
