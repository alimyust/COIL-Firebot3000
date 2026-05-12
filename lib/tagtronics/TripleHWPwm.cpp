#include "TripleHWPwm.hpp"
#include <Arduino.h>

TripleHardwarePWM::TripleHardwarePWM() {}

void TripleHardwarePWM::begin(uint32_t frequency) {
    if (_initialized) return;
    
    Serial.println("Enabling timer clocks...");
    PM->APBCMASK.reg |= PM_APBCMASK_TCC1 | PM_APBCMASK_TC3 | PM_APBCMASK_TCC2;
    Serial.println("Clocks enabled");
    
    Serial.println("Configuring pin multiplexing...");
    // Pin 9: PA07 - TCC1/WO[1] (Function E)
    PORT->Group[0].PINCFG[7].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[3].bit.PMUXO = 0x04;
    
    // Pin 10: PA18 - TC3/WO[0] (Function E)
    PORT->Group[0].PINCFG[18].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[9].bit.PMUXE = 0x04;
    
    // Pin 11: PA16 - TCC2/WO[0] (Function F)
    PORT->Group[0].PINCFG[16].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[8].bit.PMUXE = 0x05;
    Serial.println("Pin multiplexing configured");
    
    // Configure each timer with the specified frequency
    Serial.println("Configuring TCC1 for pin 9...");
    configureTCC1(frequency);
    Serial.println("TCC1 configured");
    
    Serial.println("Configuring TC3 for pin 10...");
    configureTC3(frequency);
    Serial.println("TC3 configured");
    
    Serial.println("Configuring TCC2 for pin 11...");
    configureTCC2(frequency);
    Serial.println("TCC2 configured");
    
    _initialized = true;
    Serial.println("PWM initialization complete");
}

void TripleHardwarePWM::configureTCC1(uint32_t frequency) {
    // Reset TCC1
    Serial.println("  Resetting TCC1...");
    TCC1->CTRLA.bit.SWRST = 1;
    while(TCC1->CTRLA.bit.SWRST) {
        Serial.println("    Waiting for reset...");
        delay(1);
    }
    Serial.println("  TCC1 reset complete");
    
    // Configure clock
    Serial.println("  Configuring GCLK for TCC1...");
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID(0x19);  // TCC0/1 ID
    while(GCLK->STATUS.bit.SYNCBUSY) {
        Serial.println("    Waiting for GCLK sync...");
        delay(1);
    }
    Serial.println("  GCLK configured");
    
    // Configure timer
    Serial.println("  Setting wave mode...");
    TCC1->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
    
    // Calculate period with minimum value check
    Serial.print("  Calculating period for frequency: ");
    Serial.println(frequency);
    uint32_t period = (48000000UL / (1024 * frequency)) - 1;
    if (period < 10) period = 10; // Prevent invalid values
    Serial.print("  Period value: ");
    Serial.println(period);
    
    Serial.println("  Setting period register...");
    TCC1->PER.reg = period;
    while(TCC1->SYNCBUSY.bit.PER) {
        Serial.println("    Waiting for PER sync...");
        delay(1);
    }
    Serial.println("  Period set");
    
    Serial.println("  Setting initial duty cycle to 0...");
    TCC1->CC[1].reg = 0;
    while(TCC1->SYNCBUSY.bit.CC1) {
        Serial.println("    Waiting for CC1 sync...");
        delay(1);
    }
    
    Serial.println("  Setting prescaler...");
    TCC1->CTRLA.bit.PRESCALER = TCC_CTRLA_PRESCALER_DIV1024;
    
    Serial.println("  Enabling TCC1...");
    TCC1->CTRLA.bit.ENABLE = 1;
    while(TCC1->SYNCBUSY.bit.ENABLE) {
        Serial.println("    Waiting for enable sync...");
        delay(1);
    }
    Serial.println("  TCC1 enabled");
}

