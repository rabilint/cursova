//
// Created by rabilint on 18.09.25.
//

#include "DatabaseManager.h"
#include <iostream>
#include <ctime>

DBManager::DBManager(const std::string& fileName)
{
    if (sqlite3_open(fileName.c_str(), &db_handle)   != SQLITE_OK)
    {
        std::cerr << "Can't open database " << fileName << std::endl;
        return;
    }
    createTables();

}


DBManager::~DBManager()
{
    if (db_handle){
        sqlite3_close(db_handle);
    }
}

void DBManager::createTables()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* sql = "CREATE TABLE IF NOT EXISTS SensorReadings ("
                      "ReadingID INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "Timestamp INTEGER NOT NULL,"
                      "Temperature REAL NOT NULL,"
                      "Humidity REAL NOT NULL, ";

    char* errMsg = 0;
    if (sqlite3_exec(db_handle, sql, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool DBManager::insertData(float temperature, float humidity)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* sql = "INSERT INTO SensorReadings (Timestamp, Temperature, Humidity) VALUES (?,?,?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_handle, sql,-1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, time(nullptr));
    sqlite3_bind_double(stmt, 2, temperature);
    sqlite3_bind_double(stmt, 3, humidity);

    bool success = true;
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_handle) << std::endl;
        success = false;
    }
    sqlite3_finalize(stmt);
    return success;
}

std::vector<SensorDataStruct> DBManager::getLastNReadings(int n)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<SensorDataStruct> records;
    const char* sql = "SELECT Timestamp, Temperature, Humidity FROM SensorReadings ORDER BY Timestamp DESC LIMIT ? ";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return records;
    }

    sqlite3_bind_int(stmt, 1, n);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        SensorDataStruct record{};
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.temperature = static_cast<float>(sqlite3_column_double(stmt, 1));
        record.humidity = static_cast<float>( sqlite3_column_double(stmt, 2));
        records.push_back(record);
    }
    sqlite3_finalize(stmt);
    return records;
}

std::vector<SensorDataStruct> DBManager::getReadingsInTimeRange(time_t start_from, time_t endWhen)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<SensorDataStruct> records;
    const char* sql = "SELECT Timestamp, Temperature, Humidity FROM SensorReadings WHERE Timestamp BETWEEN ?,? ORDER BY Timestamp, ASC";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return records;
    }

    sqlite3_bind_int64(stmt, 1, start_from);
    sqlite3_bind_int64(stmt, 2, endWhen);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        SensorDataStruct record{};
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.temperature = static_cast<float>( sqlite3_column_double(stmt, 1) );
        record.humidity = static_cast<float>( sqlite3_column_double(stmt, 2) );
        records.push_back(record);
    }
    sqlite3_finalize(stmt);
    return records;
}
