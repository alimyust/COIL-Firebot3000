




#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cSen66.h>


class Sensor {
    public:
        Sensor() {}

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
        bool readData(float &co2, float &tvoc, float &temp, float &rh) {
            bool isReady = false;
            uint8_t padding = 0; // Required by driver syntax

            // Check if data is ready to be fetched
            uint16_t error = sensor.getDataReady(padding, isReady);
            if (error || !isReady) {
                return false; // Data not ready or I2C error, exit without blocking
            }

            // Dummies for extra particulate matter & NOx outputs
            float pm1_0, pm2_5, pm4_0, pm10_0, noxIndex;
            uint16_t rawCo2;

            // Read all 9 measured parameters from SEN66
            error = sensor.readMeasuredValues(
                pm1_0, pm2_5, pm4_0, pm10_0, 
                rh, temp, tvoc, noxIndex, rawCo2
            );

            if (error != 0) {
                return false;
            }

            // Assign out values
            co2 = (float)rawCo2;
            
            return true; // Successfully read new data
        }

    private:
        SensirionI2cSen66 sensor;
};

// Usage Example
Sensor sen66;

void setup() {
    Serial.begin(115200);
    while(!Serial && millis() < 3000); // Wait for Serial console
    
    sen66.begin();
    Serial.println("SEN66 Initialized.");
}

void loop() {
    float co2, tvoc, temp, rh;

    // Call non-blocking read in main loop
    if (sen66.readData(co2, tvoc, temp, rh)) {
        Serial.print("CO2: "); Serial.print(co2); Serial.print(" ppm | ");
        Serial.print("VOC Index: "); Serial.print(tvoc); Serial.print(" | ");
        Serial.print("Temp: "); Serial.print(temp); Serial.print(" C | ");
        Serial.print("Humidity: "); Serial.print(rh); Serial.println(" %");
    }

    // You can perform other tasks here without delay() blocking execution
}