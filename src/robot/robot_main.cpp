#include <Arduino.h>
#include "DualHWPwm.hpp"

DualHardwarePWM pwm(9, 5);  // Pin 9 for drive, Pin 5 for steering

void setup() {
  pwm.begin(60);  // 60 Hz frequency
}

void loop() {
  pwm.setDutyCycle1(75);   // Drive at 75%
  pwm.setDutyCycle2(50);   // Steer at 50%
  delay(1000);
}
