#include "radio.hpp"
#include <string.h>

// Initialize global thread-safe variables
volatile bool RF69_Comm::s_packet_ready = false;
volatile bool RF69_Comm::s_queue_overflow = false;

// Instantiate the static queue arrays
volatile RF69_Packet RF69_Comm::s_packet_queue[QUEUE_SIZE];
volatile int16_t     RF69_Comm::s_rssi_queue[QUEUE_SIZE];
volatile uint8_t     RF69_Comm::s_queue_head = 0;
volatile uint8_t     RF69_Comm::s_queue_tail = 0;

RF69_Comm::RF69_Comm(uint8_t node_id, float frequency, uint8_t cs_pin, 
                     uint8_t int_pin, uint8_t rst_pin) 
  : _radio(cs_pin, int_pin), _node_id(node_id), _frequency(frequency),
    _rst_pin(rst_pin), _int_pin(int_pin), _receive_handler(nullptr),
    _debug_enabled(false), _last_rssi(0) {}

bool RF69_Comm::begin(const uint8_t* sync_words, const char* encryption_key) {
    // Hardware reset
    pinMode(_rst_pin, OUTPUT);
    digitalWrite(_rst_pin, LOW);
    delay(10);
    digitalWrite(_rst_pin, HIGH);
    delay(10);
    digitalWrite(_rst_pin, LOW);
    delay(10);

    // Initialize radio
    if (!_radio.init()) {
        if (_debug_enabled) Serial.println("RFM95 init failed");
        return false;
    }
    
    // Set frequency
    if (!_radio.setFrequency(_frequency)) {
        if (_debug_enabled) Serial.println("Frequency set failed");
        return false;
    }
    
    // Configure radio settings
    _radio.setTxPower(20, true);  // 20dBm, high power module
    
    // Set sync words if provided
    if (sync_words != nullptr) {
        _radio.setSyncWords(sync_words, 2);
    }
    
    // Set encryption if provided
    if (encryption_key != nullptr) {
        _radio.setEncryptionKey((uint8_t*)encryption_key);
    }

    // Configure the DIO0 interrupt pin for packet-ready events.
    pinMode(_int_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(_int_pin), RF69_Comm::onRadioInterrupt, RISING);

    _radio.setModeRx();
    
    if (_debug_enabled) {
        Serial.println("Radio initialized successfully (Event-Driven Mode)");
    }
    
    return true;
}

// Low-power asynchronous transmit execution
bool RF69_Comm::send(uint8_t receiver_id, uint8_t command, const char* message) {
    RF69_Packet packet;
    packet.sender_id = _node_id;
    packet.receiver_id = receiver_id;
    packet.command = command;
    
    strncpy(packet.payload, message, sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';
    
    // Attempt non-blocking write to radio hardware
    if (_radio.send((uint8_t*)&packet, sizeof(packet))) {
        // While the hardware finishes transmitting, the CPU can run low power sleep cycles
        while (!_radio.waitPacketSent(10)) {
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; 
            __WFI(); // Wake only on radio hardware finishing transmission flag
        }
        _radio.setModeRx(); // Flip straight back to listening
        return true;
    }
    return false;
}

// --- THE HARDWARE INTERRUPT (ISR) ---
// Keep it lightning-fast. Extract the packet immediately, copy to ring queue, exit.
void RF69_Comm::onRadioInterrupt() {
    // We utilize an instance-free raw register access context because this is a static ISR.
    // Note: Since _radio is an instance variable on your class, we interact with the hardware 
    // cleanly inside your process method OR access a singleton pointer if you want direct extraction here.
    s_packet_ready = true;
}

// --- THE NON-BLOCKING PROCESSING LOOP ---
// This completely supersedes your polling update() loop.
void RF69_Comm::processEvents() {
    // 1. Check if the hardware interrupt marked a raw packet arrival
    if (s_packet_ready) {
        s_packet_ready = false;
        
        while (_radio.available()) {
            RF69_Packet temp_packet;
            uint8_t len = sizeof(temp_packet);
            
            if (_radio.recv((uint8_t*)&temp_packet, &len)) {
                _last_rssi = _radio.lastRssi();
                
                // Address filtering and length constraints matching your business logic
                if (len == sizeof(temp_packet) && 
                   (temp_packet.receiver_id == _node_id || temp_packet.receiver_id == 0xFF)) {
                    
                    // CRITICAL: Push packet data straight into your Ring Queue
                    uint8_t next_head = (s_queue_head + 1) % QUEUE_SIZE;
                    if (next_head != s_queue_tail) {
                        s_packet_queue[s_queue_head] = temp_packet;
                        s_rssi_queue[s_queue_head] = _last_rssi;
                        s_queue_head = next_head;
                    } else {
                        s_queue_overflow = true; // Flag structural data pressure
                    }
                }
                _radio.setModeRx();
            } else {
                break;
            }
        }
    }

    // 2. Dispatch events waiting in the queue down to your application handlers sequentially
    while (s_queue_head != s_queue_tail) {
        // Pop an object off the queue
        RF69_Packet current_packet = s_packet_queue[s_queue_tail];
        _last_rssi = s_rssi_queue[s_queue_tail];
        s_queue_tail = (s_queue_tail + 1) % QUEUE_SIZE;

        if (_debug_enabled) {
            Serial.print("[Event Dispatched] Cmd: "); Serial.println(current_packet.command);
        }

        // Fire off your user code actions dynamically
        if (_receive_handler != nullptr) {
            _receive_handler(current_packet);
        }
    }
}

void RF69_Comm::enable_debug(bool enable) { _debug_enabled = enable; }
int16_t RF69_Comm::get_last_rssi() { return _last_rssi; }
void RF69_Comm::set_receive_handler(void (*handler)(RF69_Packet &packet)) { _receive_handler = handler; }