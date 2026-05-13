

#include <SPI.h>
#include <RFM69.h>
// ---------------- RFM69 pins/frequency ----------------
#define RF69_FREQ RF69_868MHZ
#define RFM69_CS 8
#define RFM69_INT 3
#define RFM69_RST 4
// Warning: set true only for RFM69HW/HCW (high power). Otherwise: false.
#define IS_RFM69HW true
// ---------------- Network/IDs/encryption --------------
#define NETWORK_ID 100
#define CONTROLLER_ID 10
#define CAR_NODE_ID 1
const char* ENCRYPT_KEY = "encryptionkey16"; // must be exactly 16 characters
// ---------------- Controller inputs -------------------
const int LpinX = A3; // Steering
const int LpinY = A2; // Throttle
const int thresh = 25;
const int midpointLY = 505;
const int midpointLX = 498;
// Duty cycles (in %)
const float NEUTRAL_THROTTLE = 9.00f;
const float NEUTRAL_STEERING = 9.00f;
const float THROTTLE_MIN = 7.0f;
const float THROTTLE_MAX = 11.0f;
const float STEERING_MIN = 6.5f;
const float STEERING_MAX = 12.5f;
const unsigned long TX_INTERVAL_MS = 50;
// ---------------- Packet ----------------
// Values are transmitted as duty * 1000 to keep 3 decimal digits as integers.
struct ControlPacket {
int16_t throttle; // duty * 1000
int16_t steering; // duty * 1000
};

ControlPacket txData;
// Radio object (interruptNum = digitalPinToInterrupt(INT_PIN) works on SAMD boards)
RFM69 radio(RFM69_CS, RFM69_INT, IS_RFM69HW, digitalPinToInterrupt(RFM69_INT));
double mapDouble(double x, double in_min, double in_max, double out_min, double out_max) {
return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float clampf(float v, float lo, float hi) {
if (v < lo) return lo;
if (v > hi) return hi;
return v;
}
void resetRadio() {
pinMode(RFM69_RST, OUTPUT);
digitalWrite(RFM69_RST, LOW);
delay(10);
digitalWrite(RFM69_RST, HIGH);
delay(10);
digitalWrite(RFM69_RST, LOW);
delay(10);
}
unsigned long lastTx = 0;
void setup() {
Serial.begin(115200);
// Hardware reset for the RFM69 module
resetRadio();
// Initialize radio and encryption
radio.initialize(RF69_FREQ, CONTROLLER_ID, NETWORK_ID);
radio.encrypt(ENCRYPT_KEY);
// Enable high-power mode if using RFM69HW/HCW
if (IS_RFM69HW) radio.setHighPower();
Serial.println("Controller TX ready");
}


void loop() {
unsigned long now = millis();
if (now - lastTx < TX_INTERVAL_MS) return;
lastTx = now;
// Read joystick values
int LjoyX = analogRead(LpinX);
int LjoyY = analogRead(LpinY);
// Start with neutral duty cycles
float steeringDuty = NEUTRAL_STEERING;
float throttleDuty = NEUTRAL_THROTTLE;
// Steering: only update if outside deadband
if (abs(LjoyX - midpointLX) > thresh) {
steeringDuty = (float)mapDouble(LjoyX, 0, 1023, STEERING_MIN, STEERING_MAX);
}
// Throttle: only update if outside deadband
if (abs(LjoyY - midpointLY) > thresh) {
throttleDuty = (float)mapDouble(LjoyY, 0, 1023, THROTTLE_MIN, THROTTLE_MAX);
}
// Clamp to a safe range
steeringDuty = clampf(steeringDuty, 0, 100);
throttleDuty = clampf(throttleDuty, 0, 100);
// Pack into integers (duty * 1000)
txData.throttle = (int16_t)(throttleDuty * 1000.0f);
txData.steering = (int16_t)(steeringDuty * 1000.0f);
// Send to car node
radio.send(CAR_NODE_ID, (const void*)&txData, sizeof(txData));
}