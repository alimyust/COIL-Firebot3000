#ifndef MICROPHONE_HPP
#define MICROPHONE_HPP

#include <Adafruit_ZeroDMA.h>

#define ADC_PIN A1
#define SAMPLE_BLOCK_LENGTH 64

class Microphone {
    public:
        Microphone();
        void begin();
        bool isBufferReady();
        void readActiveBuffer(int16_t* output_buffer);
        void adc_init();
        void dma_init();

    private:
        Adafruit_ZeroDMA ADC_DMA;
        DmacDescriptor *dmac_descriptor_1;
        DmacDescriptor *dmac_descriptor_2;
        
        static uint16_t adc_buffer[SAMPLE_BLOCK_LENGTH * 2];
        static bool filling_first_half;
        static volatile uint16_t *active_adc_buffer;
        static bool adc_buffer_filled;

        static void ADCsync() {
           while (ADC->STATUS.bit.SYNCBUSY == 1);
        }
           
        static void dma_callback(Adafruit_ZeroDMA *dma);
};

#endif // MICROPHONE_HPP