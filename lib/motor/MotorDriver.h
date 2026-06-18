

#include "ProtocolLayer.hpp"
#include "DualHWPwm.hpp"
#include "Joystick.hpp"


// HRW values measured from the cars original controller
namespace MotorConfig {
    static constexpr float NEUTRAL_THROTTLE = 9.00f;
    static constexpr float NEUTRAL_STEERING = 9.00f;

    static constexpr float THROTTLE_MIN = 7.0f;
    static constexpr float THROTTLE_MAX = 11.0f;

    static constexpr float STEERING_MIN = 6.5f;
    static constexpr float STEERING_MAX = 12.5f;
}

class MotorDriver {
public:
    MotorDriver(bool debug = false)
        : 
        _pwm(9, 5),
        _debug(debug) {}
    
    void init_motor(){
        _pwm.begin(60);
        _pwm.setDutyCycle1(0);
        _pwm.setDutyCycle2(0);
    }

    void setThrottle(uint8_t duty) { // input duty is 0-255 from joystick
        float map_duty = mapf(duty, JoystickConfig::JOY_MIN, JoystickConfig::JOY_MAX, 
            MotorConfig::THROTTLE_MIN, MotorConfig::THROTTLE_MAX);
            
        _pwm.setDutyCycle1(map_duty);//output is mapped to 7-11 for throttle control with float precision
        
        if (_debug) {
            Serial.print("Set throttle duty: ");
            Serial.print(duty);
            Serial.print(" mapped to ");
            Serial.println(map_duty);
        }
    }

    void setSteeringDuty(uint8_t duty) {
        float map_duty = mapf(duty, JoystickConfig::JOY_MIN, JoystickConfig::JOY_MAX,
             MotorConfig::STEERING_MIN, MotorConfig::STEERING_MAX);
        _pwm.setDutyCycle2(map_duty);
        if (_debug) {
            Serial.print("Set steering duty: ");
            Serial.print(duty);
            Serial.print(" mapped to ");
            Serial.println(map_duty);
        }
    }


private:
    DualHardwarePWM _pwm;
    bool _debug;

    static float mapf(float x, float in_min, float in_max, float out_min, float out_max){
        return (x - in_min) * (out_max - out_min) /(in_max - in_min) + out_min;
    }

};
