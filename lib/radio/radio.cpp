<<<<<<< HEAD
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
=======
#include "radio.h"
#include <string.h>

// Existing RF69_Comm implementation remains intact for compatibility.
RF69_Comm::RF69_Comm(uint8_t node_id, float frequency, uint8_t cs_pin,
                     uint8_t int_pin, uint8_t rst_pin)
  : _radio(cs_pin, int_pin), _node_id(node_id), _frequency(frequency),
    _rst_pin(rst_pin), _receive_handler(nullptr), _debug_enabled(false),
    _last_rssi(0) {}

bool RF69_Comm::begin(const uint8_t* sync_words, const char* encryption_key) {
>>>>>>> main
    pinMode(_rst_pin, OUTPUT);
    digitalWrite(_rst_pin, LOW);
    delay(10);
    digitalWrite(_rst_pin, HIGH);
    delay(10);
    digitalWrite(_rst_pin, LOW);
    delay(10);

<<<<<<< HEAD
    // Initialize radio
    if (!_radio.init()) {
        if (_debug_enabled) Serial.println("RFM95 init failed");
        return false;
    }
    
    // Set frequency
=======
    if (!_radio.init()) {
        if (_debug_enabled) Serial.println("RFM69 init failed");
        return false;
    }

>>>>>>> main
    if (!_radio.setFrequency(_frequency)) {
        if (_debug_enabled) Serial.println("Frequency set failed");
        return false;
    }
<<<<<<< HEAD
    
    // Configure radio settings
    _radio.setTxPower(20, true);  // 20dBm, high power module
    
    // Set sync words if provided
    if (sync_words != nullptr) {
        _radio.setSyncWords(sync_words, 2);
    }
    
    // Set encryption if provided
=======

    _radio.setTxPower(20, true);

    if (sync_words != nullptr) {
        _radio.setSyncWords(sync_words, 2);
    }

>>>>>>> main
    if (encryption_key != nullptr) {
        _radio.setEncryptionKey((uint8_t*)encryption_key);
    }

<<<<<<< HEAD
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
=======
    if (_debug_enabled) {
        Serial.println("Radio initialized successfully");
        Serial.print("Node ID: "); Serial.println(_node_id);
        Serial.print("Frequency: "); Serial.print(_frequency); Serial.println(" MHz");
    }

    return true;
}

