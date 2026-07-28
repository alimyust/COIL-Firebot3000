#pragma once
#include <Arduino.h>
//this sensor needs a 5V input, but the Feather's A0 pin is only safe up
//to 3.3v, so the sensor's AO voltage needs to be stepped down before it reaches A0.

namespace CoSensorConfig {
    static constexpr uint8_t CoPin = A0;
    
}
class CoSensor {
public:
    CoSensor() {}
    void begin() {  
    }  
    // Raw reading, still need to be calibrated
    uint16_t readRaw() {
        return analogRead(CoSensorConfig::CoPin);
    }
private:
};