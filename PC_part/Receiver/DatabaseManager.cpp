//
// Created by rabilint on 18.09.25.
//

#include "DatabaseManager.h"
#include <iostream>
#include <ctime>

DBManager::DBManager(const std::string& fileName)
{
    int rc;
    rc = sqlite3_open(fileName.c_str(), &db_handle);
    if (rc)
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
    const char* sql = "CREATE TABLE IF NOT EXISTS SensorReadings (ReadingID INTEGER PRIMARY KEY AUTOINCREMENT, Timestamp INTEGER NOT NULL, Temperature REAL NOT NULL, Humidity REAL NOT NULL); ";

    char* errMsg = nullptr;
    if (sqlite3_exec(db_handle, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool DBManager::insertData(float temperature, float humidity)
{
    if (!db_handle) {
        std::cerr << "Database handle is null. Cannot insert data." << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(db_mutex);

    const char* sql = "INSERT INTO SensorReadings (Timestamp, Temperature, Humidity) VALUES (?,?,?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql,-1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, time(nullptr));
    sqlite3_bind_double(stmt, 2, temperature);
    sqlite3_bind_double(stmt, 3, humidity);

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

    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
    }
    return records;
}

std::vector<SensorDataStruct> DBManager::getReadingsInTimeRange(time_t start_from, time_t endWhen)
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
    for (SensorDataStruct& record : records)
    {
        std::cout << record.timestamp << " T: " <<  record.temperature << " H: "<< record.humidity << std::endl;
    }
    return records;
}

// For Actuators

int DBManager::getActuatorID(const std::string& Actuator) // Technical func
{
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT ActuatorID FROM Actuator WHERE Name = ?";
    int ActuatorID = -1;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return -1;
    }
    sqlite3_bind_text(stmt, 1, Actuator.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ActuatorID = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return ActuatorID;
}

bool DBManager::addEvent(const std::string& Actuator, const int& state )
{
    if (!db_handle) {
        std::cerr << "Database handle is null. Cannot insert data." << std::endl;
        return false;
    }
    std::lock_guard<std::mutex> lock(db_mutex);
    int ActuatorID = getActuatorID(Actuator);
    if (ActuatorID == -1)
    {
        std::cerr << "ActuatorID is invalid. Cannot insert data." << std::endl;
        return false;
    }

    const char* sql = "INSERT INTO ActuatorEvents (ActuatorID, State, Timestamp) VALUES (?,?,?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, ActuatorID);
    sqlite3_bind_int(stmt, 2, state);
    sqlite3_bind_int64(stmt, 3, time(nullptr));

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_handle) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

std::vector<ActuatorDataStruct> DBManager::getActuatorsData(int n)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<ActuatorDataStruct> records;
    records.reserve(n);
    const char* sql = "SELECT ActuatorID, State, Timestamp FROM ActuatorEvents ORDER BY Timestamp DESC LIMIT ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return records;
    }
    sqlite3_bind_int(stmt, 1, n);
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ActuatorDataStruct record{};
        record.ActuatorID = sqlite3_column_int(stmt, 0);
        record.State = sqlite3_column_int(stmt, 1);
        record.timestamp = sqlite3_column_int64(stmt, 2);
        records.push_back(record);
    }
    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
    }
    return records;
}

bool DBManager::deleteActuator(const std::string& ActuatorName)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    int ActuatorID = getActuatorID(ActuatorName);
    if (ActuatorID == -1)
    {
        std::cerr << "Actuator '" << ActuatorName <<"' not found. Noting to delete.";
        return true;
    }

    const char* sql = "DELETE FROM Actuator WHERE ActuatorID = ? ";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, ActuatorID);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_handle) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
        return false;
    }
    return true;
}


bool DBManager::addActuator(const std::string& Actuator)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* sql = "INSERT INTO Actuator (Name) VALUES (?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, Actuator.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        if (rc == SQLITE_CONSTRAINT)
        {
            std::cerr << "Actuator: " << Actuator << " already exists." << std::endl;
        }else
        {
            std::cerr << "SQL error: " << sqlite3_errmsg(db_handle) << std::endl;
        }
        sqlite3_finalize(stmt);
        return false;
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
        return false;
    }
    return true;
}
