//
// Created by rabilint on 11.11.25.
//
//

#ifndef RECEIVER_ACTUATORSERVICE_H
#define RECEIVER_ACTUATORSERVICE_H

#include "IRepository.h"
#include "ISerialPort.h"
#include <memory>
#include <vector>
#include <string>

class ActuatorService {
private:
    std::shared_ptr<IActuatorRepository> repository;
    std::shared_ptr<ISerialPort> serialPort;

public:
    ActuatorService(std::shared_ptr<IActuatorRepository> repo, std::shared_ptr<ISerialPort> serial);
    [[nodiscard]] bool addActuator(const std::string& actuatorName) const;
    [[nodiscard]] bool deleteActuator(const std::string& actuatorName) const;
    [[nodiscard]] std::vector<ActuatorStruct> listActuators() const;
    bool controlActuator(int actuatorID, const std::string& action) const;
    void displayActuators() const;
    void displayActuatorEvents(int n) const;
    std::vector<ActuatorStruct> getActuatorsForHandshake() const;
};
#endif //RECEIVER_ACTUATORSERVICE_H