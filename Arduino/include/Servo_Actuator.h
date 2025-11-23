//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_SERVO_ACTUATOR_H
#define ARDUINO_SERVO_ACTUATOR_H
#include "IActuator.h"
#include "Servo.h"

class Servo_Actuator final : public IActuator
{
private:
    String name;
    int pin;
    bool state;
    Servo servo;

public:
    Servo_Actuator(const char* actuatorName, int pinNumber);
    bool initialize() override;
    void turnOn() override;
    void turnOff() override;
    bool getState() const override;
    String getName() const override;
    int getPin() const override;
};


#endif //ARDUINO_SERVO_ACTUATOR_H