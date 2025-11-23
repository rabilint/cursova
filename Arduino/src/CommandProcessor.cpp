//
// Created by rabilint on 23.11.25.
//

#include "CommandProcessor.h"


void CommandProcessor::setActuators(const std::array<std::optional<std::shared_ptr<IActuator>>, MAX_ACTUATORS>& inputActuators)
{
    this->actuators = inputActuators;
}

bool CommandProcessor::processCommand(const String& command) {
    for (const auto& actuatorOpt : actuators) {
        if (actuatorOpt && *actuatorOpt)
        {
            auto ptr = *actuatorOpt;

            String onCommand = ptr->getName() + "_ON";
            String offCommand = ptr->getName() + "_OFF";

            if (command.equalsIgnoreCase(onCommand)) {
                ptr->turnOn();
                return true;
            }

            if (command.equalsIgnoreCase(offCommand)) {
                ptr->turnOff();
                return true;
            }
        }
    }
    return false;
}

