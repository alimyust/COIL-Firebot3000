

// #include "Controller.hpp"
// #include "Display.hpp"
#include "ProtocolLayer.hpp"
#include "Arduino.h"


// #include "Joystick.hpp"

// Joystick joy_x(A3, A2, true);

// void setup() {
//     // Serial.begin(115200);
//     // while (!Serial) {}
//     // joy_x.init_joystick();
// }

// void loop() {
//     // int x = 0, y = 0;
//     // // joy_x.update_joystick(x, y);
//     // Serial.print("X=");
//     //  Serial.print(x);
//     // Serial.print(" Y="); Serial.println(y);
//     // delay(100);
// }



// Functional PWM demo

// #include <Arduino.h>
// #include "DualHWPwm.hpp"

// namespace {
// constexpr uint8_t DRIVE_PWM_PIN = 9;
// constexpr uint8_t STEER_PWM_PIN = 5;
// constexpr uint32_t PWM_FREQUENCY_HZ = 60;
// constexpr uint32_t STEP_DELAY_MS = 2000;

// constexpr uint8_t THROTTLE_MIN_DUTY = 7;
// constexpr uint8_t THROTTLE_NEUTRAL_DUTY = 9;
// constexpr uint8_t THROTTLE_MAX_DUTY = 10;

// constexpr uint8_t STEERING_MIN_DUTY = 6;
// constexpr uint8_t STEERING_NEUTRAL_DUTY = 9;
// constexpr uint8_t STEERING_MAX_DUTY = 12;

// DualHardwarePWM pwm(DRIVE_PWM_PIN, STEER_PWM_PIN);

// void setOutputs(uint8_t throttleDuty, uint8_t steeringDuty, const char* label) {
//     pwm.setDutyCycle1(throttleDuty);
//     pwm.setDutyCycle2(steeringDuty);

//     Serial.print(label);
//     Serial.print(" throttle=");
//     Serial.print(throttleDuty);
//     Serial.print("% steering=");
//     Serial.print(steeringDuty);
//     Serial.println("%");
// }

// }

// void setup() {
//     Serial.begin(115200);
//     while (!Serial) {}

//     pwm.begin(PWM_FREQUENCY_HZ);
//     setOutputs(THROTTLE_NEUTRAL_DUTY, STEERING_NEUTRAL_DUTY, "Startup neutral");
//     delay(STEP_DELAY_MS);
// }

// void loop() {
//     setOutputs(THROTTLE_NEUTRAL_DUTY, STEERING_NEUTRAL_DUTY, "Neutral");
//     delay(STEP_DELAY_MS);

//     setOutputs(THROTTLE_MIN_DUTY, STEERING_NEUTRAL_DUTY, "Throttle min");
//     delay(STEP_DELAY_MS);

//     setOutputs(THROTTLE_MAX_DUTY, STEERING_NEUTRAL_DUTY, "Throttle max");
//     delay(STEP_DELAY_MS);

//     setOutputs(THROTTLE_NEUTRAL_DUTY, STEERING_MIN_DUTY, "Steering min");
//     delay(STEP_DELAY_MS);

//     setOutputs(THROTTLE_NEUTRAL_DUTY, STEERING_MAX_DUTY, "Steering max");
//     delay(STEP_DELAY_MS);
// }
