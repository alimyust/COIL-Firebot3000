#pragma once

#include <Arduino.h>

// Uses 2 TCC timers...
// Steer and throttle on TCC2 (D11, D13)
// Turret Uses TCC0 (D5, D6, D12)

class PentaHardwarePWM {
public:
    PentaHardwarePWM(uint8_t motorPin1 = 13,
                     uint8_t motorPin2 = 11,
                     uint8_t servoPin1 = 5,
                     uint8_t servoPin2 = 6,
                     uint8_t servoPin3 = 12);

    void begin(uint32_t motorFrequency,
               uint32_t servoFrequency);

    void setDuty13(float percent); // pin 13
    void setDuty11(float percent);    // pin 11

    void setDuty12(float percent); // pin 12
    void setDuty6(float percent); // pin 6
    void setDuty5(float percent); // pin 5

private:

    uint8_t _motorPin1;
    uint8_t _motorPin2;
    uint8_t _servoPin1;
    uint8_t _servoPin2;
    uint8_t _servoPin3;

    Tcc* _motorTimer;
    Tcc* _servoTimer;

    void configureTimer(Tcc* timer,
                        uint8_t gclk_id,
                        uint32_t frequency);

    void setDutyCycle(Tcc* timer,
                      uint8_t cc_channel,
                      float percent);
};