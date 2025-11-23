//
// Created by rabilint on 22.11.25.
//

#include "ActuatorFactory.h"
#include <optional>
#include "LED_Actuator.h"
#include "Servo_Actuator.h"

std::optional<std::shared_ptr<IActuator>> ActuatorFactory::createActuator(const char* name, int pin, int initialState, Serial_Handler& serial)
{
    std::shared_ptr<IActuator> actuator;
    if (strcmp(name, "LED_RED") == 0)
    {
        actuator = std::make_shared<LED_Actuator>(name, pin);
    }else if (strcmp(name, "SERVO") == 0)
    {
        actuator = std::make_shared<Servo_Actuator>(name, pin);
    }else
    {
        serial.sendLine (F("ERROR:UNSUPPORTED_ACTUATOR!"));
        return std::nullopt;
    }

    if (actuator && actuator->initialize())
    {
        if (initialState == 1)
        {
            actuator->turnOn();
        }
        else
        {
            actuator->turnOff();
        }
    }

    return actuator;
}
