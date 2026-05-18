#include <Arduino.h>
#include "DualHWPwm.hpp"
#include "RFComm.hpp"

namespace {

constexpr uint8_t DRIVE_PWM_PIN = 9;
constexpr uint8_t STEER_PWM_PIN = 5;
constexpr uint8_t ROBOT_NODE_ID = 1;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr uint32_t PWM_FREQUENCY_HZ = 50;

enum CommandId : uint8_t {
    MOTOR_SPEED = 0x01,
    STEERING = 0x02,
    BATTERY = 0x03,
    HEARTBEAT = 0x04,
    THROTTLE = 0x05,
    STEERING_DUTY = 0x06,
};

DualHardwarePWM pwm(DRIVE_PWM_PIN, STEER_PWM_PIN);
RF69_Comm comm(ROBOT_NODE_ID, RF_FREQUENCY_MHZ);

void setThrottleDuty(uint8_t duty) {
    pwm.setDutyCycle1(duty);
    Serial.print("Throttle duty set to: ");
    Serial.println(duty);
}

void setSteeringDuty(uint8_t duty) {
    pwm.setDutyCycle2(duty);
    Serial.print("Steering duty set to: ");
    Serial.println(duty);
}

void handlePacket(RF69_Packet &packet) {
    switch (packet.command) {
        case THROTTLE:
            setThrottleDuty(static_cast<uint8_t>(atoi(packet.payload)));
            break;
        case STEERING_DUTY:
            setSteeringDuty(static_cast<uint8_t>(atoi(packet.payload)));
            break;
        case MOTOR_SPEED:
        case STEERING:
        case BATTERY:
        case HEARTBEAT:
        default:
            break;
    }
}

void receiveCallback(RF69_Packet &packet) {
    handlePacket(packet);
}

}

void setup() {
    Serial.begin(9600);
    comm.set_receive_handler(receiveCallback);
    comm.begin(nullptr, "encryptionkey16");
    pwm.begin(PWM_FREQUENCY_HZ);
    Serial.println("Robot started");
}

void loop() {
    comm.update();
}
