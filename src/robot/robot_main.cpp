#include <Arduino.h>

// constexpr uint8_t DRIVE_PWM_PIN = 5;
// constexpr uint8_t STEERING_PWM_PIN = 10;

// constexpr uint8_t dutyPercentToAnalogValue(uint8_t percent) {
//   return static_cast<uint8_t>((static_cast<uint16_t>(percent) * 255) / 100);
// }

// void setup() {
//   pinMode(DRIVE_PWM_PIN, OUTPUT);
//   pinMode(STEERING_PWM_PIN, OUTPUT);
//   Serial.begin(9600);
// }

// void loop() {

//   analogWrite(STEERING_PWM_PIN, dutyPercentToAnalogValue(0));
//   delay(200);
//   analogWrite(STEERING_PWM_PIN, dutyPercentToAnalogValue(50));
//   delay(200);
// }



// https://forum.arduino.cc/index.php?topic=590442

byte servoPin = 11;
int counter = 0;
void setup()
{
  Serial.begin(9600);
  Serial.println("poor man's servo sweep");

  //turn off L13
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(servoPin, OUTPUT);
  digitalWrite(servoPin, LOW);

  // pinMode(buttonPin, INPUT_PULLUP);

} //setup

void loop()
{
  Serial.print("one side...");
  while(counter++ < 5) {
    digitalWrite(servoPin, HIGH);
    delayMicroseconds(1900);    //position
    digitalWrite(servoPin, LOW);
    delayMicroseconds(18100);   //balance of 20000 cycle
  }
  Serial.print("other side...");
  while(counter++ < 10) {
    digitalWrite(servoPin, HIGH);
    delayMicroseconds(1100);    //position
    digitalWrite(servoPin, LOW);
    delayMicroseconds(18900);   //balance of 20000 cycle

} //loop