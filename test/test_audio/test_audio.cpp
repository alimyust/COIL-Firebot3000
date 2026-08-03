
#include <Arduino.h>

#include "handlers/scheduler.h"
#include "handlers/ControllerHandler.hpp"
#include "handlers/AudioHandler.hpp"

#include "radio.h"

#include "Joystick.hpp"
#include "ProtocolCommands.h"
#include "Microphone.hpp"
#include "Speaker.hpp"

#include "display.h"

RadioComm radio(2, 434.0, 8, 3, 4); // Node 2 (Controller)
EventScheduler eventScheduler(radio, false);
// Joystick joystick_motor;
// Joystick joystick_turret;    
Microphone microphone;
Speaker speaker;
// DisplayOLED oled(false);

// Instantiate AudioHandler with debugging turned ON
AudioHandler audioHandler(eventScheduler, microphone, speaker, false);

// Diagnostic Counters
volatile uint32_t packets_processed = 0;
volatile uint32_t total_samples_queued = 0;

void setup() {
    Serial.begin(115200);
    
    // Wait up to 3 seconds for Serial Monitor connection
    uint32_t start_time = millis();
    while (!Serial && (millis() - start_time < 3000));

    Serial.println("\n==========================================");
    Serial.println(" SAMD21 Full-Duplex Local Loopback Test   ");
    Serial.println("==========================================");

    // 1. Initialize Hardware Speaker (DAC + TC3 Interrupt)
    speaker.begin();
    Serial.println("[OK] Speaker / DAC initialized at 8 kHz.");

    // 2. Initialize Hardware Microphone (ADC + EVSYS + TC4 Timer + DMA)
    microphone.begin();
    Serial.println("[OK] Microphone / ADC initialized at 8 kHz.");

    Serial.println("\n[STATUS] Passthrough active: Speak into mic to hear output on A0.\n");
}

void loop() {
    // ========================================================================
    // 1. AUDIO PASSTHROUGH LOOP
    // ========================================================================
    if (microphone.isBufferReady()) {
        int16_t raw_pcm_buffer[SAMPLE_BLOCK_LENGTH];
        
        // Fetch raw samples from the active DMA ping-pong buffer
        microphone.readActiveBuffer(raw_pcm_buffer);

        // --- HIJACK / PASSTHROUGH STEP ---
        // Instead of triggering onAudioTrigger() which broadcasts via radio,
        // we manually construct the audio packet and feed it straight back in.
        
        ProtocolCommands::RadioAudioPacket local_packet;
        static uint16_t seq_counter = 0;
        local_packet.sequence = seq_counter++;

        // Compress full block using G.711 u-law encoder
        const size_t encoded_samples = SAMPLE_BLOCK_LENGTH / 2;
        for (size_t i = 0; i < encoded_samples; i++) {
            local_packet.data[i] = audioHandler.encodeSample(raw_pcm_buffer[i]);
        }

        // Send immediately into the RX pipeline (Decompress + Queue to Speaker)
        audioHandler.processAudio(local_packet);

        // Update telemetry counters
        packets_processed++;
        total_samples_queued += encoded_samples;
    }

    // ========================================================================
    // 2. REAL-TIME DIAGNOSTIC METRICS (Printed every 2 seconds)
    // ========================================================================
    static uint32_t last_telemetry_time = 0;
    if (millis() - last_telemetry_time >= 2000) {
        last_telemetry_time = millis();

        Serial.print("[STATS] Packets Processed: ");
        Serial.print(packets_processed);
        Serial.print(" | Total Samples: ");
        Serial.print(total_samples_queued);
        Serial.print(" | Speaker Ring Buffer Level: ");
        Serial.print(speaker.getBufferCount()); // Ensure you have a getter or access to count
        Serial.println(" / 256");
    }
}