#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RFM69.h>
#include <Adafruit_PWMServoDriver.h>

// =========================================================
// PCA9685 (8ch/16ch I2C PWM Board) Setup
// =========================================================
static const uint8_t PCA9685_ADDR = 0x40;

// Ziel-Frequenz für Servos/ESC
static const float PWM_FREQ_HZ = 60.0f;

// Dein Board läuft ohne Kalibrierung stabil ca. 65 Hz, obwohl 60 eingestellt ist.
// -> Oszillator ist ~8.33% zu schnell.
// 25'000'000 * (65/60) = 27'083'333 Hz
static const uint32_t PCA_OSC_HZ_CAL = 27083333UL;

Adafruit_PWMServoDriver pwm(PCA9685_ADDR);

// PCA9685 Kanäle 0..7 (du nutzt hier 0..3)
static const uint8_t CH_THROTTLE = 0;  // PWM0 -> ESC
static const uint8_t CH_STEERING = 1;  // PWM1 -> Lenkservo
static const uint8_t CH_TURRET_X = 2;  // PWM2 -> Turret X
static const uint8_t CH_TURRET_Y = 3;  // PWM3 -> Turret Y

// Servo/ESC Pulsgrenzen (bei Bedarf anpassen)
static const uint16_t SERVO_MIN_US = 500;
static const uint16_t SERVO_MAX_US = 2500;

// Hilfsfunktionen: µs -> PCA counts (0..4095) bei eingestellter Frequenz
static inline uint16_t usToPCAcounts(uint16_t pulse_us, float freq_hz) {
  float counts_f = (pulse_us * freq_hz * 4096.0f) / 1000000.0f;
  if (counts_f < 0) counts_f = 0;
  if (counts_f > 4095) counts_f = 4095;
  return (uint16_t)(counts_f + 0.5f);
}

static void setPulseUS(uint8_t channel, uint16_t pulse_us) {
  pulse_us = constrain(pulse_us, SERVO_MIN_US, SERVO_MAX_US);
  uint16_t off = usToPCAcounts(pulse_us, PWM_FREQ_HZ);
  pwm.setPWM(channel, 0, off);
}

// Duty% (0..100) @ 60 Hz -> µs -> PCA
static void setDuty60Hz(uint8_t channel, float dutyPercent) {
  dutyPercent = constrain(dutyPercent, 0.0f, 100.0f);
  float period_us = 1000000.0f / PWM_FREQ_HZ;               // bei 60 Hz = 16666.7 µs
  uint16_t pulse_us = (uint16_t)((dutyPercent / 100.0f) * period_us + 0.5f);
  setPulseUS(channel, pulse_us);
}

// Winkel 0..180 -> Puls
static void setAngle(uint8_t channel, uint8_t angle) {
  angle = constrain(angle, 0, 180);
  uint16_t pulse = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
  setPulseUS(channel, pulse);
}

// =========================================================
// Toggle Output
// =========================================================
static const uint8_t PIN_SW_OUT = 6;
bool ledState = false;
uint8_t prevSw = 0;

// =========================================================
// RFM69 Setup
// =========================================================
#define RF69_FREQ       RF69_868MHZ
#define RFM69_CS        8
#define RFM69_INT       3
#define RFM69_RST       4
#define IS_RFM69HW      true

#define NETWORK_ID      100
#define CONTROLLER_ID   10
#define CAR_NODE_ID     1
const char* ENCRYPT_KEY = "encryptionkey16";

RFM69 radio(RFM69_CS, RFM69_INT, IS_RFM69HW, digitalPinToInterrupt(RFM69_INT));

// =========================================================
// Steuerwerte (Duty% @ 60 Hz, wie von Original-Fernbedienung gemessen)
// =========================================================
const float NEUTRAL_THROTTLE = 9.00f;
const float NEUTRAL_STEERING = 8.70f;

const float THROTTLE_MIN = 7.5f;
const float THROTTLE_MAX = 10.5f;

const float STEERING_MIN = 5.5f;
const float STEERING_MAX = 12.5f;

// Falls deine Lenkung "falsch herum" ist:
static const bool INVERT_STEERING = true;

