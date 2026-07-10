

#include <Wire.h>
#include <SensirionI2cSen66.h>

// class for the Sen66 sensor module.

class Sensor {
    public:
        Sensor();
        void begin(){
            Wire.begin(); // SEN66 
            sensor.begin(Wire, SEN66_I2C_ADDR_6B);
            sensor.deviceReset();
            delay(1200);  // required after reset per SEN66 datasheet
            sensor.startContinuousMeasurement();
            delay(1500);  // first measurement takes up to 1.5s to be ready
        }
        
        bool readData(float &co2, float &tvoc, float &temp, float &rh){
            sensor.startContinuousMeasurement();
        }

    private:
        SensirionI2cSen66 _sen66;