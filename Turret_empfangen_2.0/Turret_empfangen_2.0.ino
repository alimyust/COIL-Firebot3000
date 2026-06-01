#include <Arduino.h>
#include <SPI.h>
#include <RFM69.h>
#include "wiring_private.h"   // pinPeripheral()
#include <Servo.h>

// ---------------- 60 Hz Servo PWM Setup (Feather M0 / SAMD21) ----------------
static const uint32_t SERVO_HZ       = 60;
static const uint32_t PRESCALER_DIV  = 64;
static const uint32_t CPU_HZ         = 48000000UL;

// LED / Toggle
bool ledState = false;
uint8_t prevSw = 0;

// 48 MHz / 64 = 750 kHz -> 750000 / 60 = 12500 counts -> PER = 12499
static const uint32_t PER_60HZ = (CPU_HZ / (PRESCALER_DIV * SERVO_HZ)) - 1;
static const float PERIOD_US_60HZ = 1000000.0f / 60.0f;  // 16666.666...

static inline uint32_t usToCounts(uint32_t pulse_us) {
  return (uint32_t)((pulse_us * (CPU_HZ / PRESCALER_DIV)) / 1000000UL);
}

// ---------------- TCC0 (für D10) auf 60 Hz ----------------
// Feather M0: D10 (PA18) -> TCC0/WO[2] über PIO_TIMER_ALT
static void setupTCC0_60Hz() {
  // GCLK for TCC0/TCC1
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC0_TCC1_Val) |
                                 GCLK_CLKCTRL_GEN_GCLK0 |
                                 GCLK_CLKCTRL_CLKEN);
  while (GCLK->STATUS.bit.SYNCBUSY) {}

  PM->APBCMASK.reg |= PM_APBCMASK_TCC0;

  TCC0->CTRLA.bit.ENABLE = 0;
  while (TCC0->SYNCBUSY.bit.ENABLE) {}

  TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  while (TCC0->SYNCBUSY.bit.WAVE) {}

  TCC0->PER.reg = PER_60HZ;
  while (TCC0->SYNCBUSY.bit.PER) {}

  TCC0->CTRLA.reg = (TCC0->CTRLA.reg & ~TCC_CTRLA_PRESCALER_Msk) |
                    TCC_CTRLA_PRESCALER_DIV64;

  TCC0->CTRLA.bit.ENABLE = 1;
  while (TCC0->SYNCBUSY.bit.ENABLE) {}
}

// ---------------- TCC2 (für D11) auf 60 Hz ----------------
static void setupTCC2_60Hz() {
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3_Val) |
                                 GCLK_CLKCTRL_GEN_GCLK0 |
                                 GCLK_CLKCTRL_CLKEN);
  while (GCLK->STATUS.bit.SYNCBUSY) {}

  PM->APBCMASK.reg |= PM_APBCMASK_TCC2;

  TCC2->CTRLA.bit.ENABLE = 0;
  while (TCC2->SYNCBUSY.bit.ENABLE) {}

  TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  while (TCC2->SYNCBUSY.bit.WAVE) {}

  TCC2->PER.reg = PER_60HZ;
  while (TCC2->SYNCBUSY.bit.PER) {}

  TCC2->CTRLA.reg = (TCC2->CTRLA.reg & ~TCC_CTRLA_PRESCALER_Msk) |
                    TCC_CTRLA_PRESCALER_DIV64;

  TCC2->CTRLA.bit.ENABLE = 1;
  while (TCC2->SYNCBUSY.bit.ENABLE) {}
}

// ---------------- Low-level: Pulsbreite in µs setzen (D10 oder D11) ----------------
void servoWriteMicroseconds60Hz(uint8_t pin, uint16_t pulse_us) {
  pulse_us = constrain(pulse_us, 500, 2500);

  uint32_t cc = usToCounts(pulse_us);
  if (cc > PER_60HZ) cc = PER_60HZ;

  // THROTTLE: D10 -> TCC0/WO[2] -> CC[2], MUX = PIO_TIMER_ALT
  if (pin == 10) {
    pinPeripheral(10, PIO_TIMER_ALT);
    TCC0->CC[2].reg = cc;
    while (TCC0->SYNCBUSY.bit.CC2) {}
  }
  // STEERING: D11 -> (Achtung: ist SPI MOSI auf Feather M0!)
  // Wenn es bei dir trotzdem laufen soll: D11 -> TCC2/WO[0] -> CC[0], MUX = PIO_TIMER
  else if (pin == 11) {
    pinPeripheral(11, PIO_TIMER);
    TCC2->CC[0].reg = cc;
    while (TCC2->SYNCBUSY.bit.CC0) {}
  }
}