// Failsafe
const uint32_t FAILSAFE_MS = 300;
uint32_t lastRxMs = 0;

// Packet (muss mit Sender identisch sein)
struct __attribute__((packed)) ControlPacket {
  int16_t throttle;      // duty * 1000
  int16_t steering;      // duty * 1000
  uint8_t sw;            // 1/0
  uint8_t turretX;       // 0..180
  uint8_t turretY;       // 0..180
};

static inline float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// Invertierung um Neutral, aber so, dass beide Enden voll genutzt werden
static float invertAroundNeutralFull(float in, float minV, float neutralV, float maxV) {
  in = clampf(in, minV, maxV);

  float left  = neutralV - minV;   // neutral -> min
  float right = maxV - neutralV;   // neutral -> max

  if (in <= neutralV) {
    float t = (neutralV - in) / left;   // 0..1
    return neutralV + t * right;
  } else {
    float t = (in - neutralV) / right;  // 0..1
    return neutralV - t * left;
  }
}

static void resetRadio() {
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);  delay(10);
  digitalWrite(RFM69_RST, HIGH); delay(10);
  digitalWrite(RFM69_RST, LOW);  delay(10);
}

static void applyNeutral() {
  setDuty60Hz(CH_THROTTLE, NEUTRAL_THROTTLE);
  setDuty60Hz(CH_STEERING, NEUTRAL_STEERING);
  setAngle(CH_TURRET_X, 90);
  setAngle(CH_TURRET_Y, 90);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_SW_OUT, OUTPUT);
  digitalWrite(PIN_SW_OUT, LOW);

  // I2C + PCA9685
  Wire.begin();
  pwm.begin();

  // Wichtig: erst Oszillator kalibrieren, dann Frequenz setzen
  pwm.setOscillatorFrequency(PCA_OSC_HZ_CAL);
  pwm.setPWMFreq(PWM_FREQ_HZ);
  delay(10);

  applyNeutral();

  // Radio
  resetRadio();
  radio.initialize(RF69_FREQ, CAR_NODE_ID, NETWORK_ID);
  radio.encrypt(ENCRYPT_KEY);
  if (IS_RFM69HW) radio.setHighPower();

  Serial.println("Car RX ready (PCA9685 @ 60Hz calibrated)");
}

void loop() {
  if (radio.receiveDone()) {
    if (radio.SENDERID == CONTROLLER_ID && radio.DATALEN == sizeof(ControlPacket)) {
      ControlPacket rx;
      memcpy(&rx, (void*)radio.DATA, sizeof(rx));

      // Toggle bei steigender Flanke
      if (rx.sw == 1 && prevSw == 0) ledState = !ledState;
      prevSw = rx.sw;
      digitalWrite(PIN_SW_OUT, ledState ? HIGH : LOW);

      // Throttle / Steering (Duty% @ 60Hz)
      float throttleDuty = rx.throttle / 1000.0f;
      float steeringDuty = rx.steering / 1000.0f;

      throttleDuty = clampf(throttleDuty, THROTTLE_MIN, THROTTLE_MAX);
      steeringDuty = clampf(steeringDuty, STEERING_MIN, STEERING_MAX);

      if (INVERT_STEERING) {
        steeringDuty = invertAroundNeutralFull(steeringDuty, STEERING_MIN, NEUTRAL_STEERING, STEERING_MAX);
      }

      setDuty60Hz(CH_THROTTLE, throttleDuty);
      setDuty60Hz(CH_STEERING, steeringDuty);

      // Turret (0..180) – wie bei dir invertiert
      uint8_t ax = constrain(rx.turretX, 0, 180);
      uint8_t ay = constrain(rx.turretY, 0, 180);
      ax = 180 - ax;
      ay = 180 - ay;

      setAngle(CH_TURRET_X, ax);
      setAngle(CH_TURRET_Y, ay);

      lastRxMs = millis();
    }

    if (radio.ACKRequested()) radio.sendACK();
  }

  // Failsafe
  if (millis() - lastRxMs > FAILSAFE_MS) {
    applyNeutral();
    digitalWrite(PIN_SW_OUT, LOW);
  }
}