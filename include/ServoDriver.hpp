

#include <Arduino.h>


class ServoDriver
{
public:
    ServoDriver();
    void init_servo();
    void set_servo_duty(uint8_t duty);

private:

}