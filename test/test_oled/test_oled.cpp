#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map to different pins depending on board:
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
#else // 32u4, M0, M4, nrf52840, esp32-s2, esp32-s3 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

void setup() {
  Serial.begin(115200);

  Serial.println("128x64 OLED FeatherWing test");
  delay(250); // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  Serial.println("Button test");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.print("Connecting to SSID\n'adafruit':");
  display.print("connected!");
  display.println("IP: 10.0.1.23");
  display.println("Sending val #0");
  display.display(); // actually display all of the above
}

void loop() {
  // if(!digitalRead(BUTTON_A)) display.print("A");
  // if(!digitalRead(BUTTON_B)) display.print("B");
  // if(!digitalRead(BUTTON_C)) display.print("C");
  // delay(10);
  // yield();
  display.println("Sending val #0");
  int bufferSize = 1024; 
  for (int i = 0; i < bufferSize; i++) {
      Serial.print(display.getBuffer()[i]);
      Serial.print(" "); // Adds a space between elements for readability
  }
  // display.display();
}






// #include <Arduino.h>
// #include <unity.h>
// #include <Wire.h>
// #include "display.h"

// // Instantiate your display driver with debug enabled
// DisplayOLED dmaDisplay(true);

// void setUp(void) {
//     // Reset state variables before every single test
//     dmaDisplay._is_pushing_frame = false;
//     dmaDisplay._waiting_for_dma = false;
//     dmaDisplay._current_page = 0;
// }

// void tearDown(void) {
//     // Clean up after tests
// }

// // ============================================================================
// // TEST 1: Frame Push Initialization
// // ============================================================================
// void test_push_frame_starts_cleanly(void) {
//     // Ensure that requesting a frame sets the exact correct starting states
//     dmaDisplay.pushFrame();
    
//     TEST_ASSERT_TRUE_MESSAGE(dmaDisplay._is_pushing_frame, "Push flag was not set to true!");
//     TEST_ASSERT_FALSE_MESSAGE(dmaDisplay._waiting_for_dma, "Waiting for DMA should initially be false.");
//     TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, dmaDisplay._current_page, "Page counter did not reset to 0.");
// }

// // ============================================================================
// // TEST 2: The Page 8 Boundary and Lock Release
// // ============================================================================
// void test_page_8_boundary_release(void) {
//     dmaDisplay.pushFrame();
    
//     // Simulate the hardware progressing through all 8 pages (0 through 7)
//     for (uint8_t i = 0; i < 8; i++) {
//         // Manually force the flags to simulate what update() does right before DMA starts
//         dmaDisplay._waiting_for_dma = true;
        
//         // Call the static ISR callback (Simulating the DMA finishing 128 bytes)
//         DisplayOLED::dma_isr_callback(nullptr);
        
//         // The callback should have incremented the page and cleared the DMA wait flag
//         TEST_ASSERT_EQUAL_UINT8_MESSAGE(i + 1, dmaDisplay._current_page, "Page did not increment correctly in ISR.");
//         TEST_ASSERT_FALSE_MESSAGE(dmaDisplay._waiting_for_dma, "DMA wait flag was not cleared by ISR.");
//     }
    
//     // At this exact moment, _current_page is 8, and _is_pushing_frame is still true.
//     // The next call to update() must catch this boundary and unlock the system.
//     dmaDisplay.update();
    
//     TEST_ASSERT_FALSE_MESSAGE(dmaDisplay._is_pushing_frame, "CRITICAL: State machine did not unlock at Page 8!");
//     TEST_ASSERT_FALSE_MESSAGE(dmaDisplay._waiting_for_dma, "DMA flag stuck on at termination.");
// }

// // ============================================================================
// // TEST 3: Back-to-Back Frame Protection
// // ============================================================================
// void test_back_to_back_frame_rejection(void) {
//     dmaDisplay.pushFrame();
    
//     // Simulate being halfway through a frame (Page 4)
//     dmaDisplay._current_page = 4;
//     dmaDisplay._waiting_for_dma = true;
    
//     // Attempt to push another frame while busy
//     dmaDisplay.pushFrame();
    
//     // It should have ignored the call and preserved the current state
//     TEST_ASSERT_EQUAL_UINT8_MESSAGE(4, dmaDisplay._current_page, "PushFrame unlawfully reset the page counter mid-transfer!");
//     TEST_ASSERT_TRUE_MESSAGE(dmaDisplay._waiting_for_dma, "PushFrame unlawfully wiped the DMA wait flag!");
// }

// // ============================================================================
// // MAIN RUNNER
// // ============================================================================
// void setup() {
//     delay(2000); 
//     // UNITY_BEGIN();
    
//     // RUN_TEST(test_push_frame_starts_cleanly);
//     // RUN_TEST(test_page_8_boundary_release);
//     // RUN_TEST(test_back_to_back_frame_rejection);
    
//     // UNITY_END();

//     dmaDisplay.begin();
// }

// void loop() {
//     // // Tests complete in setup
//     dmaDisplay.display.println("Hello World");
//     dmaDisplay.pushFrame();
//     dmaDisplay.update();
// }