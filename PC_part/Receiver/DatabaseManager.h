//
// Created by rabilint on 18.09.25.
//
#pragma once
#include <string>
#include <mutex>
#include "ActuatorDataManager.h"
#include "SensorDataManager.h"
#include <memory>

#ifndef RECEIVER_DATABASEMANAGER_H
#define RECEIVER_DATABASEMANAGER_H

class DBManager
{
public:
    explicit DBManager(const std::string& SensorDBFileName, const std::string& ActuatorDBFileName)
    {
        sensors = std::make_unique<SensorDataManager>(SensorDBFileName);
        actuators = std::make_unique<ActuatorDataManager>(ActuatorDBFileName);
    }

    ~DBManager() = default;
    DBManager(const DBManager&) = delete;
    DBManager operator = (const DBManager&) = delete;

    SensorDataManager& sensorManager() { return *sensors; }
    ActuatorDataManager& actuatorManager() { return *actuators; }

private:
    std::unique_ptr<SensorDataManager> sensors;
    std::unique_ptr<ActuatorDataManager> actuators;

};



#endif //RECEIVER_DATABASEMANAGER_H
