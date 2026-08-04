#pragma once

#include <Arduino.h>

#define SPEAKER_BUFFER_SIZE 512

class Speaker {
public:
    Speaker();
    void begin();
    bool queueAudio(int16_t pcm_sample);
    
    // Internal ISR handler called by TC3_Handler
    void isr_playNextSample();

    // Static instance pointer for global ISR access
    static Speaker* instance;

private:
    uint8_t _dac_pin;
    
    // Ring buffer components
    volatile int16_t _buffer[SPEAKER_BUFFER_SIZE];
    volatile uint16_t _head;
    volatile uint16_t _tail;
    volatile uint16_t _count;
    volatile int16_t _last_sample;
    volatile bool _has_last_sample;

    void beginTimer();
};