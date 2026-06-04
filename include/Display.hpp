#pragma once

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "DisplaySensorProtocol.hpp"
#include "RFComm.hpp"

class Display {
public:
    Display();

    void begin();
    void update();
    void handlePacket(RF69_Packet& packet);

private:
    static constexpr uint16_t kScreenWidth = 64;
    static constexpr uint16_t kScreenHeight = 128;
    static constexpr uint8_t kOledAddress = 0x3C;
    static constexpr uint32_t kSignalTimeoutMs = 3000;

    void drawWaitingScreen();
    void drawTelemetryScreen();

    Adafruit_SH1107 _display;
    RF69_Comm _comm;
    DisplaySensorProtocol::TelemetryFrame _telemetry;
    bool _hasTelemetry;
    unsigned long _lastPacketAtMs;
    uint32_t _packetCount;
};