>>>>>>> main
bool RF69_Comm::send(uint8_t receiver_id, uint8_t command, const char* message) {
    RF69_Packet packet;
    packet.sender_id = _node_id;
    packet.receiver_id = receiver_id;
    packet.command = command;
<<<<<<< HEAD
    
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
=======

    strncpy(packet.payload, message, sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';

    for (int i = 0; i < 3; i++) {
        if (_radio.send((uint8_t*)&packet, sizeof(packet))) {
            if (_radio.waitPacketSent(100)) {
                if (_debug_enabled) {
                    Serial.print("Sent to ");
                    Serial.print(receiver_id);
                    Serial.print(": ");
                    Serial.println(message);
                }

                _radio.setModeRx();
                return true;
            }
        }

        delay(random(10, 50));
    }

    if (_debug_enabled) {
        Serial.print("Failed to send to ");
        Serial.println(receiver_id);
>>>>>>> main
    }
    return false;
}

<<<<<<< HEAD
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
=======
void RF69_Comm::update() {
    if (_radio.available()) {
        RF69_Packet packet;
        uint8_t len = sizeof(packet);

        if (_radio.recv((uint8_t*)&packet, &len)) {
            _last_rssi = _radio.lastRssi();

            if (len == sizeof(packet)) {
                if (packet.receiver_id == _node_id || packet.receiver_id == RH_BROADCAST_ADDRESS) {
                    if (_debug_enabled) {
                        Serial.print("Received from ");
                        Serial.print(packet.sender_id);
                        Serial.print(" [RSSI:");
                        Serial.print(_last_rssi);
                        Serial.print("]:");
                        Serial.println(packet.payload);
                    }

                    if (_receive_handler) {
                        _receive_handler(packet);
                    }
                }
            }

            _radio.setModeRx();
>>>>>>> main
        }
    }
}

<<<<<<< HEAD
void RF69_Comm::enable_debug(bool enable) { _debug_enabled = enable; }
int16_t RF69_Comm::get_last_rssi() { return _last_rssi; }
void RF69_Comm::set_receive_handler(void (*handler)(RF69_Packet &packet)) { _receive_handler = handler; }
=======
void RF69_Comm::enable_debug(bool enable) {
    _debug_enabled = enable;
}

int16_t RF69_Comm::get_last_rssi() {
    return _last_rssi;
}

void RF69_Comm::set_receive_handler(void (*handler)(RF69_Packet &packet)) {
    _receive_handler = handler;
}

// New event-driven radio implementation.
EventRadioComm* EventRadioComm::s_instance = nullptr;

EventRadioComm::EventRadioComm(uint8_t node_id, float frequency, uint8_t cs_pin,
                               uint8_t int_pin, uint8_t rst_pin)
  : _radio(cs_pin, int_pin), _node_id(node_id), _frequency(frequency),
    _rst_pin(rst_pin), _debug_enabled(false), _test_mode(false), _last_rssi(0),
    _queue_head(0), _queue_tail(0), _queue_count(0),
    _tx_head(0), _tx_tail(0), _tx_count(0) {
    s_instance = this;
}

bool EventRadioComm::begin(const uint8_t* sync_words, const char* encryption_key) {
    pinMode(_rst_pin, OUTPUT);
    digitalWrite(_rst_pin, LOW);
    delay(10);
    digitalWrite(_rst_pin, HIGH);
    delay(10);
    digitalWrite(_rst_pin, LOW);
    delay(10);

    if (!_radio.init()) {
        if (_debug_enabled) Serial.println("Event radio init failed");
        return false;
    }

    if (!_radio.setFrequency(_frequency)) {
        if (_debug_enabled) Serial.println("Event radio frequency failed");
        return false;
    }

    _radio.setTxPower(20, true);

    if (sync_words != nullptr) {
        _radio.setSyncWords(sync_words, 2);
    }

    if (encryption_key != nullptr) {
        _radio.setEncryptionKey((uint8_t*)encryption_key);
    }

    _radio.setModeRx();
    return true;
}

bool EventRadioComm::send(uint8_t receiver_id, uint8_t command, const char* message) {
    if (_test_mode) {
        return true;
    }

    RF69_Packet packet;
    packet.sender_id = _node_id;
    packet.receiver_id = receiver_id;
    packet.command = command;

    strncpy(packet.payload, message, sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';

    enqueueTransmit(packet);
    return true;
}

void EventRadioComm::update() {
    if (_test_mode) {
        processTransmitQueue();
        return;
    }

    if (_radio.available()) {
        RF69_Packet packet;
        uint8_t len = sizeof(packet);

        if (_radio.recv((uint8_t*)&packet, &len)) {
            _last_rssi = _radio.lastRssi();
            if (len == sizeof(packet)) {
                if (packet.receiver_id == _node_id || packet.receiver_id == RH_BROADCAST_ADDRESS) {
                    handlePacket(packet);
                }
            }
            _radio.setModeRx();
        }
    }

    processTransmitQueue();
}

bool EventRadioComm::hasPendingEvent() const {
    return _queue_count > 0;
}

bool EventRadioComm::pollEvent(RadioEvent &out_event) {
    if (_queue_count == 0) {
        return false;
    }

    out_event = _event_queue[_queue_head];
    _queue_head = (_queue_head + 1) % EVENT_QUEUE_SIZE;
    _queue_count--;
    return true;
}

void EventRadioComm::queueTelemetryTick() {
    enqueueEvent(RadioEventType::TelemetryTick, RF69_Packet{});
}

void EventRadioComm::injectPacket(const RF69_Packet &packet) {
    enqueueEvent(RadioEventType::PacketReceived, packet);
}

void EventRadioComm::setTestMode(bool enable) {
    _test_mode = enable;
}

void EventRadioComm::enable_debug(bool enable) {
    _debug_enabled = enable;
}

int16_t EventRadioComm::get_last_rssi() {
    return _last_rssi;
}

void EventRadioComm::receiveCallback(RF69_Packet &packet) {
    if (s_instance != nullptr) {
        s_instance->handlePacket(packet);
    }
}

void EventRadioComm::enqueueEvent(RadioEventType type, const RF69_Packet &packet) {
    if (_queue_count >= EVENT_QUEUE_SIZE) {
        return;
    }

    RadioEvent event;
    event.type = type;
    event.packet = packet;

    _event_queue[_queue_tail] = event;
    _queue_tail = (_queue_tail + 1) % EVENT_QUEUE_SIZE;
    _queue_count++;
}

void EventRadioComm::enqueueTransmit(const RF69_Packet &packet) {
    if (_tx_count >= TX_QUEUE_SIZE) {
        return;
    }

    _tx_queue[_tx_tail] = packet;
    _tx_tail = (_tx_tail + 1) % TX_QUEUE_SIZE;
    _tx_count++;
}

void EventRadioComm::processTransmitQueue() {
    while (_tx_count > 0) {
        RF69_Packet packet = _tx_queue[_tx_head];
        _tx_head = (_tx_head + 1) % TX_QUEUE_SIZE;
        _tx_count--;

        if (_radio.send((uint8_t*)&packet, sizeof(packet))) {
            _radio.waitPacketSent(100);
            _radio.setModeRx();
        } else if (_debug_enabled) {
            Serial.println("Queued radio packet send failed");
        }
    }
}

void EventRadioComm::handlePacket(const RF69_Packet &packet) {
    enqueueEvent(RadioEventType::PacketReceived, packet);
}
>>>>>>> main
