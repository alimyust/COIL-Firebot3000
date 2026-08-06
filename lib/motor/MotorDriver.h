#pragma once

#include "PentaHWPwm.hpp"
#include "Joystick.hpp"
#include "TurretGearMap.h"

// HRW values measured from the cars original controller
namespace MotorConfig {
    static constexpr float NEUTRAL_THROTTLE = 9.00f;
    static constexpr float NEUTRAL_STEERING = 7.30f;

    static constexpr float THROTTLE_MIN = 7.0f;
    static constexpr float THROTTLE_MAX = 10.0f;

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
        _turretTilt(0.0f, 60.0f, TiltServoConfig::SERVO_MAX_ANGLE_DEG),
        _currentPanDutyPercent(PanServoConfig::STOP_DUTY_PERCENT),
        _currentTiltDutyPercent((TiltServoConfig::SERVO_MIN_DUTY_PERCENT + TiltServoConfig::SERVO_MAX_DUTY_PERCENT) / 2.0f),
        _currentTiltOutputDeg(30.0f),
        _panCommandAccumulator(0.0f),
        _tiltCommandAccumulator(0.0f) {}
    
    void init_motor(){
        _pwm.begin(60, 50); // 60Hz for motors, 50Hz for servos
        _pwm.setDuty13(0);
        _pwm.setDuty11(0);

        _pwm.setDuty5(7.5f); // set turret pan to neutral
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

    void setTurretPan(uint8_t duty) {
        const int center = JoystickConfig::JOY_CENTER;
        const int offset = int(duty) - center;
        const float deadzone = float(PanServoConfig::JOYSTICK_DEADZONE);

        if (abs(offset) < deadzone) {
            _pwm.setDuty5(PanServoConfig::STOP_DUTY_PERCENT);
            return;
        }

        const float magnitude = constrain(abs(offset) / 127.0f, 0.0f, 1.0f);
        const float targetDuty = (offset > 0)
            ? PanServoConfig::STOP_DUTY_PERCENT + magnitude * (PanServoConfig::SERVO_MAX_DUTY_PERCENT - PanServoConfig::STOP_DUTY_PERCENT)
            : PanServoConfig::STOP_DUTY_PERCENT - magnitude * (PanServoConfig::STOP_DUTY_PERCENT - PanServoConfig::SERVO_MIN_DUTY_PERCENT);

        const float clampedDuty = constrain(targetDuty,
                                            PanServoConfig::SERVO_MIN_DUTY_PERCENT,
                                            PanServoConfig::SERVO_MAX_DUTY_PERCENT);
        _pwm.setDuty5(clampedDuty);
    }

    void setTurretTilt(uint8_t duty) {
        const int center = JoystickConfig::JOY_CENTER;
        const int offset = int(duty) - center;
        const float deadzone = 10.0f;

        if (abs(offset) < deadzone) {
            _pwm.setDuty6(_currentTiltDutyPercent);
            return;
        }

        const float magnitude = constrain(abs(offset) / 127.0f, 0.0f, 1.0f);
        const float targetOutputDeg = (offset > 0)
            ? constrain(_currentTiltOutputDeg + magnitude * 8.0f, 0.0f, 60.0f)
            : constrain(_currentTiltOutputDeg - magnitude * 8.0f, 0.0f, 60.0f);
        const float targetDuty = TiltServoConfig::SERVO_MIN_DUTY_PERCENT +
            (targetOutputDeg / 60.0f) * (TiltServoConfig::SERVO_MAX_DUTY_PERCENT - TiltServoConfig::SERVO_MIN_DUTY_PERCENT);

        const float responseRate = 0.04f + (0.10f * magnitude);
        _currentTiltDutyPercent += (targetDuty - _currentTiltDutyPercent) * responseRate;
        const float minDuty = TiltServoConfig::SERVO_MIN_DUTY_PERCENT + 0.5f;
        const float maxDuty = TiltServoConfig::SERVO_MAX_DUTY_PERCENT - 0.5f;
        _currentTiltDutyPercent = constrain(_currentTiltDutyPercent,
                                            minDuty,
                                            maxDuty);
        _currentTiltOutputDeg = targetOutputDeg;

        _pwm.setDuty6(_currentTiltDutyPercent);
    }

    void setCameraMux(bool duty){
        if (duty == true) _pwm.setDuty12(6.0F);
        else _pwm.setDuty12(9.0F);
    }

    float getTurretPanSpeedPercent() const {
        return (_currentPanDutyPercent - PanServoConfig::STOP_DUTY_PERCENT) * 100.0f /
               (PanServoConfig::SERVO_MAX_DUTY_PERCENT - PanServoConfig::STOP_DUTY_PERCENT);
    }
    float getTurretTiltAngle() const { return _currentTiltOutputDeg; }
    
private:
    PentaHardwarePWM _pwm;
    bool _debug;
    ContinuousServoAxis _turretPan;
    PositionalServoAxis _turretTilt;
    float _currentPanDutyPercent;
    float _currentTiltDutyPercent;
    float _currentTiltOutputDeg;
    float _panCommandAccumulator;
    float _tiltCommandAccumulator;

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
