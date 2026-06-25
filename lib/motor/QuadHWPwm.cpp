#include "QuadHWPwm.hpp"

QuadHardwarePWM::QuadHardwarePWM(uint8_t motorPin1,
                                 uint8_t motorPin2,
                                 uint8_t servoPin1,
                                 uint8_t servoPin2)
    :
    _motorPin1(motorPin1),
    _motorPin2(motorPin2),
    _servoPin1(servoPin1),
    _servoPin2(servoPin2),
    _motorTimer(nullptr),
    _servoTimer(nullptr)
{
}

void QuadHardwarePWM::begin(uint32_t motorFrequency,
                            uint32_t servoFrequency)
{
    //
    // -----------------------
    // Motors (TCC2)
    // -----------------------
    //

    PM->APBCMASK.reg |= PM_APBCMASK_TCC2;

    _motorTimer = TCC2;

    configureTimer(_motorTimer,
                   TCC2_GCLK_ID,
                   motorFrequency);

    //
    // D11 (PA16)
    // TCC2 WO0
    //

    PORT->Group[PORTA].PINCFG[16].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[8].bit.PMUXE =
        PORT_PMUX_PMUXE_E_Val;

    //
    // D13 (PA17)
    // TCC2 WO1
    //

    PORT->Group[PORTA].PINCFG[17].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[8].bit.PMUXO =
        PORT_PMUX_PMUXO_E_Val;

    //
    // -----------------------
    // Servos (TCC0)
    // -----------------------
    //

    PM->APBCMASK.reg |= PM_APBCMASK_TCC0;

    _servoTimer = TCC0;

    configureTimer(_servoTimer,
                   TCC0_GCLK_ID,
                   servoFrequency);

    //
    // D6 (PA20)
    // TCC0 WO6
    //

    PORT->Group[PORTA].PINCFG[20].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[10].bit.PMUXE =
        PORT_PMUX_PMUXE_F_Val;

    //
    // D5 (PA15)
    // TCC0 WO5
    //

    PORT->Group[PORTA].PINCFG[15].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[7].bit.PMUXO =
        PORT_PMUX_PMUXO_F_Val;
}

void QuadHardwarePWM::configureTimer(Tcc* timer,
                                     uint8_t gclk_id,
                                     uint32_t frequency)
{
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN_GCLK0 |
        GCLK_CLKCTRL_ID(gclk_id);

    while (GCLK->STATUS.bit.SYNCBUSY);

    timer->CTRLA.bit.ENABLE = 0;
    while (timer->SYNCBUSY.bit.ENABLE);

    timer->CTRLA.bit.SWRST = 1;
    while (timer->SYNCBUSY.bit.SWRST);

    timer->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
    while (timer->SYNCBUSY.bit.WAVE);

    constexpr uint32_t prescaler = 1024;

    timer->CTRLA.bit.PRESCALER = TCC_CTRLA_PRESCALER_DIV1024_Val;

    timer->PER.reg =
        (48000000UL /
        (prescaler * frequency)) - 1;

    while (timer->SYNCBUSY.bit.PER);

    timer->CTRLA.bit.ENABLE = 1;
    while (timer->SYNCBUSY.bit.ENABLE);
}

void QuadHardwarePWM::setDutyCycle(Tcc* timer,
                                   uint8_t cc_channel,
                                   float percent)
{
    if (!timer)
        return;

    percent = constrain(percent, 0.0f, 100.0f);

    timer->CC[cc_channel].reg =
        (timer->PER.reg * percent) / 100.0f;

    while (timer->SYNCBUSY.reg &
          (1 << (TCC_SYNCBUSY_CC0_Pos + cc_channel)));
}

void QuadHardwarePWM::setDutyCycle1(float percent)
{
    setDutyCycle(_motorTimer, 0, percent);
}

void QuadHardwarePWM::setDutyCycle2(float percent)
{
    setDutyCycle(_motorTimer, 1, percent);
}

void QuadHardwarePWM::setDutyCycle3(float percent)
{
    setDutyCycle(_servoTimer, 2, percent);
}

void QuadHardwarePWM::setDutyCycle4(float percent)
{
    setDutyCycle(_servoTimer, 1, percent);
}