#ifndef RFCOMM_HPP
#define RFCOMM_HPP

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

class RF69_Comm {
public:
    RF69_Comm(uint8_t node_id, float frequency, uint8_t cs_pin = 8, 
              uint8_t int_pin = 3, uint8_t rst_pin = 4);
    
    bool begin(const uint8_t* sync_words = nullptr, 
               const char* encryption_key = nullptr);
    
    bool send(uint8_t receiver_id, uint8_t command, const char* message, uint8_t len =0);
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

#endif