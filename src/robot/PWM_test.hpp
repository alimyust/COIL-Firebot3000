#ifndef PWM_TEST_HPP
#define PWM_TEST_HPP

#include <Arduino.h>

class QuadHardwarePWM {
public:
    QuadHardwarePWM(uint8_t pin1 = 5, uint8_t pin2 = 6, uint8_t pin3 = 9, uint8_t pin4 = 10);

    void begin(uint32_t frequency = 60);
    void setDutyCycle(uint8_t index, uint8_t percent);
    void setDutyCycle1(uint8_t percent);
    void setDutyCycle2(uint8_t percent);
    void setDutyCycle3(uint8_t percent);
    void setDutyCycle4(uint8_t percent);

private:
    struct PWMChannel {
        uint8_t pin;
        Tcc* timer;
        uint8_t cc_channel;
    };

    PWMChannel _channels[4];
    bool _configuredTCC0;
    bool _configuredTCC1;
    uint32_t _frequency;

    void configureTimer(Tcc* timer, uint8_t gclk_id);
    void setupChannel(uint8_t index, uint8_t pin);
    void setDutyCycleOnChannel(const PWMChannel& channel, uint8_t percent);
    void attachPinToTimer(uint8_t pin, Tcc* timer, uint8_t gclk_id, uint8_t cc_channel, uint8_t pmuxFunction);
};

#endif
