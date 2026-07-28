#pragma once
 
#include <Arduino.h>
 
// Pan currently uses 360 degree continuous servo
// Tilt uses 270 degree positional servo
 
namespace PanServoConfig {
    //  Duty cycle range 50Hz (20ms period): test range 1ms min and 2ms max, or 5% and 10%
    static constexpr float SERVO_MIN_DUTY_PERCENT = 5.0f;
    static constexpr float SERVO_MAX_DUTY_PERCENT = 10.0f;
    static constexpr float STOP_DUTY_PERCENT = (SERVO_MIN_DUTY_PERCENT + SERVO_MAX_DUTY_PERCENT) / 2.0f;
    // joystick deadzone, in raw 0 to 255 so centered on 128
    static constexpr int JOYSTICK_DEADZONE = 10;
}
 
class ContinuousServoAxis {
public:
    ContinuousServoAxis() {}
 
    float computeDutyPercentFromJoystick(uint8_t duty) {
        const int center = 128;
        int offset = int(duty) - center;
 
        if (abs(offset) < PanServoConfig::JOYSTICK_DEADZONE) {
            _lastSpeedPercent = 0.0f;
            return PanServoConfig::STOP_DUTY_PERCENT;
        }
 
        if (offset > 0) {
            float t = float(offset - PanServoConfig::JOYSTICK_DEADZONE) /
                      float(127 - PanServoConfig::JOYSTICK_DEADZONE);
            t = constrain(t, 0.0f, 1.0f);
            _lastSpeedPercent = t * 100.0f;
            return PanServoConfig::STOP_DUTY_PERCENT +
                   t * (PanServoConfig::SERVO_MAX_DUTY_PERCENT - PanServoConfig::STOP_DUTY_PERCENT);
        } else {
            float t = float(-offset - PanServoConfig::JOYSTICK_DEADZONE) /
                      float(128 - PanServoConfig::JOYSTICK_DEADZONE);
            t = constrain(t, 0.0f, 1.0f);
            _lastSpeedPercent = -t * 100.0f;
            return PanServoConfig::STOP_DUTY_PERCENT -
                   t * (PanServoConfig::STOP_DUTY_PERCENT - PanServoConfig::SERVO_MIN_DUTY_PERCENT);
        }
    }
 
    // -100 = full speed one way, 0 = stopped, +100 = full speed the other way.
    float getLastCommandedSpeedPercent() const { return _lastSpeedPercent; }
 
private:
    float _lastSpeedPercent = 0.0f;
};
 
namespace TiltServoConfig {
    static constexpr float SERVO_MAX_ANGLE_DEG = 270.0f;
    static constexpr float SERVO_MIN_DUTY_PERCENT = 5.0f;
    static constexpr float SERVO_MAX_DUTY_PERCENT = 10.0f;
}
 
class PositionalServoAxis {
public:
    PositionalServoAxis(float outputMinDeg, float outputMaxDeg,
                         float servoMaxAngleDeg = TiltServoConfig::SERVO_MAX_ANGLE_DEG,
                         float minDutyPercent = TiltServoConfig::SERVO_MIN_DUTY_PERCENT,
                         float maxDutyPercent = TiltServoConfig::SERVO_MAX_DUTY_PERCENT)
        : _outputMin(outputMinDeg), _outputMax(outputMaxDeg),
          _servoMaxAngleDeg(servoMaxAngleDeg),
          _minDuty(minDutyPercent), _maxDuty(maxDutyPercent),
          _currentOutputDeg((outputMinDeg + outputMaxDeg) / 2.0f) {}
 
    float computeDutyPercentFromJoystick(uint8_t duty) {
        float outputDeg = _outputMin + (float(duty) / 255.0f) * (_outputMax - _outputMin);
        return computeDutyPercent(outputDeg);
    }
 
    float computeDutyPercent(float outputDeg) {
        outputDeg = constrain(outputDeg, _outputMin, _outputMax);
        float servoDeg = constrain(outputDeg, 0.0f, _servoMaxAngleDeg);
        float duty = _minDuty + (servoDeg / _servoMaxAngleDeg) * (_maxDuty - _minDuty);
        _currentOutputDeg = outputDeg;
        return duty;
    }
 
    float getOutputAngle() const { return _currentOutputDeg; }
 
private:
    float _outputMin, _outputMax;
    float _servoMaxAngleDeg;
    float _minDuty, _maxDuty;
    float _currentOutputDeg;
};
 
