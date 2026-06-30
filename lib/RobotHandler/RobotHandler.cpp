#include "RobotHandler.hpp"
#include "Arduino.h"

namespace {
constexpr unsigned long HEARTBEAT_INTERVAL_MS = 1000;
}

void RobotHandler::update() {
    const unsigned long now = millis();
    if (now - _lastHeartbeatSentMs >= HEARTBEAT_INTERVAL_MS) {
        _lastHeartbeatSentMs = now;
        Serial.print("[robot] heartbeat sent @ ");
        Serial.print(now);
        Serial.println(" ms");
    }
}

void RobotHandler::onThrottle(uint8_t duty) {
    _motor_driver.setThrottle(duty);
}

void RobotHandler::onSteering(uint8_t duty) {
    _motor_driver.setSteeringDuty(duty);
}

void RobotHandler::onHeartbeat() {
    const unsigned long now = millis();
    _lastHeartbeatMs = now;
    Serial.print("[robot] heartbeat received @ ");
    Serial.print(now);
    Serial.print(" ms, last sent=");
    Serial.print(_lastHeartbeatSentMs);
    Serial.print(" ms, delta=");
    Serial.print(now - _lastHeartbeatSentMs);
    Serial.println(" ms");
}
  