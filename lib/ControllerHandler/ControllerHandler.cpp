#include "ControllerHandler.hpp"
#include "Joystick.hpp"
#include "DebugLog.hpp"

ControllerHandler::ControllerHandler(EventScheduler &scheduler, Joystick &joystick, bool debug)
    : _scheduler(scheduler),
      _joystick(joystick),
      _debug(debug),
      _lastThrottleDuty(0),
      _lastSteeringDuty(0) {}

void ControllerHandler::handlePeriodicUpdate() {
    int x = 0;
    int y = 0;
    _joystick.update_joystick(x, y);

    const uint8_t steeringDuty = static_cast<uint8_t>(x);
    const uint8_t throttleDuty = static_cast<uint8_t>(y);

    // Transmit new values instantly (Cadence managed by the Universal Scheduler)
    sendControlValues(throttleDuty, steeringDuty);
    
    _lastSteeringDuty = steeringDuty;
    _lastThrottleDuty = throttleDuty;

    if (_debug) {
        DebugLog::appendField("tx_Throt", _lastThrottleDuty);
        DebugLog::appendField("tx_Steer", _lastSteeringDuty);
    }
}

void ControllerHandler::sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty) {
    char throttleBuf[4];
    char steeringBuf[4];
    
    // Convert numerical values to character payloads cleanly
    itoa(throttleDuty, throttleBuf, 10);
    itoa(steeringDuty, steeringBuf, 10);

    // Route out using the scheduler's pass-through gateway
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_THROTTLE, throttleBuf);
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_STEERING, steeringBuf);
}

void ControllerHandler::onBatteryLevel(const RadioComm::RF69_Packet& packet) {
    if (_debug) {
        float level = atof(packet.payload);
        DebugLog::appendField("bat", level);
    }
}

void ControllerHandler::onHeartbeat(const RadioComm::RF69_Packet& packet) {
    if (_debug) {
        DebugLog::appendField("tx_HB", millis());
    }
}