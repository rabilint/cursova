//
// Created by rabilint on 11.11.25.
// Бізнес-логіка актуаторів
//

#include "ActuatorService.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <utility>

ActuatorService::ActuatorService(std::shared_ptr<IActuatorRepository> repo, std::shared_ptr<ISerialPort> serial)
    : repository(std::move(repo)), serialPort(std::move(serial))
{}

bool ActuatorService::addActuator(const std::string& actuatorName) const
{
    std::string name = actuatorName;
    std::ranges::replace(name, ' ', '_');
    std::ranges::transform(name, name.begin(), ::toupper);
    return repository->addActuator(name);
}

bool ActuatorService::deleteActuator(const std::string& actuatorName) const
{
    std::string name = actuatorName;
    std::ranges::replace(name, ' ', '_');
    std::ranges::transform(name, name.begin(), ::toupper);
    return repository->deleteActuator(name);
}

std::vector<ActuatorStruct> ActuatorService::listActuators() const
{
    return repository->listActuators();
}

bool ActuatorService::controlActuator(int actuatorID, const std::string& action) const
{
    std::string actuatorName = repository->getActuatorName(actuatorID);
    if (actuatorName.empty())
    {
        return false;
    }

    std::string upperAction = action;
    std::ranges::transform(actuatorName, upperAction.begin(), ::toupper);

    int newState = -1;
    std::string actionPrefix;

    if (upperAction == "ON")
    {
        newState = 1;
        actionPrefix = "_ON";
    }else if (upperAction == "OFF")
    {
        newState = 0;
        actionPrefix = "_OFF";
    }else
    {
        return false;
    }

    if (serialPort && serialPort->isConnected())
    {
        serialPort->writeLine(actuatorName+actionPrefix);
        if (repository->updateActuatorState(actuatorID,newState))
        {
            repository->addEvent(actuatorName, newState);
        }
        return true;
    }
    std::cerr << "Failed to update actuator state: " << actuatorName << std::endl;
    return false;
}

void ActuatorService::displayActuators() const
{
    auto actuators_list = repository->listActuators();
    for (auto [ActuatorID, ActuatorName, State] : actuators_list)
    {
        std::cout << "| ID: " << ActuatorID;
        std::cout << " | Name: " << ActuatorName;
        std::cout << " | State: " << State;
        std::cout << " |" << std::endl;
    }
}

void ActuatorService::displayActuatorEvents(const int n) const
{
    auto events_list = repository->getActuatorsData(n);
    for (auto [ActuatorID, State, ActuatorName, timestamp] : events_list)
    {
        tm* gmt_time_info = gmtime(&timestamp);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", gmt_time_info);
        std::cout << "| ID: " << ActuatorID
                  << " | Name: " << ActuatorName
                  << " | State: " << State
                  << " | Time: " << buffer << std::endl;
    }
}

std::vector<ActuatorStruct> ActuatorService::getActuatorsForHandshake() const
{
    return listActuators();
}



