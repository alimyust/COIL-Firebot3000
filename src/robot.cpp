#include <Arduino.h>
#include "ProtocolCommands.h"

#include "radio.h"
#include "MotorDriver.h"
#include "Microphone.hpp"
#include "Speaker.hpp"
#include "sensor.h"
#include "CoSensor.h"

#include "handlers/scheduler.h"
#include "handlers/RobotHandler.hpp"
#include "handlers/AudioHandler.hpp"

namespace RobotPins {
    constexpr uint8_t LIGHT_MUX_PIN = 10;
    constexpr uint8_t WALKIE_MUX_PIN = A1;
    constexpr uint8_t CAMERA_MUX_PIN = 12;

    constexpr uint8_t RADIO_CS_PIN = 8;
    constexpr uint8_t RADIO_INT_PIN = 3;
    constexpr uint8_t RADIO_RST_PIN = 4;

    constexpr uint8_t AUDIO_MIC_ADC_PIN = A1;
    constexpr uint8_t AUDIO_SPEAKER_DAC_PIN = A0;

    constexpr uint8_t CO_SENSOR_PIN = A5;
}
RadioComm radio(1, 915.0, RobotPins::RADIO_CS_PIN, RobotPins::RADIO_INT_PIN, RobotPins::RADIO_RST_PIN); // Node 1 (Robot)
EventScheduler scheduler(radio, true); 
MotorDriver motorDriver; 
Sen66_Sensor sensor;

CoSensor coSensor(RobotPins::CO_SENSOR_PIN);
RobotHandler robotHandler(scheduler, motorDriver, sensor, coSensor, true, RobotPins::LIGHT_MUX_PIN, RobotPins::WALKIE_MUX_PIN, RobotPins::CAMERA_MUX_PIN, false, false, false);

// Microphone mic(RobotPins::AUDIO_MIC_ADC_PIN);
// Speaker speaker(RobotPins::AUDIO_SPEAKER_DAC_PIN);
// AudioHandler audioHandler(scheduler, mic, speaker, false); // Initialize AudioHandler with microphone
// connect to mic

void setup() {
    Serial.begin(115200);
    // while(!Serial);
    pinMode(RobotPins::LIGHT_MUX_PIN, OUTPUT);
    pinMode(RobotPins::WALKIE_MUX_PIN, OUTPUT);
    pinMode(RobotPins::CAMERA_MUX_PIN, OUTPUT);
    radio.begin(); 
    motorDriver.init_motor();
    sensor.begin();
    coSensor.begin();

    scheduler.registerPacketHandler(
        ProtocolCommands::CMD_MOTOR,
        EventPriority::PRIORITY_HIGH,
        RobotHandler::onMotorReceived,
        &robotHandler
    );
    
    scheduler.registerPacketHandler(
        ProtocolCommands::CMD_HB,
        EventPriority::PRIORITY_LOW,
        RobotHandler::onHeartbeatReceived,
        &robotHandler
    );

    scheduler.registerPacketHandler(
        ProtocolCommands::CMD_MUX,
        EventPriority::PRIORITY_HIGH,
        RobotHandler::onMuxReceived,
        &robotHandler
    );

    scheduler.addPeriodicTask(
        5000,
        EventPriority::PRIORITY_MEDIUM,
        RobotHandler::onSensorUpdate,
        &robotHandler);

}
void loop() {
    radio.update();
    scheduler.update();
    // Serial.println("Looping");
}