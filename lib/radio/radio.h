#ifndef RADIO_HPP
#define RADIO_HPP

#include <Arduino.h>
#include <RH_RF69.h>

class RadioComm {
public:
    // Ring buffer size for asynchronous packet containment
    static const size_t RF69_RX_QUEUE_SIZE = 8;
    static const size_t RF69_TX_QUEUE_SIZE = 4;
    
    // Explicit maximum payload payload size safely fitting inside RadioHead's limits
    static const size_t RF69_MAX_PAYLOAD_LEN = 50;

    // Structured raw over-the-air packet format
    struct RF69_Packet {
        uint8_t sender_id;
        uint8_t receiver_id;
        uint8_t command;
        char payload[RF69_MAX_PAYLOAD_LEN];
    };

    // Meta-wrapped packet stored inside the ring buffer for Stage 2 processing
    struct RF69_ReceivedPacket {
        RF69_Packet packet;
        int16_t rssi;
        unsigned long timestamp_ms;
    };

    struct RF69_OutboundPacket {
        uint8_t receiver_id;
        uint8_t command;
        char payload[RF69_MAX_PAYLOAD_LEN];
    };

    /**
     * @brief Construct a new Radio Comm object
     * @param node_id Unique identification address for this specific node
     * @param frequency Operating frequency in MHz (e.g., 434.0 or 915.0)
     * @param cs_pin SPI chip select / slave select pin
     * @param int_pin Hardware interrupt pin connected to DIO0
     * @param rst_pin Hardware reset pin connected to radio reset
     */
    RadioComm(uint8_t node_id, float frequency, uint8_t cs_pin, uint8_t int_pin, uint8_t rst_pin);

    /**
     * @brief Boots hardware interfaces and assigns default properties
     * @param sync_words Pointer to a 2-byte network identifier array
     * @param encryption_key Pointer to a 16-character AES-128 secret key string (null for unencrypted)
     * @return true if initialization passes, false if hardware missing/unresponsive
     */
    bool begin(const uint8_t* sync_words = nullptr, const char* encryption_key = nullptr);

    /**
     * @brief Hands an outbound payload directly to hardware SPI registers non-blockingly
     * @return true if the transmission started successfully, false if driver is busy or blocked
     */
    bool send(uint8_t receiver_id, uint8_t command, const void* data, uint8_t data_len);

    /**
     * @brief Essential execution tick. Evaluates background TX flags and drains hardware 
     * FIFOs into the software ring buffer. Run continuously in the main loop.
     */
    void update();

    /**
     * @brief Checks if there are any unread packets waiting in the software buffer
     */
    bool available() const;

    /**
     * @brief Returns the total count of unread packets currently residing in the buffer
     */
    size_t queued_count() const;

    /**
     * @brief Pops the oldest unread packet out of the ring buffer ring array
     * @param out Reference to a storage container where the packet data will copy to
     * @return true if a packet was successfully retrieved, false if buffer is empty
     */
    bool receive(RF69_ReceivedPacket& out);

    /**
     * @brief Toggles local hardware status messaging via the Serial interface
     */
    void enable_debug(bool enable);

    /**
     * @brief Returns the absolute RSSI value calculated from the last received packet
     */
    int16_t get_last_rssi() const;

private:
    // Core hardware instance controller
    RH_RF69 _radio;

    // Physical configurations
    uint8_t _node_id;
    float _frequency;
    uint8_t _cs_pin;
    uint8_t _int_pin;
    uint8_t _rst_pin;

    // State Tracking Flags
    bool _tx_in_progress;
    bool _debug_enabled = false;
    int16_t _last_rssi;

    // Software Circular Ring Buffer Mechanics
    RF69_ReceivedPacket _rx_queue[RF69_RX_QUEUE_SIZE];
    volatile size_t _rx_head;
    volatile size_t _rx_tail;
    volatile size_t _rx_count;

    RF69_OutboundPacket _tx_queue[RF69_TX_QUEUE_SIZE];
    volatile size_t _tx_head;
    volatile size_t _tx_tail;
    volatile size_t _tx_count;

    /**
     * @brief Internal helper to push verified payloads cleanly into storage arrays
     */
    void push_received(const RF69_Packet& packet, int16_t rssi);
    bool transmit_next_packet();
};

#endif // RADIO_HPP