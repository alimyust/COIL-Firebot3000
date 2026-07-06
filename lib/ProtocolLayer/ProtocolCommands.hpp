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
        CMD_BATTERY = 0x03,
        CMD_HEARTBEAT = 0x04,
        CMD_MESSAGE = 0x05
    };
}

#endif
