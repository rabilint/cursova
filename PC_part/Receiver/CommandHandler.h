//
// Created by rabilint on 11.11.25.
//

#ifndef RECEIVER_COMMANDHANDLER_H
#define RECEIVER_COMMANDHANDLER_H

#include "ICommand.h"
#include "SensorService.h"
#include "ActuatorService.h"
#include <memory>
#include <string>
#include <map>

class CommandHandler : public ICommand {
private:
    std::shared_ptr<SensorService> sensorService;
    std::shared_ptr<ActuatorService> actuatorService;
    std::map<std::string, std::string> commandDescriptions;

public:
    CommandHandler(const std::shared_ptr<SensorService>& sensorSvc,
                   const std::shared_ptr<ActuatorService>& actuatorSvc);
    bool executeCommand(const std::string& command) override;
    void displayHelp() override;

};

#endif //RECEIVER_COMMANDHANDLER_H