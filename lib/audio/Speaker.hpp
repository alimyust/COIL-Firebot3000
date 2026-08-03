#ifndef SPEAKER_HPP
#define SPEAKER_HPP

#include <Arduino.h>

#define SPEAKER_BUFFER_SIZE 512 // Big enough to hold a few 64-sample blocks

class Speaker {
public:
    Speaker();
    void begin();
    void beginTimer();
    // Pushes sample into the buffer (called by AudioHandler)
    bool queueAudio(int16_t pcm_sample);
    uint16_t getBufferCount() const { return _count; }
    // Pops sample from buffer to DAC (called by the Timer ISR)
    void isr_playNextSample();

    static Speaker* instance; // Singleton pointer for the C-style ISR

private:
    uint8_t _dac_pin = A0;
    
    volatile int16_t _buffer[SPEAKER_BUFFER_SIZE];
    volatile uint16_t _head;
    volatile uint16_t _tail;
    volatile uint16_t _count;
};

#endif // SPEAKER_HPP