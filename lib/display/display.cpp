#include "display.h"

// Initialize the static instance pointer
DisplayOLED* DisplayOLED::_instance = nullptr;

DisplayOLED::DisplayOLED(bool debug_enabled) 
    : display(64, 128, &Wire), 
      _debug_enabled(debug_enabled),
      _is_pushing_frame(false),
      _waiting_for_dma(false),
      _current_page(0) 
{
    _instance = this;
}

void DisplayOLED::begin() {
    // 1. Force fast I2C mode
    // Wire.setClock(400000); 

    // 2. Initialize the OLED normally (Let Adafruit handle the heavy startup config)
    display.begin(0x3C, true); 
    display.clearDisplay();
    display.display();
    display.setRotation(1);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(SH110X_WHITE);
    display.display();
    // 3. Allocate and Configure the DMA Controller for SERCOM3 [cite: 249]
    I2C_DMA.allocate();
    I2C_DMA.setTrigger(SERCOM3_DMAC_ID_TX); // Only push when mailbox is empty [cite: 254, 256]
    I2C_DMA.setAction(DMA_TRIGGER_ACTON_BEAT); // Push 1 byte at a time [cite: 190]

    // Create a dummy descriptor to be modified later. 
    // Destination is strictly the SERCOM3 Data Register [cite: 253]
    uint8_t dummy_src = 0;
    _descriptor = I2C_DMA.addDescriptor(
        (void*)&dummy_src,
        (void*)&SERCOM3->I2CM.DATA.reg, 
        128,                            // 128 bytes per page [cite: 231]
        DMA_BEAT_SIZE_BYTE,
        true,                           // Increment source pointer [cite: 280]
        false                           // Do not increment destination
    );

    I2C_DMA.setCallback(dma_isr_callback);

    if (_debug_enabled) Serial.println("Display DMA Initialized");
}

void DisplayOLED::pushFrame() {
    // If we are already pushing a frame in the background, ignore this call
    // Serial.println(_current_page);
    if (_is_pushing_frame) return;
    _current_page = 0;
    _is_pushing_frame = true; 
    _waiting_for_dma = false;
}
void DisplayOLED::update() {
    if (!_is_pushing_frame || _waiting_for_dma) return;

    // --- STAGE 0: Bus Closure Check ---
    if (_current_page > 0 && _current_page <= 16) {
        while (!SERCOM3->I2CM.INTFLAG.bit.MB); 
        SERCOM3->I2CM.CTRLB.bit.CMD = 0x3; // Send STOP condition
        while (SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
    }

    // --- STAGE 1: Check Termination (16 PAGES TOTAL) ---
    if (_current_page >= 16) {
        _is_pushing_frame = false;
        _current_page = 0;
        return;
    }

    // --- STAGE 2: Set Page Address via Wire ---
    Wire.beginTransmission(0x3C);
    Wire.write(0x00);                  // Command byte stream flag
    Wire.write(0xB0 + _current_page);  // Page 0 to 15 (0xB0 to 0xBF)
    Wire.write(0x00);                  // Low Column 0
    Wire.write(0x10);                  // High Column 0
    Wire.endTransmission(); 

    // --- STAGE 3: SERCOM Hijack ---
    SERCOM3->I2CM.ADDR.bit.ADDR = (0x3C << 1) | 0; 
    while (SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
    while (!SERCOM3->I2CM.INTFLAG.bit.MB); 

    SERCOM3->I2CM.DATA.reg = 0x40;     // Data Payload indicator
    while (!SERCOM3->I2CM.INTFLAG.bit.MB); 

    // --- STAGE 4: Pointer Arithmetic & DMA (64 BYTES PER PAGE) ---
    // Memory stride is 64 bytes per page slice
    uint8_t* slice_start = display.getBuffer() + (_current_page * 64);

    // Update descriptor for 64-byte burst
    I2C_DMA.changeDescriptor(_descriptor, (void*)slice_start, (void*)&SERCOM3->I2CM.DATA.reg, 64);
    
    _waiting_for_dma = true;
    _current_page++; 

    I2C_DMA.startJob();
}
void DisplayOLED::dma_isr_callback(Adafruit_ZeroDMA *dma) {
    if (_instance == nullptr) return;


    // 2. Advance the state machine for the main loop to catch 
    // _instance->_current_page++;
    _instance->_waiting_for_dma = false;
}