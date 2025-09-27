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

class DBManager
{
    public:
    explicit DBManager(const std::string& fileName);
    ~DBManager();

    DBManager(const DBManager&) = delete;
    DBManager operator = (const DBManager&) = delete;

    bool insertData(float temperature, float humidity);
    std::vector<SensorDataStruct> getLastNReadings(int n);
    std::vector<SensorDataStruct> getReadingsInTimeRange(time_t start_from, time_t endWhen);


private:
    sqlite3* db_handle{};
    std::mutex db_mutex;
    void createTables();

};

#endif //RECEIVER_DATABASEMANAGER_H