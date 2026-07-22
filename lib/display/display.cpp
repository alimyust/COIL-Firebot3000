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
    // If no frame is actively being sent, or the DMA is currently busy, do nothing.
    // Serial.println(_waiting_for_dma);
    if (!_is_pushing_frame || _waiting_for_dma){
        return;}
    // If we finished all 8 pages[cite: 230], release the state machine
    if (_current_page >= 8) {
        _is_pushing_frame = false;
        _current_page = 0;
        return;
    }

    // --- STAGE 1: CPU Handshake (Takes ~100us) ---
    // Safely use standard Wire in the main loop to address the exact page
    Wire.beginTransmission(0x3C);
    Wire.write(0x00); // Command byte stream
    Wire.write(0xB0 + _current_page); // Send Page address
    Wire.write(0x10); // High column
    Wire.write(0x00); // Low column
    Wire.endTransmission(); 

    // --- STAGE 2: Manual SERCOM Hijack ---
    // Generate a START condition + Write bit (0) to claim the bus
    SERCOM3->I2CM.ADDR.bit.ADDR = (0x3C << 1) | 0; 
    while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
    while(!SERCOM3->I2CM.INTFLAG.bit.MB); // Wait for ACK
    
    // Send the Data Control Byte (0x40) indicating the following payload is pixel data
    SERCOM3->I2CM.DATA.reg = 0x40;
    while(!SERCOM3->I2CM.INTFLAG.bit.MB); // Wait for ACK

    // --- STAGE 3: Pointer Arithmetic & DMA Unleash ---
    // Calculate memory slice: Move base pointer forward by (page * 128) bytes [cite: 278, 281]
    uint8_t* slice_start = display.getBuffer() + (_current_page * 128);

    // Update the descriptor with the new memory address
    I2C_DMA.changeDescriptor(_descriptor, (void*)slice_start, (void*)&SERCOM3->I2CM.DATA.reg, 128);
    
    // Lock the state machine and kick off the hardware
    _waiting_for_dma = true;
    I2C_DMA.startJob();
    yield();

}

// ISR Callback: Triggers exactly when the 128th byte is physically pushed
void DisplayOLED::dma_isr_callback(Adafruit_ZeroDMA *dma) {
    if (_instance == nullptr) return;

    // 1. Manually force a hardware I2C STOP condition to release the bus 
    SERCOM3->I2CM.CTRLB.bit.CMD = 0x3; 
    while(SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);

    // 2. Advance the state machine for the main loop to catch 
    _instance->_current_page++;
    _instance->_waiting_for_dma = false;
}