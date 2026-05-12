#include "DualHWPwm.hpp"
#include <Arduino.h>

DualHardwarePWM::DualHardwarePWM(uint8_t pin1, uint8_t pin2) 
  : _pin1(pin1), _pin2(pin2), _timer1(nullptr), _timer2(nullptr) {}

void DualHardwarePWM::begin(uint32_t frequency) {
    _frequency = frequency;

    // Configure Pin 9 (PA07, TCC1/WO[1])
    if (_pin1 == 9) {
        PM->APBCMASK.reg |= PM_APBCMASK_TCC1;  // Enable TCC1 clock
        _timer1 = TCC1;
        PORT->Group[PORTA].PINCFG[7].bit.PMUXEN = 1;
        PORT->Group[PORTA].PMUX[3].bit.PMUXO = PORT_PMUX_PMUXO_E_Val;  // Function E
        configureTimer(_timer1, TCC1_GCLK_ID);
    }

    // Configure Pin 5 (PA15, TCC0/WO[1])
    if (_pin2 == 5) {
        PM->APBCMASK.reg |= PM_APBCMASK_TCC0;  // Enable TCC0 clock
        _timer2 = TCC0;
        PORT->Group[PORTA].PINCFG[15].bit.PMUXEN = 1;
        PORT->Group[PORTA].PMUX[7].bit.PMUXO = PORT_PMUX_PMUXO_F_Val;  // Function F (TCC0/WO[5])
        configureTimer(_timer2, TCC0_GCLK_ID);
    }
}

void DualHardwarePWM::configureTimer(Tcc* timer, uint8_t gclk_id) {
    // Enable clock
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID(gclk_id);
    while (GCLK->STATUS.bit.SYNCBUSY);

    // Reset timer
    timer->CTRLA.bit.ENABLE = 0;
    while (timer->SYNCBUSY.bit.ENABLE);
    timer->CTRLA.bit.SWRST = 1;
    while (timer->SYNCBUSY.bit.SWRST);

    // Waveform setup
    timer->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
    while (timer->SYNCBUSY.bit.WAVE);

    // Set frequency
    const uint32_t prescaler = 1024;
    timer->CTRLA.bit.PRESCALER = 0x07;  // Prescaler 1024
    timer->PER.reg = (48000000UL / (prescaler * _frequency)) - 1;
    while (timer->SYNCBUSY.bit.PER);

    // Enable timer
    timer->CTRLA.bit.ENABLE = 1;
    while (timer->SYNCBUSY.bit.ENABLE);
}

void DualHardwarePWM::setDutyCycle(Tcc* timer, uint8_t cc_channel, uint8_t percent) {
    if (!timer) return;
    
    const uint32_t per = timer->PER.reg;
    timer->CC[cc_channel].reg = (per * constrain(percent, 0, 100)) / 100;
    
    // Wait for CC channel sync
    switch (cc_channel) {
        case 0: while (timer->SYNCBUSY.bit.CC0); break;
        case 1: while (timer->SYNCBUSY.bit.CC1); break;
    }
}

void DualHardwarePWM::setDutyCycle1(uint8_t percent) {
    if (_timer1) setDutyCycle(_timer1, 1, percent);
}

void DualHardwarePWM::setDutyCycle2(uint8_t percent) {
    if (_timer2) setDutyCycle(_timer2, 1, percent);
}