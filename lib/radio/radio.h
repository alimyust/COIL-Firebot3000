

#include <Arduino.h>

#define QUEUE_SIZE 4 // Multi-level buffer structure replacing single double-buffering 

struct RF69_Packet {
    uint8_t sender_id;
    uint8_t receiver_id;
    uint8_t command;
    char payload[61]; // Adjust size to align with your payload specification
};

class RF69_Comm {
public:
    // ... your existing constructor and methods ...
    void processEvents(); // Call this instead of update()

    static void onRadioInterrupt();

private:
    // Thread-safe ring buffer queue allocations
    static volatile bool s_packet_ready;
    static volatile bool s_queue_overflow;
    static volatile RF69_Packet s_packet_queue[QUEUE_SIZE];
    static volatile int16_t s_rssi_queue[QUEUE_SIZE];
    static volatile uint8_t s_queue_head;
    static volatile uint8_t s_queue_tail;
    
    // ... your existing private variables ...
};