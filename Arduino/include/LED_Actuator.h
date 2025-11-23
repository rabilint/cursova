//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_LED_ACTUATOR_H
#define ARDUINO_LED_ACTUATOR_H
#include "IActuator.h"


class LED_Actuator final : public IActuator
{
private:
    String name;
    int pin = 0;
    bool state = false;

    public:
    LED_Actuator(const char* actuatorName, int pinNumber);
    bool initialize() override;
    void turnOn() override;
    void turnOff() override;
    bool getState() const override;
    String getName() const override;
    int getPin() const override;
};


#endif //ARDUINO_LED_ACTUATOR_H