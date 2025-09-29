//
// Created by rabilint on 18.09.25.
//
#pragma once
#include <string>
#include <mutex>
#include <vector>
#include "./vendor/sqlite3/sqlite3.h"
#ifndef RECEIVER_DATABASEMANAGER_H
#define RECEIVER_DATABASEMANAGER_H

struct SensorDataStruct
{
    time_t timestamp;
    float temperature;
    float humidity;
};

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


class DBManager
{
    public:
    explicit DBManager(const std::string& fileName);
    explicit DBManager(const std::string& fileName, bool isActuator);
    ~DBManager();

    DBManager(const DBManager&) = delete;
    DBManager operator = (const DBManager&) = delete;

    bool insertData(float temperature, float humidity);
    std::vector<SensorDataStruct> getLastNReadings(int n);
    std::vector<SensorDataStruct> getReadingsInTimeRange(time_t start_from, time_t endWhen);

    bool addEvent(const std::string& Actuator, const int& state );
    int getActuatorID(const std::string& Actuator);
    std::vector<ActuatorDataStruct> getActuatorsData(int n);
    bool deleteActuator(const std::string& ActuatorName);
    bool addActuator(const std::string& Actuator);
    void listActuators();
    std::string getActuatorName(const int& ActuatorID) const;
    bool UpdateActuatorState(const int& ActuatorID, const int& state);

private:
    sqlite3* db_handle{};
    std::mutex db_mutex;
    void createTables();
    void createActuatorTables();
    void createActuatorsLib();

};

#endif //RECEIVER_DATABASEMANAGER_H