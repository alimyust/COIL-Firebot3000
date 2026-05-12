#include <Arduino.h>

constexpr uint8_t DRIVE_PWM_PIN = 5;
constexpr uint8_t STEERING_PWM_PIN = 10;

constexpr uint8_t dutyPercentToAnalogValue(uint8_t percent) {
  return static_cast<uint8_t>((static_cast<uint16_t>(percent) * 255) / 100);
}

void setup() {
  pinMode(DRIVE_PWM_PIN, OUTPUT);
  pinMode(STEERING_PWM_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {

  analogWrite(STEERING_PWM_PIN, dutyPercentToAnalogValue(0));
  delay(200);
  analogWrite(STEERING_PWM_PIN, dutyPercentToAnalogValue(50));
  delay(200);
}
