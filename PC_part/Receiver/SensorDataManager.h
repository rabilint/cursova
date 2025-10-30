//
// Created by rabilint on 18.10.25.
//
#pragma once
#include <string>
#include <mutex>
#include <vector>
#include <iostream>
#include "./vendor/sqlite3/sqlite3.h"
#ifndef RECEIVER_SENSORDATAMANAGER_H
#define RECEIVER_SENSORDATAMANAGER_H

struct SensorDataStruct
{
    time_t timestamp;
    float temperature;
    float humidity;
};

struct SensorsStruct
{
    int sensorID{};
    std::string name{};
};

class SensorDataManager
{
    public:
    explicit SensorDataManager(const std::string& fileName);
    ~SensorDataManager();

    SensorDataManager(const SensorDataManager&) = delete;
    SensorDataManager& operator=(const SensorDataManager&) = delete;

    bool insertData(int sensorID, double Data);
    std::vector<SensorDataStruct> getLastNReadings(int n);
    std::vector<SensorDataStruct> getReadingsInTimeRange(time_t start_from, time_t endWhen);
    bool setBasicSensors(std::vector<const char*> sensorNames);

    private:
    sqlite3* db_handle{};
    std::mutex db_mutex;
    void createTables();


};


#endif //RECEIVER_SENSORDATAMANAGER_H