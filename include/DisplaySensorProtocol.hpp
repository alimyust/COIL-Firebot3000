#pragma once

#include <Arduino.h>
#include <stdio.h>

namespace DisplaySensorProtocol {

constexpr uint8_t SENSOR_NODE_ID = 1;
constexpr uint8_t DISPLAY_NODE_ID = 2;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr uint8_t SENSOR_TELEMETRY_COMMAND = 0x20;

struct TelemetryFrame {
    int16_t temperature_centi_c = 0;
    uint16_t humidity_centi_percent = 0;
    uint16_t pm25_tenths_ug_m3 = 0;
    uint16_t voc_index_tenths = 0;
    uint16_t nox_index_tenths = 0;
    uint16_t co2_ppm = 0;
};

inline bool encodeTelemetry(const TelemetryFrame& frame,
                            char* buffer,
                            size_t bufferSize) {
    if (buffer == nullptr || bufferSize == 0) {
        return false;
    }

    const int written = snprintf(
        buffer,
        bufferSize,
        "%d,%u,%u,%u,%u,%u",
        frame.temperature_centi_c,
        frame.humidity_centi_percent,
        frame.pm25_tenths_ug_m3,
        frame.voc_index_tenths,
        frame.nox_index_tenths,
        frame.co2_ppm);

    return written > 0 && static_cast<size_t>(written) < bufferSize;
}

inline bool decodeTelemetry(const char* payload, TelemetryFrame& frame) {
    if (payload == nullptr) {
        return false;
    }

    int temperature = 0;
    unsigned int humidity = 0;
    unsigned int pm25 = 0;
    unsigned int voc = 0;
    unsigned int nox = 0;
    unsigned int co2 = 0;

    const int fields = sscanf(payload, "%d,%u,%u,%u,%u,%u",
                              &temperature, &humidity, &pm25, &voc, &nox, &co2);
    if (fields != 6) {
        return false;
    }

    frame.temperature_centi_c = static_cast<int16_t>(temperature);
    frame.humidity_centi_percent = static_cast<uint16_t>(humidity);
    frame.pm25_tenths_ug_m3 = static_cast<uint16_t>(pm25);
    frame.voc_index_tenths = static_cast<uint16_t>(voc);
    frame.nox_index_tenths = static_cast<uint16_t>(nox);
    frame.co2_ppm = static_cast<uint16_t>(co2);
    return true;
}

}  // namespace DisplaySensorProtocol
