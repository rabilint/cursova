//
// Created by rabilint on 23.11.25.
//

#include "Servo_Actuator.h"
Servo_Actuator::Servo_Actuator(const char actuatorName[26], int pin)
    : name(actuatorName), pin(pin), state(false){}

bool Servo_Actuator::initialize()
{
    if (!servo.attached())
    {
        servo.attach(pin);
    }
    servo.write(0);
    state = false;
    return true;
}

int Servo_Actuator::getPin() const
{
    return pin;
}

String Servo_Actuator::getName() const
{
    return name;
}

bool Servo_Actuator::getState() const
{
    return state;
}

void Servo_Actuator::turnOn()
{
    if (!servo.attached())
    {
        servo.attach(pin);
    }
    servo.write(90);
    state = true;
}

void Servo_Actuator::turnOff()
{
    if (!servo.attached())
    {
        servo.attach(pin);
    }
    servo.write(0);
    state = false;
}





