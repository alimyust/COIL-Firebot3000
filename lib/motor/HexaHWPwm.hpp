#pragma once

#include <Arduino.h>

// Uses 3 TCC timers...
// Steer and throttle on TCC2 (D11, D13)
// Turret Uses TCC0 (D5, D6, D12)
// Extra Aux uses TCC1 (D9)

class HexaHardwarePWM {
public:
    HexaHardwarePWM(uint8_t motorPin1 = 13,
                    uint8_t motorPin2 = 11,
                    uint8_t servoPin1 = 5,
                    uint8_t servoPin2 = 6,
                    uint8_t servoPin3 = 12,
                    uint8_t auxPin1 = 9);

    void begin(uint32_t motorFrequency,
               uint32_t servoFrequency,
               uint32_t auxFrequency);

    void setDutyThrottle(float percent); //pin 13
    void setDutySteer(float percent);  //pin 11

    void setDutyPinTurretZ(float percent); //pin 12
    void setDutyPinTurretX(float percent); //pin 6
    void setDutyPinTurretY(float percent); // pin 5
    void setDutyPinAux(float percent); //pin 9

private:

    uint8_t _motorPin1;
    uint8_t _motorPin2;
    uint8_t _servoPin1;
    uint8_t _servoPin2;
    uint8_t _servoPin3;
    // uint8_t _servoPin4;
    uint8_t _auxPin1;

    Tcc* _motorTimer;
    Tcc* _servoTimer;
    Tcc* _auxTimer;

    void configureTimer(Tcc* timer,
                        uint8_t gclk_id,
                        uint32_t frequency);

    void setDutyCycle(Tcc* timer,
                      uint8_t cc_channel,
                      float percent);
};
