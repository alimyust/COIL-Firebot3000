#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <unity.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32) && \
    !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) && \
    !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3) && \
    !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_NOPSRAM)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_NRF52832_FEATHER)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840, esp32-s2, esp32-s3, 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

void test_oled_hardware_connection(void) {
  // Return true on successful I2C handshake
  bool status = display.begin(0x3C, true);
  TEST_ASSERT_TRUE_MESSAGE(status, "OLED SH1107 failed to initialize at address 0x3C!");
}

void test_button_pins_setup(void) {
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, digitalRead(BUTTON_A), "Button A pin is LOW");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, digitalRead(BUTTON_B), "Button B pin is LOW");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, digitalRead(BUTTON_C), "Button C pin is LOW");
}

void setup() {
  delay(2000); // Allow board/serial port to settle

  UNITY_BEGIN();
  RUN_TEST(test_oled_hardware_connection);
  RUN_TEST(test_button_pins_setup);
  UNITY_END();

  // Prepare display for interactive loop testing
  display.clearDisplay();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  // Header frame
  display.setCursor(0, 0);
  display.println("--- OLED TEST ---");
  display.println("Press Buttons:");
  display.display();
}

void loop() {
  static uint32_t counter = 0;

  // Clear live dynamic region (y = 20 to bottom)
  display.fillRect(0, 20, 128, 44, SH110X_BLACK);
  display.setCursor(0, 20);

  // Print loop heartbeat counter
  display.print("Loop count: ");
  display.println(counter++);

  // Read live button states (0 = Pressed, 1 = Released)
  bool a_pressed = (digitalRead(BUTTON_A) == LOW);
  bool b_pressed = (digitalRead(BUTTON_B) == LOW);
  bool c_pressed = (digitalRead(BUTTON_C) == LOW);

  display.print("Buttons: ");
  if (a_pressed) display.print("[A] ");
  if (b_pressed) display.print("[B] ");
  if (c_pressed) display.print("[C] ");
  if (!a_pressed && !b_pressed && !c_pressed) display.print("None");
  display.println();

  // Render to display
  display.display();

  delay(100);
}