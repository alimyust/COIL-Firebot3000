#pragma once

#include <Wire.h>
#include <SensirionI2cSen66.h>


class Sen66_Sensor {
    public:
        Sen66_Sensor() {}

        void begin() {
            Wire.begin();
            
            // Initialize sensor at default I2C address 0x6B
            sensor.begin(Wire, SEN66_I2C_ADDR_6B);
            
            // Reset sensor and wait for boot
            sensor.deviceReset();
            delay(1200); 

            // Start continuous measurement mode ONCE
            uint16_t error = sensor.startContinuousMeasurement();
            if (error) {
                Serial.println("Error starting SEN66 measurement!");
            }
        }
        
        // Non-blocking read. Returns 'true' only when fresh data was updated.
        bool readData(float &co2, float &tvoc, float &temp, float &rh,
                    float &pm1_0, float &pm2_5, float &pm4_0, float &pm10_0
                    ,float &noxIndex) {
            bool isReady = false;
            uint8_t padding = 0; // Required by driver syntax
            uint16_t raw_co2 = 0;
            // Check if data is ready to be fetched
            uint16_t error = sensor.getDataReady(padding, isReady);
            if (error || !isReady) {
                return false; // Data not ready or I2C error, exit without blocking
            }

            // Read all 9 measured parameters from SEN66
            error = sensor.readMeasuredValues(
                pm1_0, pm2_5, pm4_0, pm10_0, 
                rh, temp, tvoc, noxIndex, raw_co2
            );

            co2 = (float) raw_co2;

            if (error != 0) {
                return false;
            }
            return true; // Successfully read new data
        }

    private:
        SensirionI2cSen66 sensor;
};
