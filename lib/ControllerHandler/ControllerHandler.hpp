#pragma once

#include "ProtocolLayer.hpp"
#include "Joystick.hpp"

class ControllerHandler : public ProtocolLayer::ProtocolHandler {
public:
    ControllerHandler(ProtocolLayer &protocol, Joystick &joystick, bool debug = false);

    void update();
    void onBatteryLevel(float level) override;
    void onHeartbeat() override;

private:
    void sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty);

    ProtocolLayer &_protocol;
    Joystick &_joystick;
    bool _debug;
    int _lastThrottleDuty;
    int _lastSteeringDuty;
    unsigned long _lastSendTime;
};
