#include <Arduino.h>

#include "handlers/scheduler.h"
#include "handlers/ControllerHandler.hpp"
#include "handlers/AudioHandler.hpp"

#include "radio.h"

#include "Joystick.hpp"
#include "ProtocolCommands.h"
#include "Microphone.hpp"
#include "Speaker.hpp"

#include "display.h"

namespace ControllerPins {
    constexpr uint8_t JOYSTICK_MOTOR_X_PIN = A3;
    constexpr uint8_t JOYSTICK_MOTOR_Y_PIN = A2;
    constexpr uint8_t JOYSTICK_MOTOR_SWITCH_PIN = 13;

    constexpr uint8_t JOYSTICK_TURRET_X_PIN = A4;
    constexpr uint8_t JOYSTICK_TURRET_Y_PIN = A5;
    constexpr uint8_t JOYSTICK_TURRET_SWITCH_PIN = 12;

    constexpr uint8_t WALKIE_MUX_PIN = 10; // transistor control pin for walkie mux
    constexpr uint8_t WALKIE_STATE_PIN = 11; // button to enable talking
    // low listening, high sending

    constexpr uint8_t RADIO_CS_PIN = 8;
    constexpr uint8_t RADIO_INT_PIN = 3;
    constexpr uint8_t RADIO_RST_PIN = 4;

    // constexpr uint8_t AUDIO_MIC_ADC_PIN = A1;
    // constexpr uint8_t AUDIO_SPEAKER_DAC_PIN = A0;
}

RadioComm radio(2, 915.0, ControllerPins::RADIO_CS_PIN, ControllerPins::RADIO_INT_PIN, ControllerPins::RADIO_RST_PIN); // Node 2 (Controller)
EventScheduler scheduler(radio, true);
Joystick joystick_motor(ControllerPins::JOYSTICK_MOTOR_X_PIN, ControllerPins::JOYSTICK_MOTOR_Y_PIN, ControllerPins::JOYSTICK_MOTOR_SWITCH_PIN); // throttle, steering, camera_mux
Joystick joystick_turret(ControllerPins::JOYSTICK_TURRET_X_PIN, ControllerPins::JOYSTICK_TURRET_Y_PIN, ControllerPins::JOYSTICK_TURRET_SWITCH_PIN); // turret_x, turret_y, light_mux
// Microphone mic(ControllerPins::AUDIO_MIC_ADC_PIN);
// Speaker speaker(ControllerPins::AUDIO_SPEAKER_DAC_PIN);
DisplayOLED oled(true);

ControllerHandler controller_handler(scheduler, joystick_motor, joystick_turret, oled, true, ControllerPins::WALKIE_MUX_PIN, ControllerPins::WALKIE_STATE_PIN);
// AudioHandler audioHandler(scheduler, mic,speaker, false);

void setup() {

    Serial.begin(115200);
    // while(!Serial);

    radio.begin();
    joystick_motor.init_joystick();
    joystick_turret.init_joystick();
    oled.begin();
    pinMode(ControllerPins::WALKIE_MUX_PIN, OUTPUT);
    pinMode(ControllerPins::WALKIE_STATE_PIN, INPUT_PULLUP);
    scheduler.addPeriodicTask(
        50,
        EventPriority::PRIORITY_MEDIUM,
        ControllerHandler::onJoystickUpdate, 
        &controller_handler
    );

    scheduler.addPeriodicTask(
        1000, 
        EventPriority::PRIORITY_LOW, 
        ControllerHandler::onHeartbeat, 
        &controller_handler
    );

    scheduler.addPeriodicTask(
        250,
        EventPriority::PRIORITY_MEDIUM,
        ControllerHandler::onMuxUpdate, 
        &controller_handler
    );

    scheduler.registerPacketHandler(
        ProtocolCommands::CMD_SENSORS, 
        EventPriority::PRIORITY_MEDIUM, 
        ControllerHandler::onSensorReceived, 
        &controller_handler
    );
    Serial.println("Controller Setup Complete");

}
void loop() {
    radio.update();
    scheduler.update();
    oled.update();  

}