
#include "CommandHandler.hpp"
#include "RFComm.hpp"
// 
// A class to handle all tx/rx radio comms, separating robot
// operation logic from communication layer.
// 

/*
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

*/

class ProtocolLayer {
public:
    ProtocolLayer(RF69_Comm &comm);
    void begin();
    void handle_incoming();