void TripleHardwarePWM::configureTC3(uint32_t frequency) {
    // Reset TC3
    Serial.println("  Resetting TC3...");
    TC3->COUNT16.CTRLA.bit.SWRST = 1;
    while(TC3->COUNT16.CTRLA.bit.SWRST) {
        Serial.println("    Waiting for reset...");
        delay(1);
    }
    Serial.println("  TC3 reset complete");
    
    // Configure clock
    Serial.println("  Configuring GCLK for TC3...");
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID(0x1B);  // TC3 ID
    while(GCLK->STATUS.bit.SYNCBUSY) {
        Serial.println("    Waiting for GCLK sync...");
        delay(1);
    }
    Serial.println("  GCLK configured");
    
    // Configure timer
    Serial.println("  Setting mode and wave generation...");
    TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 |
                             TC_CTRLA_WAVEGEN_MPWM |
                             TC_CTRLA_PRESCALER_DIV1024;
    
    // Calculate period
    Serial.print("  Calculating period for frequency: ");
    Serial.println(frequency);
    uint16_t period = (48000000UL / (1024 * frequency)) - 1;
    Serial.print("  Period value: ");
    Serial.println(period);
    
    Serial.println("  Setting period (CC0)...");
    TC3->COUNT16.CC[0].reg = period;     // PERIOD in CC0
    
    Serial.println("  Setting initial duty cycle to 0 (CC1)...");
    TC3->COUNT16.CC[1].reg = 0;          // DUTY in CC1
    
    Serial.println("  Enabling TC3...");
    TC3->COUNT16.CTRLA.bit.ENABLE = 1;
    Serial.println("  TC3 enabled");
}

void TripleHardwarePWM::configureTCC2(uint32_t frequency) {
    // Reset TCC2
    Serial.println("  Resetting TCC2...");
    TCC2->CTRLA.bit.SWRST = 1;
    while(TCC2->CTRLA.bit.SWRST) {
        Serial.println("    Waiting for reset...");
        delay(1);
    }
    Serial.println("  TCC2 reset complete");
    
    // Configure clock
    Serial.println("  Configuring GCLK for TCC2...");
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID(0x1A);  // TCC2 ID
    while(GCLK->STATUS.bit.SYNCBUSY) {
        Serial.println("    Waiting for GCLK sync...");
        delay(1);
    }
    Serial.println("  GCLK configured");
    
    // Configure timer
    Serial.println("  Setting wave mode...");
    TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
    
    // Calculate period with minimum value check
    Serial.print("  Calculating period for frequency: ");
    Serial.println(frequency);
    uint32_t period = (48000000UL / (1024 * frequency)) - 1;
    if (period < 10) period = 10; // Prevent invalid values
    Serial.print("  Period value: ");
    Serial.println(period);
    
    Serial.println("  Setting period register...");
    TCC2->PER.reg = period;
    while(TCC2->SYNCBUSY.bit.PER) {
        Serial.println("    Waiting for PER sync...");
        delay(1);
    }
    Serial.println("  Period set");
    
    Serial.println("  Setting initial duty cycle to 0...");
    TCC2->CC[0].reg = 0;
    while(TCC2->SYNCBUSY.bit.CC0) {
        Serial.println("    Waiting for CC0 sync...");
        delay(1);
    }
    
    Serial.println("  Setting prescaler...");
    TCC2->CTRLA.bit.PRESCALER = TCC_CTRLA_PRESCALER_DIV1024;
    
    Serial.println("  Enabling TCC2...");
    TCC2->CTRLA.bit.ENABLE = 1;
    while(TCC2->SYNCBUSY.bit.ENABLE) {
        Serial.println("    Waiting for enable sync...");
        delay(1);
    }
    Serial.println("  TCC2 enabled");
}

// Duty cycle setters
void TripleHardwarePWM::setDutyCycle1(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    uint32_t per = TCC1->PER.reg;
    TCC1->CC[1].reg = (per * percent) / 100;
    while(TCC1->SYNCBUSY.bit.CC1); // Wait for sync
}

void TripleHardwarePWM::setDutyCycle2(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    uint16_t period = TC3->COUNT16.CC[0].reg;
    TC3->COUNT16.CC[1].reg = period - ((period * percent) / 100);
}

void TripleHardwarePWM::setDutyCycle3(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    uint32_t per = TCC2->PER.reg;
    TCC2->CC[0].reg = (per * percent) / 100;
    while(TCC2->SYNCBUSY.bit.CC0); // Wait for sync
}

void TripleHardwarePWM::end() {
    // Disable all timers
    TCC1->CTRLA.bit.ENABLE = 0;
    TC3->COUNT16.CTRLA.bit.ENABLE = 0;
    TCC2->CTRLA.bit.ENABLE = 0;
    
    // Reset pins to default
    pinMode(9, INPUT);
    pinMode(10, INPUT);
    pinMode(11, INPUT);
    
    _initialized = false;
}