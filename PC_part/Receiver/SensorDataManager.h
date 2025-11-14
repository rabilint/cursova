//
// Created by rabilint on 18.10.25.
//
#pragma once
#include <string>
#include <mutex>
#include "IRepository.h"
#include <vector>
#include <iostream>
#include <map>
#include "./vendor/sqlite3/sqlite3.h"
#ifndef RECEIVER_SENSORDATAMANAGER_H
#define RECEIVER_SENSORDATAMANAGER_H

inline int sensorsReceived = 0;


struct SensorsStruct
{
    int sensorID{};
    std::string name{};
};

class SensorDataManager : public ISensorRepository
{
    public:
    explicit SensorDataManager(const std::string& fileName);
    ~SensorDataManager() override;

    SensorDataManager(const SensorDataManager&) = delete;
    SensorDataManager& operator=(const SensorDataManager&) = delete;

    bool insertData(int sensorID, double Data);
    std::vector<RecordDataStruct> getLastNReadings(int n, int amountOfSensors) override;
    std::vector<RecordDataStruct> getReadingsInTimeRange(time_t start_from, time_t endWhen) override;
    void synchronizeSensors(const std::map<int, std::string>& arduinoSensors) override;
    bool updateSensorName(int sensorID, const std::string& newName);
    bool insertNewSensor(int SensorId,const std::string& name);
    int amountOfSensors() override;



    private:
    std::map<int, SensorsStruct> getAllSensorsFromDB();
    sqlite3* db_handle{};
    std::mutex db_mutex;
    void createTables();


};


#endif //RECEIVER_SENSORDATAMANAGER_H