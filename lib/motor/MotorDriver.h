#pragma once

#include "PentaHWPwm.hpp"
#include "Joystick.hpp"
#include "TurretGearMap.h"

// HRW values measured from the cars original controller
namespace MotorConfig {
    static constexpr float NEUTRAL_THROTTLE = 9.00f;
    static constexpr float NEUTRAL_STEERING = 7.30f;

    static constexpr float THROTTLE_MIN = 7.0f;
    static constexpr float THROTTLE_MAX = 11.0f;

    static constexpr float STEERING_MIN = 5.5f;
    static constexpr float STEERING_MAX = 12.5f;
}

class MotorDriver {
public:
    MotorDriver(bool debug = false)
        : 
        _pwm(), // 9 throttle, 5 steering, 6 servo1, 7 servo2 (I think?)
        _debug(debug),
        _turretPan(), //continuous servo, no gear ratio/angle range to configure
        _turretTilt(0.0f, 60.0f, TiltServoConfig::SERVO_MAX_ANGLE_DEG) {}
    
    void init_motor(){
        _pwm.begin(60, 50); // 60Hz for motors, 50Hz for servos
        _pwm.setDuty13(0);
        _pwm.setDuty11(0);

        _pwm.setDuty5(0);
        _pwm.setDuty6(0);
        _pwm.setDuty12(0);
    }

    void setThrottlePWM(uint8_t duty) { // input duty is 0-255 from joystick
        float map_duty = mapAroundNeutral(duty,
            JoystickConfig::JOY_MIN,
            JoystickConfig::JOY_CENTER,
            JoystickConfig::JOY_MAX,
            MotorConfig::THROTTLE_MIN,
            MotorConfig::NEUTRAL_THROTTLE,
            MotorConfig::THROTTLE_MAX
        );

        _pwm.setDuty13(map_duty);//output is mapped to 7-11 for throttle control with float precision
    }

    void setSteerPWM(uint8_t duty) {
    float map_duty = mapAroundNeutral(duty,
        JoystickConfig::JOY_MIN,
        JoystickConfig::JOY_CENTER,
        JoystickConfig::JOY_MAX,
        MotorConfig::STEERING_MIN,
        MotorConfig::NEUTRAL_STEERING,
        MotorConfig::STEERING_MAX
    );
             
        _pwm.setDuty11(map_duty);
    }

    //duty percent using the gear ratio math
    void setTurretPan(uint8_t duty) {
        Serial.println("Setting turret pan duty: " + String(duty));
        _pwm.setDuty5(_turretPan.computeDutyPercentFromJoystick(duty));
    }
    void setTurretTilt(uint8_t duty) {
        Serial.println("Setting turret tilt duty: " + String(duty));    
        _pwm.setDuty6(_turretTilt.computeDutyPercentFromJoystick(duty));
    }

    void setCameraMux(bool duty){
        if (duty == true) _pwm.setDuty12(5.0F);
        else _pwm.setDuty12(3.0F);
    }


    float getTurretPanSpeedPercent() const { return _turretPan.getLastCommandedSpeedPercent(); }
    float getTurretTiltAngle() const { return _turretTilt.getOutputAngle(); }
    
private:
    PentaHardwarePWM _pwm;
    bool _debug;
    ContinuousServoAxis _turretPan;
    PositionalServoAxis _turretTilt;

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
