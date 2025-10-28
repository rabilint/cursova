//
// Created by rabilint on 18.10.25.
//

#include "SensorDataManager.h"
#include <array>
#include <cmath>

SensorDataManager::SensorDataManager(const std::string& fileName)
{

    if (sqlite3_open(fileName.c_str(), &db_handle))
    {
        std::cerr << "Can't open database " << fileName << std::endl;
        return;
    }
    createTables();

}

SensorDataManager::~SensorDataManager()
{
    if (db_handle)
    {
        sqlite3_close(db_handle);
    }
}


void SensorDataManager::createTables()
{
    std::lock_guard<std::mutex> lock(db_mutex);

    const char* sql = "CREATE TABLE IF NOT EXISTS SensorsID (SensorID INTEGER PRIMARY KEY AUTOINCREMENT, SensorName TEXT NOT NULL UNIQUE )";

    char* errMsg = nullptr;
    if (sqlite3_exec(db_handle, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    const char* sql2 = "CREATE TABLE IF NOT EXISTS SensorData (ReadingID INTEGER PRIMARY KEY AUTOINCREMENT, Timestamp INTEGER NOT NULL ,SensorID INTEGER NOT NULL,  Data REAL NOT NULL, FOREIGN KEY(SensorID) REFERENCES SensorsID(SensorID))";
    if (sqlite3_exec(db_handle, sql2, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool SensorDataManager::setBasicSensors()
{
    if (!db_handle) {
        std::cerr << "Database handle is null. Cannot insert data." << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(db_mutex);


    //sensors
    std::array<const char*, 4> sensorNames = {
        "Temperature","Humidity","Gas", "Pressure"
    };
    char* errMsg = nullptr;

    if (sqlite3_exec(db_handle,"BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Begin transaction failed: " << sqlite3_errmsg(db_handle) << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    const char* sql = "INSERT OR IGNORE INTO SensorsID (SensorName) VALUES (?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Failed to prepare: " << sqlite3_errmsg(db_handle) << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    bool success = true;

    for (const char* name : sensorNames)
    {
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << "Failed to insert sensor: " << sqlite3_errmsg(db_handle) << std::endl;
            success = false;
            break;
        }
        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    const char* final_sql = success ? "COMMIT" : "ROLLBACK";

    if (sqlite3_exec(db_handle, final_sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Failed to " << (success ? "commit" : "rollback") << " transaction: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return success;
}



bool SensorDataManager::insertData(int sensorID, double Data)
{
    if (!db_handle) {
        std::cerr << "Database handle is null. Cannot insert data." << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(db_mutex);

    const char* sql = "INSERT INTO SensorReadings (Timestamp, SensorID, Data) VALUES (?,?,?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql,-1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, time(nullptr));
    sqlite3_bind_int(stmt, 2, sensorID);
    sqlite3_bind_double(stmt, 3, Data);

    bool success = true;
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_handle) << std::endl;
        sqlite3_finalize(stmt);
        success = false;
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << sqlite3_errmsg(db_handle) << std::endl;
        success = false;
    }
    return success;
}


std::vector<SensorDataStruct> SensorDataManager::getLastNReadings(int n)
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

    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
    }
    return records;
}

std::vector<SensorDataStruct> SensorDataManager::getReadingsInTimeRange(time_t start_from, time_t endWhen)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<SensorDataStruct> records;
    const char* sql = "SELECT Timestamp, Temperature, Humidity FROM SensorReadings WHERE Timestamp BETWEEN ? and ? ORDER BY Timestamp";
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

