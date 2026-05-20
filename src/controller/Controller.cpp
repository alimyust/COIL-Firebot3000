
// #include "Controller.hpp"

// namespace {

// constexpr char ENCRYPTION_KEY[] = "encryptionkey16";

// }

// Controller::Controller(uint8_t joyXPin, uint8_t joyYPin)
//     : _comm(CONTROLLER_NODE_ID, RF_FREQUENCY_MHZ),
//       _joyXPin(joyXPin),
//       _joyYPin(joyYPin),
//       _lastSendTime(0),
//       _lastThrottleDuty(50),
//       _lastSteeringDuty(50),
//       _hasSentState(false) {}

// void Controller::begin() {

//     pinMode(_joyXPin, INPUT);
//     pinMode(_joyYPin, INPUT);
//     Serial.println("Initializing controller...");
//     const bool radioStarted = _comm.begin(nullptr, ENCRYPTION_KEY);
//     if (!radioStarted) {
//         Serial.println("Controller radio init failed");
//         return;
//     }

//     Serial.println("Controller started");
// }

// void Controller::update() {
//     _comm.update();
//     Serial.print("Joystick X: ");
//     Serial.print(analogRead(_joyXPin));
//     Serial.print(" | Joystick Y: ");
//     Serial.println(analogRead(_joyYPin));
//     const unsigned long now = millis();
//     if (now - _lastSendTime < SEND_INTERVAL_MS) {
//         return;
//     }

//     const uint8_t steeringDuty = mapAnalogToDuty(_joyXPin);
//     const uint8_t throttleDuty = mapAnalogToDuty(_joyYPin, true);
//     const bool shouldSend =
//         !_hasSentState ||
//         steeringDuty != _lastSteeringDuty ||
//         throttleDuty != _lastThrottleDuty;

//     if (shouldSend) {
//         sendDuty(STEERING_DUTY, steeringDuty);
//         sendDuty(THROTTLE, throttleDuty);
//         _lastSteeringDuty = steeringDuty;
//         _lastThrottleDuty = throttleDuty;
//         _hasSentState = true;
//     }

//     _lastSendTime = now;
// }

// uint8_t Controller::mapAnalogToDuty(uint8_t analogPin, bool invert) const {
//     int reading = analogRead(analogPin);
//     if (invert) {
//         reading = 1023 - reading;
//     }

//     if (abs(reading - static_cast<int>(ANALOG_CENTER)) <= JOYSTICK_DEADZONE) {
//         return 50;
//     }

//     const long duty = map(reading, 0, 1023, 0, 100);
//     return static_cast<uint8_t>(constrain(duty, 0L, 100L));
// }

// bool Controller::sendDuty(uint8_t command, uint8_t duty) {
//     char payload[4];
//     snprintf(payload, sizeof(payload), "%u", duty);
//     return _comm.send(ROBOT_NODE_ID, command, payload);
// }

// Controller controller;
#include "Arduino.h"
void setup() {
    Serial.begin(115200);
    // controller.begin();
}

void loop() {
    Serial.println("Running in main loop");
    // controller.update();
}
