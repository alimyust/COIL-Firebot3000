#include <SensirionI2cSen66.h>
#include "ProtocolLayer.hpp"
#include "DualHWPwm.hpp"
#include "Arduino.h"
#include <Wire.h>

SensirionI2cSen66 sensor; // SEN66 
unsigned long lastSensorRead = 0;

namespace {
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";
constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;

RF69_Comm comm(ROBOT_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, CONTROLLER_NODE_ID);
DualHardwarePWM pwm(9, 5);
// Servo throttleServo;
// Servo steeringServo;

uint16_t dutyToMicroseconds(uint8_t duty) {
    return map(duty, -100, 100, SERVO_MIN_US, SERVO_MAX_US);
}

/*uint16_t rangeToDuty(int16_t value) {
    return map(value, 0, 50, 7, 13);
}*/

void onThrottleCommand(uint8_t duty) {
    Serial.print("RX throttle=");
    Serial.println((duty));
    // throttleServo.writeMicroseconds(dutyToMicroseconds(duty));
    pwm.setDutyCycle1((duty));
}

void onSteeringCommand(uint8_t duty) {
    Serial.print("RX steering=");
    Serial.println((duty));
    // steeringServo.writeMicroseconds(dutyToMicroseconds(duty));

    pwm.setDutyCycle2((duty));
}

void onMotorSpeed(uint8_t speed) {
    Serial.print("RX motor speed=");
    Serial.println((speed));
}

void onSteeringAngle(uint8_t angle) {
    Serial.print("RX steering angle=");
    Serial.println((angle));
}
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    if (!comm.begin(nullptr, ENCRYPTION_KEY)) {
        Serial.println("Robot radio init failed");
        return;
    }
    comm.enable_debug(true);
    // throttleServo.attach(9);
    // steeringServo.attach(11);
    // throttleServo.writeMicroseconds(SERVO_MIN_US);
    // steeringServo.writeMicroseconds(SERVO_MIN_US);

    pwm.begin(60);
    pwm.setDutyCycle1(0);
    pwm.setDutyCycle2(0);

    protocol.setRemoteNodeId(CONTROLLER_NODE_ID);
    
    protocol.setThrottleCallback(onThrottleCommand);
    protocol.setSteeringCallback(onSteeringCommand);
   
    
    Wire.begin(); // SEN66 
    sensor.begin(Wire, SEN66_I2C_ADDR_6B);
    sensor.deviceReset();
    delay(1200);  // required after reset per SEN66 datasheet
    sensor.startContinuousMeasurement();
    delay(1500);  // first measurement takes up to 1.5s to be ready

}

void loop() {
    protocol.process();  // radio RX — unchanged

    if (millis() - lastSensorRead >= 1000) {
        lastSensorRead = millis();

        float pm1, pm2p5, pm4p0, pm10, rh, temp, voc, nox;
        uint16_t co2;

        int16_t err = sensor.readMeasuredValues(
            pm1, pm2p5, pm4p0, pm10,
            rh, temp, voc, nox, co2
        );

        if (err == 0) {
            sen66_packet pkt; // packet sent
            pkt.pm1_0    = pm1;
            pkt.pm2_5    = pm2p5;
            pkt.pm10     = pm10;
            pkt.rh       = rh;
            pkt.temp     = temp;
            pkt.voc      = voc;
            pkt.nox      = nox;
            pkt.co2_hcho = (float)co2;

            protocol.sendSensorData(pkt);

            Serial.print("Sensor TX — Temp: ");
            Serial.print(temp, 1);
            Serial.print("C  RH: ");
            Serial.print(rh, 1);
            Serial.println("%");
        } else {
            Serial.print("SEN66 read error: ");
            Serial.println(err);
        }
    }
}

