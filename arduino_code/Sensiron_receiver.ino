//empfänger

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RFM69.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ============================================================================
// RFM69
// ============================================================================

#define NETWORKID       100
#define NODEID_DISPLAY  2

#define FREQUENCY RF69_868MHZ

#define RFM69_CS   8
#define RFM69_INT  3
#define RFM69_RST  4

#define ENCRYPTKEY "1234567890123456"

RFM69 radio(RFM69_CS, RFM69_INT, true);

// ============================================================================
// OLED
// ============================================================================

Adafruit_SH1107 display(64, 128, &Wire, -1);

#define OLED_ADDR 0x3C

// ============================================================================
// Buttons
// ============================================================================

#define BUTTON_A 9
#define BUTTON_B 6
#define BUTTON_C 5

// ============================================================================
// Datenpaket vom Sender
// ============================================================================

struct Sen66Packet {
  float pm1p0;
  float pm2p5;
  float pm4p0;
  float pm10p0;

  float humidity;
  float temperature;

  float vocIndex;
  float noxIndex;

  uint16_t co2;

  uint32_t counter;
};

// ============================================================================
// Lokale Anzeige-Daten
// ============================================================================

struct Sen66Values {
  float pm1p0 = NAN;
  float pm2p5 = NAN;
  float pm4p0 = NAN;
  float pm10p0 = NAN;

  float humidity = NAN;
  float temperature = NAN;

  float vocIndex = NAN;
  float noxIndex = NAN;

  uint16_t co2 = 0;

  bool valid = false;
};

Sen66Values v;

// ============================================================================

static const uint8_t PAGE_COUNT = 4;

uint8_t page = 0;
bool autoScroll = false;

bool lastA = true;
bool lastB = true;
bool lastC = true;

unsigned long lastAutoMs = 0;
unsigned long lastPacketMs = 0;

// ============================================================================

void showNoDataScreen() {
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 20);
  display.println("No");

  display.setCursor(0, 40);
  display.println("Data");

  display.display();
}

// ============================================================================

void drawTwoBigPortrait(
  const char* label1,
  float val1,
  const char* unit1,
  const char* label2,
  float val2,
  const char* unit2
) {

  display.clearDisplay();

  display.setTextWrap(false);
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("A< B> C:");
  display.print(autoScroll ? "ON" : "OFF");

  display.setCursor(0, 10);
  display.print("S ");
  display.print(page + 1);
  display.print("/");
  display.print(PAGE_COUNT);

  display.setCursor(0, 24);
  display.print(label1);

  display.setTextSize(2);
  display.setCursor(0, 34);
  display.print(val1, 1);

  display.setTextSize(1);
  display.setCursor(0, 52);
  display.print(unit1);

  display.setCursor(0, 72);
  display.print(label2);

  display.setTextSize(2);
  display.setCursor(0, 82);
  display.print(val2, 1);

  display.setTextSize(1);
  display.setCursor(0, 100);
  display.print(unit2);

  display.display();
}

// ============================================================================

void drawCo2BigPortrait(uint16_t co2ppm) {

  display.clearDisplay();

  display.setTextWrap(false);
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("A< B> C:");
  display.print(autoScroll ? "ON" : "OFF");

  display.setCursor(0, 10);
  display.print("S ");
  display.print(page + 1);
  display.print("/");
  display.print(PAGE_COUNT);

  display.setCursor(0, 30);
  display.print("CO2 ppm");

  display.setTextSize(3);
  display.setCursor(0, 48);
  display.print(co2ppm);

  display.display();
}

// ============================================================================

void drawPage() {

  if (!v.valid) {
    showNoDataScreen();
    return;
  }

  switch (page) {

    case 0:
      drawTwoBigPortrait(
        "PM2.5",
        v.pm2p5,
        "ug/m3",
        "PM10",
        v.pm10p0,
        "ug/m3"
      );
      break;

    case 1:
      drawTwoBigPortrait(
        "Temp",
        v.temperature,
        "C",
        "RH",
        v.humidity,
        "%"
      );
      break;

    case 2:
      drawTwoBigPortrait(
        "VOC idx",
        v.vocIndex,
        "",
        "NOx idx",
        v.noxIndex,
        ""
      );
      break;

    case 3:
      drawCo2BigPortrait(v.co2);
      break;
  }
}

// ============================================================================

void handleButtons() {

  bool a = digitalRead(BUTTON_A);
  bool b = digitalRead(BUTTON_B);
  bool c = digitalRead(BUTTON_C);

  if (lastA && !a) {
    page = (page == 0) ? PAGE_COUNT - 1 : page - 1;
    drawPage();
  }

  if (lastB && !b) {
    page = (page + 1) % PAGE_COUNT;
    drawPage();
  }

  if (lastC && !c) {
    autoScroll = !autoScroll;
    lastAutoMs = millis();
    drawPage();
  }

  lastA = a;
  lastB = b;
  lastC = c;
}

// ============================================================================

void autoScrollIfDue() {

  if (!autoScroll)
    return;

  if (millis() - lastAutoMs < 2000)
    return;

  lastAutoMs = millis();

  page = (page + 1) % PAGE_COUNT;

  drawPage();
}

// ============================================================================

void receiveRadio() {

  if (!radio.receiveDone())
    return;

  if (radio.DATALEN != sizeof(Sen66Packet))
    return;

  Sen66Packet pkt;

  memcpy(
    &pkt,
    (void*)radio.DATA,
    sizeof(Sen66Packet)
  );

  v.pm1p0 = pkt.pm1p0;
  v.pm2p5 = pkt.pm2p5;
  v.pm4p0 = pkt.pm4p0;
  v.pm10p0 = pkt.pm10p0;

  v.humidity = pkt.humidity;
  v.temperature = pkt.temperature;

  v.vocIndex = pkt.vocIndex;
  v.noxIndex = pkt.noxIndex;

  v.co2 = pkt.co2;

  v.valid = true;

  lastPacketMs = millis();

  Serial.print("Packet #");
  Serial.println(pkt.counter);

  drawPage();
}

// ============================================================================

void setup() {

  Serial.begin(115200);

  Wire.begin();

  // OLED

  if (!display.begin(OLED_ADDR, true)) {

    if (!display.begin(0x3D, true)) {

      while (1) {
        Serial.println("OLED init failed");
        delay(1000);
      }
    }
  }

  display.setRotation(2);

  display.clearDisplay();
  display.display();

  // Buttons

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // RFM69 Reset

  pinMode(RFM69_RST, OUTPUT);

  digitalWrite(RFM69_RST, LOW);
  delay(10);

  digitalWrite(RFM69_RST, HIGH);
  delay(10);

  digitalWrite(RFM69_RST, LOW);
  delay(10);

  // RFM69 Init

  radio.initialize(
    FREQUENCY,
    NODEID_DISPLAY,
    NETWORKID
  );

  radio.encrypt(ENCRYPTKEY);

  Serial.println("Display Empfaenger gestartet");

  showNoDataScreen();
}

// ============================================================================

void loop() {

  handleButtons();

  receiveRadio();

  autoScrollIfDue();

  // Timeout nach 10 Sekunden ohne Daten

  if (millis() - lastPacketMs > 10000) {
    v.valid = false;
  }

  delay(10);
}