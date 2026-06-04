#pragma once

#include <Arduino.h>
#include <Wire.h>

#include <SensirionI2cSen66.h>

#include "DisplaySensorProtocol.hpp"
#include "RFComm.hpp"

class Sensor {
public:
    Sensor();

    void begin();
    void update();

private:
    static constexpr uint32_t kSendIntervalMs = 1000;

    bool startMeasurement();
    bool readTelemetry(DisplaySensorProtocol::TelemetryFrame& frame);
    bool sendTelemetry(const DisplaySensorProtocol::TelemetryFrame& frame);
    void printError(const char* label, int16_t error);

    RF69_Comm _comm;
    SensirionI2cSen66 _sen66;
    unsigned long _lastSendAtMs;
};
