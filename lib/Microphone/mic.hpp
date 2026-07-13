
#include <Adafruit_ZeroDMA.h>
 // ^ Code adjusted from ZeroDMA library examples
#define ADC_PIN A0 
#define SAMPLE_BLOCK_LENGTH 256

class Microphone {
    public:
        Microphone() {
            filling_first_half = true;
            active_adc_buffer = nullptr;
            adc_buffer_filled = false;
        }
        void begin(){
            adc_init();
            dma_init();
            ADC_DMA.startJob();
        }
        
        void update() {
            if(!adc_buffer_filled){
                return;
            }
            adc_buffer_filled = false;
            // memcpy(adc_sample_block, (const void*)active_adc_buffer, SAMPLE_BLOCK_LENGTH*sizeof(uint16_t));
            for(int i = 0; i < SAMPLE_BLOCK_LENGTH; i++) {
                // Convert the unsigned 0-4095 value to a signed value centered around 0
                int16_t signed_sample = (int16_t)active_adc_buffer[i] - 2048;
                Serial.println(signed_sample);
            }
        }
        void adc_init() {
            analogRead(ADC_PIN);
            ADC->CTRLA.bit.ENABLE = 0;
            ADCsync();
            ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2;
            ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1;
            ADCsync();
            ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[ADC_PIN].ulADCChannelNumber;
            ADCsync();
            ADC->AVGCTRL.reg = 0;
            ADC->SAMPCTRL.reg = 2;
            ADCsync();
            ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_FREERUN | ADC_CTRLB_RESSEL_12BIT;
            ADCsync();
            ADC->CTRLA.bit.ENABLE = 1;
            ADCsync();
        }

        void dma_init(){
            ADC_DMA.allocate();
            ADC_DMA.setTrigger(ADC_DMAC_ID_RESRDY);
            ADC_DMA.setAction(DMA_TRIGGER_ACTON_BEAT);
            dmac_descriptor_1 = ADC_DMA.addDescriptor(
                    (void *)(&ADC->RESULT.reg),
                    adc_buffer,
                    SAMPLE_BLOCK_LENGTH,
                    DMA_BEAT_SIZE_HWORD,
                    false,
                    true);
            dmac_descriptor_1->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;

            dmac_descriptor_2 = ADC_DMA.addDescriptor(
                    (void *)(&ADC->RESULT.reg),
                    adc_buffer + SAMPLE_BLOCK_LENGTH,
                    SAMPLE_BLOCK_LENGTH,
                    DMA_BEAT_SIZE_HWORD,
                    false,
                    true);
            dmac_descriptor_2->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;

            ADC_DMA.loop(true);
            ADC_DMA.setCallback(dma_callback);
        }
        //void (*callback)(Adafruit_ZeroDMA *) = (void (*)(Adafruit_ZeroDMA *))__null, dma_callback_type type = DMA_CALLBACK_TRANSFER_DONE
        
        uint16_t peak_to_peak(uint16_t *data, int data_length){
            int signalMax = 0;
            int signalMin = 4096;  // max value for 12 bit adc
            
            for(int i=0; i<data_length; i++){
                if ( data[i] > signalMax ) {
                signalMax = data[i];
                }
                if (data[i] < signalMin ){
                signalMin = data[i];
                }
            }
            return signalMax - signalMin;
            }

        private:

        Adafruit_ZeroDMA ADC_DMA;
        DmacDescriptor *dmac_descriptor_1;
        DmacDescriptor *dmac_descriptor_2;
        
        static uint16_t adc_buffer[SAMPLE_BLOCK_LENGTH * 2];
        static uint16_t adc_sample_block[SAMPLE_BLOCK_LENGTH];
        static bool filling_first_half;
        static volatile uint16_t *active_adc_buffer;
        static bool adc_buffer_filled;


        static void ADCsync() {
           while (ADC->STATUS.bit.SYNCBUSY == 1);
        }
           
        static void dma_callback(Adafruit_ZeroDMA *dma) {
            (void) dma;
            if (filling_first_half) {
                // FIXED: First half just finished filling. Safe to read first half!
                active_adc_buffer = &adc_buffer[0];
                filling_first_half = false;
            } else {
                // FIXED: Second half just finished filling. Safe to read second half!
                active_adc_buffer = &adc_buffer[SAMPLE_BLOCK_LENGTH];
                filling_first_half = true;
            }
            adc_buffer_filled = true;
        }

};  


uint16_t Microphone::adc_buffer[SAMPLE_BLOCK_LENGTH * 2];
uint16_t Microphone::adc_sample_block[SAMPLE_BLOCK_LENGTH];
bool Microphone::filling_first_half = true;
volatile uint16_t* Microphone::active_adc_buffer = nullptr;
bool Microphone::adc_buffer_filled = false;