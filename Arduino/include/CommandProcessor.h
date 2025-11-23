//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_COMMANDPROCESSOR_H
#define ARDUINO_COMMANDPROCESSOR_H

#include "ICommandProcessor.h"
#include "IActuator.h"
#include <memory>
#include <optional>


class   CommandProcessor : public ICommandProcessor
{
private:
    std::array<std::optional<std::shared_ptr<IActuator>> , MAX_ACTUATORS> actuators;

public:
    void setActuators(const std::array<std::optional<std::shared_ptr<IActuator>>, MAX_ACTUATORS>& inputActuators);
    bool processCommand(const String& command) override;

};


#endif //ARDUINO_COMMANDPROCESSOR_H