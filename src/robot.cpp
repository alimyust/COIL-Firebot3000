#include <Arduino.h>
#include "ProtocolCommands.h"

#include "radio.h"
#include "MotorDriver.h"
#include "Microphone.hpp"
#include "Speaker.hpp"
#include "sensor.h"

#include "handlers/scheduler.h"
#include "handlers/RobotHandler.hpp"
#include "handlers/AudioHandler.hpp"


RadioComm radio(ProtocolCommands::NODE_ROBOT, 915.0, 8, 3, 4); // Node 1 (Robot)
EventScheduler scheduler(radio, true); 

MotorDriver motorDriver; 
Sen66_Sensor sensor;
RobotHandler robotHandler(scheduler, motorDriver, sensor, true);

Microphone mic;
Speaker speaker;
AudioHandler audioHandler(scheduler, mic, speaker, true); // Initialize AudioHandler with microphone
// connect to mic

void setup() {
    Serial.begin(115200);
    // while(!Serial);

    radio.begin(); 
    // motorDriver.init_motor();
    // sensor.begin();
    // audioHandler.beginTimer();
    speaker.begin();
    // scheduler.registerPacketHandler(ProtocolCommands::CMD_MOTOR, EventPriority::PRIORITY_HIGH, RobotHandler::onMotorReceived, &robotHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_AUDIO, EventPriority::PRIORITY_CRITICAL, AudioHandler::onAudioPacketReceived, &audioHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_AUDIO_RAW, EventPriority::PRIORITY_CRITICAL, AudioHandler::onAudioPacketReceivedRaw, &audioHandler);
    scheduler.registerPacketHandler(ProtocolCommands::CMD_HB, EventPriority::PRIORITY_LOW, RobotHandler::onHeartbeatReceived, &robotHandler);
    
    // scheduler.addPeriodicTask(1000, EventPriority::PRIORITY_MEDIUM, RobotHandler::onSensorUpdate, &robotHandler);
    Serial.println("Robot Setup Complete");
}
void loop() {
    radio.update();
    scheduler.update();
    // oled.update();      

    // static unsigned long lastPrintTime = 0;
    // constexpr unsigned long PRINT_INTERVAL = 1000;

    // unsigned long currentTime = millis();

    // if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
    //     lastPrintTime = currentTime;
    //     Serial.println("Robot Loop Alive");
    // }

}
// #include <Arduino.h>
// #include <math.h>

// const int DAC_PIN = A0;

// // Signal configuration
// const float TARGET_VPP   = 2.0;   // 2.0 V peak-to-peak
// const float REF_VOLTAGE  = 3.3;   // 3.3 V DAC reference
// const size_t TABLE_SIZE  = 16;    // 16 samples per cycle (16 kHz update rate)
// const uint32_t STEP_DELAY_US = 62; // 1,000,000 us / 16,000 Hz = 62.5 us per sample

// // Precomputed 10-bit DAC values (0 to 1023)
// uint16_t dac_sine_table[TABLE_SIZE];

// void setup() {
//     Serial.begin(115200);
//     delay(2000);

//     Serial.println("--- SAMD21 Direct analogWrite DAC 1 kHz Sine Wave ---");

//     // Configure Arduino to use 10-bit resolution (0 - 1023)
//     analogWriteResolution(10);

//     /*
//      * Math breakdown for 10-bit DAC (0..1023):
//      * Mid-point (1.65V bias) = 511.5 counts
//      * Full Scale Range (3.3V)  = 1023 counts
//      * Peak Amplitude (2.0 Vpp) = (2.0 / 3.3) * 1023 / 2 ≈ 310 counts
//      * Output range will swing between ~201 (0.65V) and ~821 (2.65V)
//      */
//     const float dac_midpoint  = 1023.0 / 2.0; // 511.5
//     const float dac_amplitude = (1023.0 * (TARGET_VPP / REF_VOLTAGE)) / 2.0; // ~310

//     for (size_t i = 0; i < TABLE_SIZE; i++) {
//         float angle = (2.0 * M_PI * i) / TABLE_SIZE;
//         // Shift sine wave (-1.0 to +1.0) up by mid-point bias
//         dac_sine_table[i] = (uint16_t)(dac_midpoint + (sin(angle) * dac_amplitude));
//     }
// }

// void loop() {
//     static size_t index = 0;
//     static uint32_t last_micros = 0;

//     // Fixed timing at ~62.5 us per step (16 kHz sample rate -> 1 kHz output)
//     if (micros() - last_micros >= STEP_DELAY_US) {
//         last_micros = micros();

//         analogWrite(DAC_PIN, dac_sine_table[index]);
//         index = (index + 1) % TABLE_SIZE;
//     }
// }