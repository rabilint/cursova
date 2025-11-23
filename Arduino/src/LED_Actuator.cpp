//
// Created by rabilint on 23.11.25.
//

#include "../include/LED_Actuator.h"

LED_Actuator::LED_Actuator(const char actuatorName[26], int pinNumber):
    name(actuatorName),
    pin(pinNumber),
    state(false) {}

int LED_Actuator::getPin() const
{
    return pin;
}

bool LED_Actuator::getState() const
{
    return state;
}
String LED_Actuator::getName() const
{
    return name;
}

bool LED_Actuator::initialize()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state ? HIGH : LOW);
    state = false;
    return true;
}

void LED_Actuator::turnOn()
{
    digitalWrite(pin, HIGH);
    state = true;
}
void LED_Actuator::turnOff()
{
    digitalWrite(pin, LOW);
    state = false;
}
