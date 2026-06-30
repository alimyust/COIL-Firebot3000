#ifndef RFCOMM_HPP
#define RFCOMM_HPP

#include <Arduino.h>
#include <RH_RF69.h>
#include <SPI.h>

// Packet structure with addressing
#pragma pack(push, 1)
struct RF69_Packet {
    uint8_t sender_id;
    uint8_t receiver_id;
    uint8_t command;
    char payload[50];
};
#pragma pack(pop)

enum class RadioEventType : uint8_t {
    None = 0,
    PacketReceived = 1,
    TelemetryTick = 2
};

struct RadioEvent {
    RadioEventType type;
    RF69_Packet packet;
};

class RF69_Comm {
public:
    RF69_Comm(uint8_t node_id, float frequency, uint8_t cs_pin = 8, 
              uint8_t int_pin = 3, uint8_t rst_pin = 4);
    
    bool begin(const uint8_t* sync_words = nullptr, 
               const char* encryption_key = nullptr);
    
    bool send(uint8_t receiver_id, uint8_t command, const char* message);
    void update();
    void set_receive_handler(void (*handler)(RF69_Packet &packet));
    
    void enable_debug(bool enable);
    int16_t get_last_rssi();

private:
    RH_RF69 _radio;
    uint8_t _node_id;
    float _frequency;
    uint8_t _rst_pin;
    void (*_receive_handler)(RF69_Packet &packet);
    bool _debug_enabled;
    int16_t _last_rssi;
};

class EventRadioComm {
public:
    EventRadioComm(uint8_t node_id, float frequency, uint8_t cs_pin = 8,
                   uint8_t int_pin = 3, uint8_t rst_pin = 4);

    bool begin(const uint8_t* sync_words = nullptr,
               const char* encryption_key = nullptr);

    bool send(uint8_t receiver_id, uint8_t command, const char* message);
    void update();
    bool hasPendingEvent() const;
    bool pollEvent(RadioEvent &out_event);
    void queueTelemetryTick();
    void injectPacket(const RF69_Packet &packet);
    void setTestMode(bool enable);

    void enable_debug(bool enable);
    int16_t get_last_rssi();

private:
    static EventRadioComm* s_instance;
    static void receiveCallback(RF69_Packet &packet);

    void enqueueEvent(RadioEventType type, const RF69_Packet &packet);
    void handlePacket(const RF69_Packet &packet);

    RH_RF69 _radio;
    uint8_t _node_id;
    float _frequency;
    uint8_t _rst_pin;
    bool _debug_enabled;
    bool _test_mode;
    int16_t _last_rssi;

    static constexpr uint8_t EVENT_QUEUE_SIZE = 8;
    RadioEvent _event_queue[EVENT_QUEUE_SIZE];
    uint8_t _queue_head;
    uint8_t _queue_tail;
    uint8_t _queue_count;
};

#endif