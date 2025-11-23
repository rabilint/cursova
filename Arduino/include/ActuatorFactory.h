//
// Created by rabilint on 22.11.25.
//
#ifndef ARDUINO_ACTUATORFACTORY_H
#define ARDUINO_ACTUATORFACTORY_H

#include "Arduino.h"
#include "IActuator.h"
#include <memory>
#include <optional>

#include "Serial_Handler.h"
class ActuatorFactory
{
    public:
    static std::optional<std::shared_ptr<IActuator>> createActuator( const char* name, int pin, int initialState, Serial_Handler& serial);
};


#endif //ARDUINO_ACTUATORFACTORY_H