#pragma once

#include <Arduino.h>

class QuadHardwarePWM {
public:
    QuadHardwarePWM(uint8_t motorPin1,
                    uint8_t motorPin2,
                    uint8_t servoPin1,
                    uint8_t servoPin2);

    void begin(uint32_t motorFrequency,
               uint32_t servoFrequency);

    void setDutyCycle1(float percent);
    void setDutyCycle2(float percent);
    void setDutyCycle3(float percent);
    void setDutyCycle4(float percent);

private:

    uint8_t _motorPin1;
    uint8_t _motorPin2;
    uint8_t _servoPin1;
    uint8_t _servoPin2;

    Tcc* _motorTimer;
    Tcc* _servoTimer;

    void configureTimer(Tcc* timer,
                        uint8_t gclk_id,
                        uint32_t frequency);

    void setDutyCycle(Tcc* timer,
                      uint8_t cc_channel,
                      float percent);
};