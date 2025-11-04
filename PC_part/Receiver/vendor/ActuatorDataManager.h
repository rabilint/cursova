//
// Created by rabilint on 18.10.25.
//
#pragma once
#include <string>
#include <mutex>
#include <iostream>
#include <vector>
#include "./vendor/sqlite3/sqlite3.h"

#ifndef RECEIVER_ACTUATORDATAMANAGER_H
#define RECEIVER_ACTUATORDATAMANAGER_H

struct ActuatorDataStruct
{
    int ActuatorID;
    int State;
    std::string ActuatorName;
    time_t timestamp;
};

struct ActuatorStruct
{
    int ActuatorID;
    std::string ActuatorName;
    int State;
};


class ActuatorDataManager
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
    bool UpdateActuatorState(const int& ActuatorID, const int& state);

    private:
    sqlite3* db_handle{};
    std::mutex db_mutex;
    void createActuatorTables();
    void createActuatorsLib();

};


#endif //RECEIVER_ACTUATORDATAMANAGER_H