#ifndef TripleHWPwm_h
#define TripleHWPwm_h

#include <Arduino.h>

class TripleHardwarePWM {
public:
    TripleHardwarePWM();
    void begin(uint32_t frequency);
    void setDutyCycle1(float percent);
    void setDutyCycle2(float percent);
    void setDutyCycle3(float percent);
    void end();

private:
    void configureTCC1(uint32_t frequency);
    void configureTC3(uint32_t frequency);
    void configureTCC2(uint32_t frequency);
    
    bool _initialized = false;
};

#endif