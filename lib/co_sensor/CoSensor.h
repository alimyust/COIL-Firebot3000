#pragma once
#include <Arduino.h>
//this sensor needs a 5V input, but the Feather's A0 pin is only safe up
//to 3.3v, so the sensor's AO voltage needs to be stepped down before it reaches A0.

namespace CoSensorConfig {
    static constexpr uint8_t DefaultCoPin = A5;
}
class CoSensor {
public:
    CoSensor(uint8_t co_pin = CoSensorConfig::DefaultCoPin) : _co_pin(co_pin) {}
    void begin() {  
    }  
    // Raw reading, still need to be calibrated
    float readRaw() {
        return analogRead(_co_pin);
    }
private:
    uint8_t _co_pin;
};