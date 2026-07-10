#include "HexaHWPwm.hpp"

HexaHardwarePWM::HexaHardwarePWM(uint8_t motorPin1,
                                 uint8_t motorPin2,
                                 uint8_t servoPin1,
                                 uint8_t servoPin2,
                                 uint8_t servoPin3,
                                 uint8_t auxPin1)
    :
    _motorPin1(motorPin1),
    _motorPin2(motorPin2),
    _servoPin1(servoPin1),
    _servoPin2(servoPin2),
    _servoPin3(servoPin3),
    _auxPin1(auxPin1),
    _motorTimer(nullptr),
    _servoTimer(nullptr),
    _auxTimer(nullptr)
{
}

void HexaHardwarePWM::begin(uint32_t motorFrequency,
                            uint32_t servoFrequency,
                            uint32_t auxFrequency)
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
    // D13 (PA17)
    // TCC2 WO1
    //

    PORT->Group[PORTA].PINCFG[17].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[8].bit.PMUXO =
        PORT_PMUX_PMUXO_E_Val;

    //
    // D11 (PA16)
    // TCC2 WO0
    //

    PORT->Group[PORTA].PINCFG[16].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[8].bit.PMUXE =
        PORT_PMUX_PMUXE_E_Val;

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
    // D5 (PA15)
    // TCC0 WO5
    //

    PORT->Group[PORTA].PINCFG[15].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[7].bit.PMUXO =
        PORT_PMUX_PMUXO_F_Val;

    //
    // D6 (PA20)
    // TCC0 WO6
    //

    PORT->Group[PORTA].PINCFG[20].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[10].bit.PMUXE =
        PORT_PMUX_PMUXE_F_Val;

    //
    // D12 (PA19)
    // TCC0 WO3
    //

    PORT->Group[PORTA].PINCFG[19].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[9].bit.PMUXO =
        PORT_PMUX_PMUXO_F_Val;

    //
    // -----------------------
    // Aux (TCC1)
    // -----------------------
    //

    PM->APBCMASK.reg |= PM_APBCMASK_TCC1;

    _auxTimer = TCC1;

    configureTimer(_auxTimer,
                   TCC1_GCLK_ID,
                   auxFrequency);

    //
    // D9 (PA7)
    // TCC1 WO1
    //

    PORT->Group[PORTA].PINCFG[7].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[3].bit.PMUXO =
        PORT_PMUX_PMUXO_E_Val;
}

void HexaHardwarePWM::configureTimer(Tcc* timer,
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

void HexaHardwarePWM::setDutyCycle(Tcc* timer,
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

void HexaHardwarePWM::setDutyThrottle(float percent)
{ // 13
    setDutyCycle(_motorTimer, 0, percent);
}

void HexaHardwarePWM::setDutySteer(float percent)
{ // 11
    setDutyCycle(_motorTimer, 1, percent);
}

void HexaHardwarePWM::setDutyPinTurretZ(float percent)
{ // 12
    setDutyCycle(_servoTimer, 2, percent);
}

void HexaHardwarePWM::setDutyPinTurretX(float percent)
{ // 6
    setDutyCycle(_servoTimer, 1, percent);
}

void HexaHardwarePWM::setDutyPinTurretY(float percent)
{ // 5
    setDutyCycle(_servoTimer, 3, percent);
}

void HexaHardwarePWM::setDutyPinAux(float percent)
{ // 9
    setDutyCycle(_auxTimer, 0, percent);
}
