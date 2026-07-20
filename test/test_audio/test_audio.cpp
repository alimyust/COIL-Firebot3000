// #include <Arduino.h>
// #include <unity.h>

// // Include only your direct hardware abstraction layer logic
// #include "Microphone.hpp"
// #include "Speaker.hpp"

// // Setup hardware objects
// Microphone mic;
// Speaker speaker; // Ensure this maps to your DAC pin (e.g., Speaker speaker(A0) if needed)

// void setUp(void) {
//     // This runs before every single test case
// }

// void tearDown(void) {
//     // This runs after every single test case
// }

// // ============================================================================
// // TEST CASES
// // ============================================================================

// void test_peripherals_initialization(void) {
//     // Test that initializing hardware peripherals completes without crashing
//     speaker.begin();
//     mic.begin(); // Spawns the ADC configuration and DMAC Jobs
    
//     // If the board gets here without dropping off the native USB stack,
//     // your register modifications inside adc_init() and dma_init() are valid.
//     TEST_PASS();
// }

// void test_live_dmac_and_dac_loopback(void) {
//     Serial.println("\n==================================================");
//     Serial.println("RUNNING HARDWARE-ONLY AUDIO LOOPBACK");
//     Serial.println("Make noise into the microphone!");
//     Serial.println("Captured buffer blocks will copy directly to the Speaker.");
//     Serial.println("==================================================");

//     int16_t local_pcm_buffer[SAMPLE_BLOCK_LENGTH];
//     unsigned long start_time = millis();
//     uint32_t buffers_processed_count = 0;

//     // Run verification loop window for 5 seconds
//     while (millis() - start_time < 5000) {
        
//         // 1. Direct hardware flag check (Bypasses scheduler updates entirely)
//         if (mic.isBufferReady()) {
            
//             // 2. Read, center, and shift raw 12-bit unsigned ADC samples to signed 16-bit PCM
//             mic.readActiveBuffer(local_pcm_buffer);
            
//             // 3. Directly stream the raw samples out to the speaker hardware
//             for (int i = 0; i < SAMPLE_BLOCK_LENGTH; i++) {
//                 speaker.write(local_pcm_buffer[i]);
//             }
            
//             buffers_processed_count++;
//         }
        
//         yield(); // Feed native USB watchdogs to keep connection stable
//     }

//     // Print processing summary metrics to the test console
//     Serial.print("Total hardware buffer cycles completed: ");
//     Serial.println(buffers_processed_count);

//     // Assert that the DMAC actually filled the ping-pong destination registers at least once
//     TEST_ASSERT_TRUE_MESSAGE(buffers_processed_count > 0, "Hardware Failure: DMAC never flipped the buffer flag. Check your clock lines and peripheral triggers!");
// }

// // ============================================================================
// // MAIN UNITY TEST RUNNER ENTRYPOINT
// // ============================================================================
// void setup() {
//     // Wait for native USB serial connection to open safely
//     delay(2000); 
    
//     UNITY_BEGIN();
//     RUN_TEST(test_peripherals_initialization);
//     RUN_TEST(test_live_dmac_and_dac_loopback);
//     UNITY_END();
// }

// void loop() {
//     // Left empty for testing environments
// }