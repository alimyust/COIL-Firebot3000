

// #include <Adafruit_ZeroDMA.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>
// #include <ProtocolCommands.hpp>

// class DisplayOLED {

//     public:
//         DisplayOLED(bool _debug_enabled){}

//         void init_display(){
//             display.begin(0x3C, true); // Address 0x3C default
//             if (_debug_enabled){
//                 Serial.println("Oled Init Success");
//             }
//         }

//         void display_HB(const ProtocolCommands::HeartbeatPayload& payload){
//             uint32_t data = payload.timestamp;
//             if (_debug_enabled){
//                 Serial.print("Printing to OLED:");
//                 Serial.println(data);
//             }
//             display.print("HB: ");
//             display.println(data);
//             // ^ using display print to fill oled buffer, without sending anything
//         }

//         void display_Sensor(const ProtocolCommands::SensorPayload& payload){
            

//         }

//         void send_Display(){
//             // setting the DMAC to feed the display buffer into the sercom port
//             uint8_t* disp_buffer = display.getBuffer();
            
//             uint8_t* page_start_pointer = disp_buffer + (current_page * 128);
            
//             Wire.beginTransmission(SERCOM3->I2CM.DATA.reg);

//             // 3. Configure the DMA Descriptor
//             // We hand the DMA controller our offset pointer directly.
//             DmacDescriptor* desc = I2C_DMA.addDescriptor(
//                 (void*)page_start_pointer,        // Source: The exact memory address where this page begins
//                 (void*)&SERCOM3->I2CM.DATA.reg,   // Destination: The SERCOM hardware mailbox
//                 128,                              // Count: Send exactly 128 bytes
//                 DMA_BEAT_SIZE_BYTE,               // Move exactly 1 byte at a time
//                 true,                             // Increment source: Walk forward through the array
//                 false                             // Do NOT increment destination: Keep shoving into the same mailbox
//             );
            
//         }

//         void dma_callback(){

//         }
         
//     private:
        
//         bool _debug_enabled;
//         Adafruit_ZeroDMA I2C_DMA;
//         uint8_t current_page; // e[0,8\7] for each slice of the oled buffer
        
//         Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// };


#ifndef DISPLAY_DMA_HPP
#define DISPLAY_DMA_HPP

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_ZeroDMA.h>

class DisplayOLED {
public:
    DisplayOLED(bool debug_enabled = false);

    // Initializes the display and allocates the DMA hardware
    void begin();

    // Replaces display.display(). Call this to trigger a background frame push.
    void pushFrame();

    // Call this continuously in your EventScheduler or main loop()
    void update();

    // Publicly accessible Adafruit display object for drawing (print, drawPixel, etc.)
    Adafruit_SH1107 display;

    // DMA and Hardware control
    Adafruit_ZeroDMA I2C_DMA;
    DmacDescriptor* _descriptor;

private:

    static DisplayOLED* _instance; // Required for static ISR routing

        // State Machine Tracking
    volatile bool _is_pushing_frame;
    volatile bool _waiting_for_dma;
    volatile uint8_t _current_page;
    bool _debug_enabled;


    // The static callback caught by the ZeroDMA library
    static void dma_isr_callback(Adafruit_ZeroDMA *dma);
};

#endif // DISPLAY_DMA_HPP