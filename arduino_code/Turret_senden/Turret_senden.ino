#include <SPI.h>
#include <RFM69.h>
#include <Arduino.h>

// ---------------- RFM69 Pins/Frequenz ----------------
#define RF69_FREQ       RF69_868MHZ
#define RFM69_CS        8
#define RFM69_INT       3
#define RFM69_RST       4
#define IS_RFM69HW      true

// ---------------- Network/IDs/Encryption --------------
#define NETWORK_ID      100
#define CONTROLLER_ID   10
#define CAR_NODE_ID     1
const char* ENCRYPT_KEY = "encryptionkey16"; // genau 16 Zeichen

// ---------------- Controller Inputs (Auto) ------------
const int LpinX = A2;   // Steering
const int LpinY = A3;   // Throttle

// ---------------- Controller Inputs (Turret) ----------
const int TURRET_X_PIN = A0;
const int TURRET_Y_PIN = A1;

// Joystick Button (SW) -> an D6 anschließen, SW nach GND
const uint8_t SW_PIN = 6;

const int thresh = 25;
const int midpointLY = 505;
const int midpointLX = 498;

// Endzone (Randbereich)
const int ENDZONE_X = 40;   // Lenkung

// DutyCycles (in %)
const float NEUTRAL_THROTTLE = 9.00f;
const float NEUTRAL_STEERING = 9.00f;

const float THROTTLE_MIN = 7.5f;
const float THROTTLE_MAX = 10.5f;

const float STEERING_MIN = 6.5f;
const float STEERING_MAX = 12.5f;

// Sendeintervall (ms) – 20ms ist für Turret deutlich smoother als 50ms
const unsigned long TX_INTERVAL_MS = 20;

// ---------------- Turret Filter wie im Original -------
float fx = 90.0f;
float fy = 90.0f;

static int mapJoyToAngle(int v) {
  v = constrain(v, 0, 1023);
  int angle = map(v, 0, 1023, 0, 180);

  const int center = 90;
  const int dead = 10; // Grad
  if (abs(angle - center) <= dead) angle = center;

  return constrain(angle, 0, 180);
}

// ---------------- Packet ----------------
struct __attribute__((packed)) ControlPacket {
  int16_t throttle;      // duty * 1000
  int16_t steering;      // duty * 1000
  uint8_t sw;            // 1 = gedrückt, 0 = nicht gedrückt

  // NEW: Turret
  uint8_t turretX;       // 0..180
  uint8_t turretY;       // 0..180
};
ControlPacket txData;

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
  digitalWrite(RFM69_RST, LOW);  delay(10);
  digitalWrite(RFM69_RST, HIGH); delay(10);
  digitalWrite(RFM69_RST, LOW);  delay(10);
}

unsigned long lastTx = 0;

void setup() {
  Serial.begin(115200);

  // Feather M0: 10-bit ist Standard, wir setzen es explizit
  analogReadResolution(10);

  // SW ist active LOW -> Pullup aktivieren
  pinMode(SW_PIN, INPUT_PULLUP);

  resetRadio();

  radio.initialize(RF69_FREQ, CONTROLLER_ID, NETWORK_ID);
  radio.encrypt(ENCRYPT_KEY);
  if (IS_RFM69HW) radio.setHighPower();

  // Initialwerte Turret
  txData.turretX = 90;
  txData.turretY = 90;

  Serial.println("Controller TX ready (with turret)");
}

void loop() {
  unsigned long now = millis();
  if (now - lastTx < TX_INTERVAL_MS) return;
  lastTx = now;

  // -------- Auto Joysticks ----------
  int LjoyX = analogRead(LpinX);
  LjoyX = 1024- LjoyX;
  int LjoyY = analogRead(LpinY);

  float steeringDuty = NEUTRAL_STEERING;
  float throttleDuty = NEUTRAL_THROTTLE;

  // --- SW lesen und mitsenden ---
  bool swPressed = (digitalRead(SW_PIN) == LOW); // active LOW
  txData.sw = swPressed ? 1 : 0;

  // --- Lenkung mit Endzone ---
  if (LjoyX >= (1023 - ENDZONE_X)) {
    steeringDuty = STEERING_MAX;
  } else if (LjoyX <= ENDZONE_X) {
    steeringDuty = STEERING_MIN;
  } else if (abs(LjoyX - midpointLX) > thresh) {
    steeringDuty = (float)mapDouble(LjoyX, 0, 1023, STEERING_MIN, STEERING_MAX);
  }

  // --- Gas (ohne Endzone) ---
  if (abs(LjoyY - midpointLY) > thresh) {
    throttleDuty = (float)mapDouble(LjoyY, 0, 1023, THROTTLE_MIN, THROTTLE_MAX);
  }

  steeringDuty = clampf(steeringDuty, 0, 100);
  throttleDuty = clampf(throttleDuty, 0, 100);
  throttleDuty = 2.0f * NEUTRAL_THROTTLE - throttleDuty;

  txData.throttle = (int16_t)(throttleDuty * 1000.0f);
  txData.steering = (int16_t)(steeringDuty * 1000.0f);

  // -------- Turret Joysticks (wie dein Original) ----------
  int rawX = analogRead(TURRET_X_PIN);
  int rawY = analogRead(TURRET_Y_PIN);

  int targetX = mapJoyToAngle(rawX);
  int targetY = mapJoyToAngle(rawY);

  // Low-Pass Filter
  fx = fx * 0.85f + targetX * 0.15f;
  fy = fy * 0.85f + targetY * 0.15f;

  txData.turretX = (uint8_t)constrain((int)(fx + 0.5f), 0, 180);
  txData.turretY = (uint8_t)constrain((int)(fy + 0.5f), 0, 180);

  // -------- Senden ----------
  radio.send(CAR_NODE_ID, (const void*)&txData, sizeof(txData));
}