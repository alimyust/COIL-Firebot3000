

#include "ProtocolLayer.hpp"
#include <Servo.h>
#include "Arduino.h"
#include "MotorDriver.h"

namespace {
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";
constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;

RF69_Comm comm(ROBOT_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, CONTROLLER_NODE_ID);
MotorDriver motor_driver(true);
}

void setup() {
    Serial.begin(115200);
    // while (!Serial) {}


    // if (!comm.begin(nullptr, ENCRYPTION_KEY)) {
    //     Serial.println("Robot radio init failed");
    //     return;
    // }

   
    // protocol.setRemoteNodeId(CONTROLLER_NODE_ID);
    // protocol.setThrottleCallback([](uint8_t duty) {
    //     motor_driver.setThrottle(duty);
    // });
    // protocol.setSteeringCallback([](uint8_t duty) {
    //     motor_driver.setSteeringDuty(duty);
    // });
    // motor_driver.init_motor();

    Serial.println("Robot radio started");
}




void loop() {
    // protocol.process();
}

