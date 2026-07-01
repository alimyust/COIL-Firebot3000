#pragma once

#include "ProtocolLayer.hpp"
#include "QuadHWPwm.hpp"
#include "Joystick.hpp"
#include "DebugLog.hpp"

// HRW values measured from the cars original controller
namespace MotorConfig {
    static constexpr float NEUTRAL_THROTTLE = 9.00f;
    static constexpr float NEUTRAL_STEERING = 8.30f;

    static constexpr float THROTTLE_MIN = 7.0f;
    static constexpr float THROTTLE_MAX = 11.0f;

    static constexpr float STEERING_MIN = 5.5f;
    static constexpr float STEERING_MAX = 12.5f;
}

class MotorDriver {
public:
    MotorDriver(bool debug = false)
        : 
        _pwm(9, 5, 6, 7), // 9 throttle, 5 steering, 6 servo1, 7 servo2 (I think?)
        _debug(debug) {}
    
    void init_motor(){
        _pwm.begin(60, 50); // 60Hz for motors, 50Hz for servos
        _pwm.setDutyCycle1(0);
        _pwm.setDutyCycle2(0);
        _pwm.setDutyCycle3(0);
        _pwm.setDutyCycle4(0);
    }

    void setThrottle(uint8_t duty) { // input duty is 0-255 from joystick
        float map_duty = mapAroundNeutral(duty,
            JoystickConfig::JOY_MIN,
            JoystickConfig::JOY_CENTER,
            JoystickConfig::JOY_MAX,
            MotorConfig::THROTTLE_MIN,
            MotorConfig::NEUTRAL_THROTTLE,
            MotorConfig::THROTTLE_MAX
        );

        _pwm.setDutyCycle1(map_duty);//output is mapped to 7-11 for throttle control with float precision
    }

    void setSteeringDuty(uint8_t duty) {
    float map_duty = mapAroundNeutral(duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::STEERING_MIN,
        MotorConfig::NEUTRAL_STEERING,
        MotorConfig::STEERING_MAX
    );
             
        _pwm.setDutyCycle2(map_duty);
    }


private:
    QuadHardwarePWM _pwm;
    bool _debug;

    static float mapAroundNeutral(uint8_t value,
                              uint8_t in_min, uint8_t in_center, uint8_t in_max,
                              float out_min, float out_neutral, float out_max) {
    if (value <= in_center) {
        float t = float(value - in_min) / float(in_center - in_min);
        return out_min + t * (out_neutral - out_min);
    } else {
        float t = float(value - in_center) / float(in_max - in_center);
        return out_neutral + t * (out_max - out_neutral);
    }
}

};
