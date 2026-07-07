#include "radio.h"
#include <string.h>

RadioComm::RadioComm(uint8_t node_id, float frequency, uint8_t cs_pin,
                     uint8_t int_pin, uint8_t rst_pin)
  : _radio(cs_pin, int_pin), _node_id(node_id), _frequency(frequency),
    _rst_pin(rst_pin), _int_pin(int_pin), _tx_in_progress(false),
    _debug_enabled(false), _last_rssi(0),
    _rx_head(0), _rx_tail(0), _rx_count(0),
    _tx_head(0), _tx_tail(0), _tx_count(0) {}

bool RadioComm::begin(const uint8_t* sync_words, const char* encryption_key) {
    // 1. Hardware pin reset sequence for RFM69HCW
    pinMode(_rst_pin, OUTPUT);
    digitalWrite(_rst_pin, LOW);
    delay(10);
    digitalWrite(_rst_pin, HIGH);
    delay(10);
    digitalWrite(_rst_pin, LOW);
    delay(10);

    // 2. Initialize driver
    // RadioHead internally calculates vectors and registers its own ISR (isr0/1/2) 
    // to the hardware interrupt pin here. Do NOT attach an interrupt manually.
    if (!_radio.init()) {
        if (_debug_enabled) Serial.println("Radio init failed");
        return false;
    }

    // 3. Configure Carrier Frequency
    if (!_radio.setFrequency(_frequency)) {
        if (_debug_enabled) Serial.println("Frequency set failed");
        return false;
    }

    // 4. Configure Power Amplifier Parameters
    // 20dBm power configuration specifically matching high-power modules (RFM69HW/HCW)
    _radio.setTxPower(20, true);  

    // 5. Apply Network Addressing Sync Words
    if (sync_words != nullptr) {
        _radio.setSyncWords(sync_words, 2);
    }

    // 6. Apply Hardware AES Encryption Options
    if (encryption_key != nullptr) {
        _radio.setEncryptionKey((uint8_t*)encryption_key);
    }

    // 7. Establish initial background listening state
    _tx_in_progress = false;
    _radio.setModeRx();

    if (_debug_enabled) {
        Serial.println("Radio initialized successfully");
        Serial.print("Node ID: "); Serial.println(_node_id);
        Serial.print("Frequency: "); Serial.print(_frequency); Serial.println(" MHz");
        Serial.print("Interrupt Hooked internally to pin: "); Serial.println(_int_pin);
    }

    return true;
}

bool RadioComm::send(uint8_t receiver_id, uint8_t command, const char* message) {
    if (_tx_count >= RF69_TX_QUEUE_SIZE) {
        if (_debug_enabled) Serial.println("TX Rejected: Queue Full");
        return false;
    }

    RF69_OutboundPacket& packet = _tx_queue[_tx_tail];
    packet.receiver_id = receiver_id;
    packet.command = command;
    strncpy(packet.payload, message, sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';

    _tx_tail = (_tx_tail + 1) % RF69_TX_QUEUE_SIZE;
    _tx_count++;

    if (!_tx_in_progress) {
        return transmit_next_packet();
    }

    return true;
}

void RadioComm::update() {
    // Stage A: Handle Asynchronous Transmit State Machines
    // RadioHead's internal hardware ISR automatically shifts driver mode out of 
    // RHModeTx into RHModeIdle the exact microsecond the physical transmission completes.
    if (_tx_in_progress && _radio.mode() != RHGenericDriver::RHModeTx) {
        _radio.setModeRx(); // Re-arm local receiver circuits to listen for packets
        _tx_in_progress = false;
        if (_debug_enabled) Serial.println("Asynchronous TX Complete. Reverted to RX.");
    }

    if (!_tx_in_progress && _tx_count > 0) {
        transmit_next_packet();
    }

    // Stage B: Process Incoming FIFO Ring Buffering
    // Only pull packets if we are not actively executing a radio transmission.
    if (!_tx_in_progress) {
        while (_radio.available()) {
            RF69_Packet packet;
            uint8_t len = sizeof(packet);

            if (_radio.recv((uint8_t*)&packet, &len)) {
                _last_rssi = _radio.lastRssi();

                // Validate complete frame structures
                if (len == sizeof(packet)) {
                    // Filter matching node targets or system-wide broadcast frames
                    if (packet.receiver_id == _node_id || packet.receiver_id == RH_BROADCAST_ADDRESS) {
                        if (_debug_enabled) {
                            Serial.print("Received from "); Serial.print(packet.sender_id);
                            Serial.print(" [RSSI: "); Serial.print(_last_rssi); Serial.println("]");
                        }
                        push_received(packet, _last_rssi);
                    }
                } else if (_debug_enabled) {
                    Serial.print("Dropped bad frame length: "); Serial.println(len);
                }

                // Explicitly preserve uninterrupted RX streaming loops
                _radio.setModeRx();
            } else {
                break;
            }
        }
    }
}

bool RadioComm::transmit_next_packet() {
    if (_tx_in_progress || _tx_count == 0) {
        return false;
    }

    const RF69_OutboundPacket& queued_packet = _tx_queue[_tx_head];
    RF69_Packet packet;
    packet.sender_id = _node_id;
    packet.receiver_id = queued_packet.receiver_id;
    packet.command = queued_packet.command;
    strncpy(packet.payload, queued_packet.payload, sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';

    if (_radio.send((uint8_t*)&packet, sizeof(packet))) {
        _tx_in_progress = true;
        _tx_head = (_tx_head + 1) % RF69_TX_QUEUE_SIZE;
        _tx_count--;
        return true;
    }

    return false;
}

void RadioComm::push_received(const RadioComm::RF69_Packet& packet, int16_t rssi) {
    if (_rx_count >= RadioComm::RF69_RX_QUEUE_SIZE) {
        if (_debug_enabled) {
            Serial.println("RX queue full, dropping oldest packet");
        }
        _rx_head = (_rx_head + 1) % RadioComm::RF69_RX_QUEUE_SIZE;
        _rx_count--;
    }

    RadioComm::RF69_ReceivedPacket& slot = _rx_queue[_rx_tail];
    slot.packet = packet;
    slot.rssi = rssi;
    slot.timestamp_ms = millis();

    _rx_tail = (_rx_tail + 1) % RadioComm::RF69_RX_QUEUE_SIZE;
    _rx_count++;
}

bool RadioComm::available() const {
    return _rx_count > 0;
}

size_t RadioComm::queued_count() const {
    return _rx_count;
}

bool RadioComm::receive(RadioComm::RF69_ReceivedPacket& out) {
    if (_rx_count == 0) {
        return false;
    }

    out = _rx_queue[_rx_head];
    _rx_head = (_rx_head + 1) % RadioComm::RF69_RX_QUEUE_SIZE;
    _rx_count--;
    return true;
}

void RadioComm::enable_debug(bool enable) {
    _debug_enabled = enable;
}

int16_t RadioComm::get_last_rssi() const {
    return _last_rssi;
}