#include "RFComm.hpp"
#include <string.h>

RF69_Comm::RF69_Comm(uint8_t node_id, float frequency, uint8_t cs_pin, 
                     uint8_t int_pin, uint8_t rst_pin) 
  : _radio(cs_pin, int_pin), _node_id(node_id), _frequency(frequency),
    _rst_pin(rst_pin), _receive_handler(nullptr), _debug_enabled(false),
    _last_rssi(0) {}

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
        if (_debug_enabled) Serial.println("RFM69 init failed");
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
    
    if (_debug_enabled) {
        Serial.println("Radio initialized successfully");
        Serial.print("Node ID: "); Serial.println(_node_id);
        Serial.print("Frequency: "); Serial.print(_frequency); Serial.println(" MHz");
    }
    
    return true;
}

bool RF69_Comm::send(uint8_t receiver_id, uint8_t command, const char* message) {
    // Create packet
    RF69_Packet packet;
    packet.sender_id = _node_id;
    packet.receiver_id = receiver_id;
    packet.command = command;
    
    // Copy message safely
    strncpy(packet.payload, message, sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';
    
    // Send with retries and random backoff
    for (int i = 0; i < 3; i++) {
        if (_radio.send((uint8_t*)&packet, sizeof(packet))) {
            if (_radio.waitPacketSent(100)) {  // Reduced timeout
                if (_debug_enabled) {
                    Serial.print("Sent to ");
                    Serial.print(receiver_id);
                    Serial.print(": ");
                    Serial.println(message);
                }
                
                // Put radio back in receive mode immediately
                _radio.setModeRx();
                return true;
            }
        }
        
        // Random backoff to avoid collisions
        delay(random(10, 50));
    }
    
    if (_debug_enabled) {
        Serial.print("Failed to send to ");
        Serial.println(receiver_id);
    }
    return false;
}

void RF69_Comm::update() {
    // Only process one packet per update to avoid flooding
    if (_radio.available()) {
        RF69_Packet packet;
        uint8_t len = sizeof(packet);
        
        if (_radio.recv((uint8_t*)&packet, &len)) {
            _last_rssi = _radio.lastRssi();
            
            // Validate packet length
            if (len == sizeof(packet)) {
                // Check if packet is for this node
                if (packet.receiver_id == _node_id || 
                    packet.receiver_id == RH_BROADCAST_ADDRESS) {
                    
                    if (_debug_enabled) {
                        Serial.print("Received from ");
                        Serial.print(packet.sender_id);
                        Serial.print(" [RSSI:");
                        Serial.print(_last_rssi);
                        Serial.print("]: ");
                        Serial.println(packet.payload);
                    }
                    
                    if (_receive_handler) {
                        _receive_handler(packet);
                    }
                }
            } else if (_debug_enabled) {
                Serial.print("Invalid packet length: ");
                Serial.print(len);
                Serial.print(" expected ");
                Serial.println(sizeof(packet));
            }
            
            // Put radio back in receive mode
            _radio.setModeRx();
        }
    }
}

void RF69_Comm::enable_debug(bool enable) {
    _debug_enabled = enable;
}

int16_t RF69_Comm::get_last_rssi() {
    return _last_rssi;
}
void RF69_Comm::set_receive_handler(void (*handler)(RF69_Packet &packet)) {
    _receive_handler = handler;
}