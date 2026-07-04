#include <Arduino.h>
#include "radio.h"
#include "scheduler.h"

// Hardware Pin Maps for Adafruit Feather M0 RFM69HCW
#define RFM69_CS      8
#define RFM69_INT     3
#define RFM69_RST     4
#define MOTOR_PWM_PIN 5
#define SERVO_PIN     6
#define TELEMETRY_PIN A0

// Local Network Identity Address Settings
const uint8_t NODE_ID = 1; 
const float FREQUENCY = 434.0;
const uint8_t SYNC_WORDS[2] = {0xDE, 0xAD};
const char* ENCRYPTION_KEY = "sampleEncryptKey";

// ============================================================================
// STATE & PIPELINE INITIALIZATION
// ============================================================================

// The Shared Source of Truth (Zero-initialized global state container)
VehicleTelemetry local_vehicle_state = {0, 0, 0};

// Instantiate Architecture Layer 1 and Layer 2
RadioComm radio(NODE_ID, FREQUENCY, RFM69_CS, RFM69_INT, RFM69_RST);
EventScheduler scheduler(radio);

// ============================================================================
// STAGE 3 HANDLERS: PURE BUSINESS LOGIC & HARDWARE INTERFACING
// ============================================================================

/**
 * @brief Processes incoming throttling commands. Blind to radio hardware.
 */
void handleThrottlePacket(const RadioComm::RF69_Packet& packet) {
    int speed_value = atoi(packet.payload);
    
    // Boundary check input parameters safely
    if (speed_value < 0) speed_value = 0;
    if (speed_value > 255) speed_value = 255;
    
    analogWrite(MOTOR_PWM_PIN, speed_value);
}

/**
 * @brief Processes incoming steering commands. Blind to radio hardware.
 */
void handleSteeringPacket(const RadioComm::RF69_Packet& packet) {
    int angle_value = atoi(packet.payload);
    
    if (angle_value < 0) angle_value = 0;
    if (angle_value > 180) angle_value = 180;
    
    analogWrite(SERVO_PIN, angle_value);
}

/**
 * @brief Periodic Task: Reads physical analog sensor hardware pins.
 * Does NOT look at, know about, or attempt to send via radio.
 */
void pollLocalSensors(VehicleTelemetry& state) {
    // Read local hardware directly
    int raw_analog = analogRead(TELEMETRY_PIN);
    
    // Commit values to the shared systemic memory model
    state.sensor_reading = raw_analog;
    state.last_update_time_ms = millis();
}

// ============================================================================
// STANDARD ARDUINO ENTRY & EXECUTION CORES
// ============================================================================

void setup() {
    Serial.begin(115200);
    
    // Configure peripheral pin directions
    pinMode(MOTOR_PWM_PIN, OUTPUT);
    pinMode(SERVO_PIN, OUTPUT);

    // Bootstrap Stage 1 Radio Drivers
    if (!radio.begin(SYNC_WORDS, ENCRYPTION_KEY)) {
        while (true); // Lock up locally if radio hardware is missing
    }
    radio.enable_debug(true);

    // Connect Stage 2 Event Types to Stage 3 Functional Blocks
    scheduler.registerPacketHandler(EventType::MOTOR_THROTTLE, handleThrottlePacket);
    scheduler.registerPacketHandler(EventType::MOTOR_STEERING, handleSteeringPacket);

    // Schedule internal clock generator: Poll every 20ms at Medium Priority
    scheduler.addPeriodicTask(EventType::SENSOR_POLL, EventPriority::PRIORITY_MEDIUM, 20, pollLocalSensors);
}

void loop() {
    // Keep Stage 1 background driver processing routines feeding hardware arrays
    radio.update();

    // Keep Stage 2 processing transitions executing asynchronously
    scheduler.update(local_vehicle_state);
}