// ---------------- High-level: Duty in % setzen (0..100) ----------------
void servoWriteDuty60Hz(uint8_t pin, float dutyPercent) {
  dutyPercent = constrain(dutyPercent, 0.0f, 100.0f);
  float pulse_us_f = (dutyPercent / 100.0f) * PERIOD_US_60HZ;
  uint16_t pulse_us = (uint16_t)(pulse_us_f + 0.5f);
  servoWriteMicroseconds60Hz(pin, pulse_us);
}

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
const char* ENCRYPT_KEY = "encryptionkey16"; // 16 Zeichen

// ---------------- Auto-Pins ----------------
static const uint8_t PIN_THROTTLE = 10;    // throttle (D10, TCC0)
static const uint8_t PIN_STEERING = 11;    // Steering (D11, TCC2)  Caution: SPI MOSI
static const uint8_t PIN_SW_OUT   = 6;     // Output (Toggle)

// ---------------- Turret-Pins (Servo Library) ----------
static const uint8_t PIN_TURRET_X = 9;
static const uint8_t PIN_TURRET_Y = 5;
Servo turretXServo;
Servo turretYServo;

// Neutral / Limits (wie beim Sender)
const float NEUTRAL_THROTTLE = 9.30f;
const float NEUTRAL_STEERING = 9.00f;

const float THROTTLE_MIN = 7.5f;
const float THROTTLE_MAX = 10.5f;

const float STEERING_MIN = 6.5f;
const float STEERING_MAX = 12.5f;

// Failsafe
const uint32_t FAILSAFE_MS = 300;

// ---------------- Packet (MUSS mit Sender identisch sein) ----------------
struct __attribute__((packed)) ControlPacket {
  int16_t throttle;      // duty * 1000
  int16_t steering;      // duty * 1000
  uint8_t sw;            // 1 = gedrückt, 0 = nicht gedrückt
  uint8_t turretX;       // 0..180
  uint8_t turretY;       // 0..180
};

RFM69 radio(RFM69_CS, RFM69_INT, IS_RFM69HW, digitalPinToInterrupt(RFM69_INT));

static inline float clampf(float v, float lo, float hi) {
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

uint32_t lastRxMs = 0;

void applyNeutral() {
  servoWriteDuty60Hz(PIN_THROTTLE, NEUTRAL_THROTTLE);
  servoWriteDuty60Hz(PIN_STEERING, NEUTRAL_STEERING);

  turretXServo.write(90);
  turretYServo.write(90);
}

void setup() {
  Serial.begin(115200);

  // Output Toggle
  pinMode(PIN_SW_OUT, OUTPUT);
  digitalWrite(PIN_SW_OUT, LOW);

  // 60Hz Timer starten (Auto)
  setupTCC0_60Hz();  // <-- Fix für D10
  setupTCC2_60Hz();  // für D11

  // Turret Servos starten (Servo Library)
  turretXServo.attach(PIN_TURRET_X);
  turretYServo.attach(PIN_TURRET_Y);
  turretXServo.write(90);
  turretYServo.write(90);

  applyNeutral();

  // Radio
  resetRadio();
  radio.initialize(RF69_FREQ, CAR_NODE_ID, NETWORK_ID);
  radio.encrypt(ENCRYPT_KEY);
  if (IS_RFM69HW) radio.setHighPower();

  Serial.println("Car RX ready (with turret)");
}

void loop() {
  if (radio.receiveDone()) {
    if (radio.SENDERID == CONTROLLER_ID && radio.DATALEN == sizeof(ControlPacket)) {
      ControlPacket rx;
      memcpy(&rx, (void*)radio.DATA, sizeof(rx));

      // --- Toggle bei steigender Flanke ---
      if (rx.sw == 1 && prevSw == 0) {
        ledState = !ledState;
      }
      prevSw = rx.sw;
      digitalWrite(PIN_SW_OUT, ledState ? HIGH : LOW);

      // --- Auto anwenden ---
// --- Auto anwenden ---
float throttleDuty = rx.throttle / 1000.0f;
float steeringDuty = rx.steering / 1000.0f;

// LENKUNG invertieren (um Neutral herum)
steeringDuty = 2.0f * NEUTRAL_STEERING - steeringDuty;

throttleDuty = clampf(throttleDuty, THROTTLE_MIN, THROTTLE_MAX);
steeringDuty = clampf(steeringDuty, STEERING_MIN, STEERING_MAX);

servoWriteDuty60Hz(PIN_THROTTLE, throttleDuty);
servoWriteDuty60Hz(PIN_STEERING, steeringDuty);

      // --- Turret anwenden ---
      uint8_t ax = constrain(rx.turretX, 0, 180);
      ax = 180 - ax;
      uint8_t ay = constrain(rx.turretY, 0, 180);
      ay = 180 - ay;
      turretXServo.write(ax);
      turretYServo.write(ay);

      lastRxMs = millis();
    }

    if (radio.ACKRequested()) {
      radio.sendACK();
    }
  }

  // Failsafe
  if (millis() - lastRxMs > FAILSAFE_MS) {
    applyNeutral();
    digitalWrite(PIN_SW_OUT, LOW);
  }
}