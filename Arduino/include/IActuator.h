//
// Created by rabilint on 22.11.25.
//

#ifndef ARDUINO_IACTUATOR_H
#define ARDUINO_IACTUATOR_H
#include <Arduino.h>

constexpr int MAX_ACTUATORS = 10;


class IActuator {
public:
    virtual ~IActuator() = default;
    virtual bool initialize() = 0;
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual bool getState() const = 0;
    virtual String getName() const = 0;
    virtual int getPin() const = 0;
};

#endif //ARDUINO_IACTUATOR_H