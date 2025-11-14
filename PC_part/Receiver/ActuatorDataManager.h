//
// Created by rabilint on 18.10.25.
//
#pragma once
#include <string>
#include <mutex>
#include <iostream>
#include <vector>
#include "IRepository.h"
#include "./vendor/sqlite3/sqlite3.h"

#ifndef RECEIVER_ACTUATORDATAMANAGER_H
#define RECEIVER_ACTUATORDATAMANAGER_H



class ActuatorDataManager : public IActuatorRepository
{
    public:
    explicit ActuatorDataManager(const std::string& filename);
    ~ActuatorDataManager();

    ActuatorDataManager(const ActuatorDataManager&) = delete;
    ActuatorDataManager operator = (const ActuatorDataManager&) = delete;

    bool addEvent(const std::string& Actuator, const int& state );
    int getActuatorID(const std::string& Actuator);
    std::vector<ActuatorDataStruct> getActuatorsData(int n);
    bool deleteActuator(const std::string& ActuatorName);
    bool addActuator(const std::string& Actuator);
    std::vector<ActuatorStruct> listActuators();
    std::string getActuatorName(const int& ActuatorID) const;
    bool updateActuatorState(const int& ActuatorID, const int& state);

    private:
    sqlite3* db_handle{};
    std::mutex db_mutex;
    void createActuatorTables();
    void createActuatorsLib();

};


#endif //RECEIVER_ACTUATORDATAMANAGER_H