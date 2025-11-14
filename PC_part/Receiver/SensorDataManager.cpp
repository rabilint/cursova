//
// Created by rabilint on 18.10.25.
//

#include "SensorDataManager.h"
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

    const char* sql = "CREATE TABLE IF NOT EXISTS SensorsID (SensorID INTEGER PRIMARY KEY, SensorName TEXT NOT NULL )";

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




bool SensorDataManager::insertData(const int sensorID, const double Data)
{
    if (!db_handle) {
        std::cerr << "Database handle is null. Cannot insert data." << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(db_mutex);

    const char* sql = "INSERT INTO SensorData (Timestamp, SensorID, Data) VALUES (?,?,?);";
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

std::map<int, SensorsStruct> SensorDataManager::getAllSensorsFromDB() {
    std::map<int, SensorsStruct> dbSensors;
    std::lock_guard<std::mutex> lock(db_mutex);

    const char* sql = "SELECT SensorID, SensorName FROM SensorsID;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return dbSensors;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SensorsStruct sensor;
        sensor.sensorID = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name) {
            sensor.name = name;
        }
        dbSensors[sensor.sensorID] = sensor;
    }

    sqlite3_finalize(stmt);
    return dbSensors;
}

bool SensorDataManager::updateSensorName(const int sensorID, const std::string& newName) {
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* sql = "UPDATE SensorsID SET SensorName = (?) WHERE SensorID = (?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
       std::cerr << "Failed to prepare: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, sensorID);

    bool success = true;
    if (sqlite3_step(stmt) != SQLITE_DONE )
    {
        std::cout << "SQL error: " << sqlite3_errmsg(db_handle) << std::endl;
        sqlite3_finalize(stmt);
        success = false;
    }
    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cout << "Finalize failed" << sqlite3_errmsg(db_handle) << std::endl;
        success = false;
    }
    return success;
}


bool SensorDataManager::insertNewSensor(const int SensorID, const std::string& name) {
    if (!db_handle)
    {
        std::cerr << "DB handle is null" << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(db_mutex);

    const char* sql = "INSERT INTO SensorsID (SensorID, SensorName) VALUES (?,?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, SensorID);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);

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


void SensorDataManager::synchronizeSensors(const std::map<int, std::string>& arduinoSensors) {
    if (!db_handle) {
        std::cerr << "DB handle is null." << std::endl;
        return;
    }

    std::map<int, SensorsStruct> dbSensors = getAllSensorsFromDB();
    std::map<int, bool> processedDbSensors;

    for (const auto& pair : arduinoSensors) {
        int arduinoId = pair.first;
        const std::string& arduinoName = pair.second;

        auto it = dbSensors.find(arduinoId);
        if (it != dbSensors.end()) {
            SensorsStruct& dbSensor = it->second;
            processedDbSensors[dbSensor.sensorID] = true;

            if (dbSensor.name != arduinoName) {
                // Імена не збігаються
                if (!updateSensorName(arduinoId, arduinoName)) {
                    std::cerr << "[Sync] UPDATE FAILED." << std::endl;
                }
            }
        } else {
            insertNewSensor(arduinoId,arduinoName);
        }
    }

    for (const auto& pair : dbSensors) {
        if (!processedDbSensors.contains(pair.first)) {
            std::cout << "[Sync] EXTRA! Sensor ID " << pair.first << " ('" << pair.second.name
                      << "') exists in DB but not on Arduino." << std::endl;
        }
    }
}

std::vector<RecordDataStruct> SensorDataManager::getLastNReadings(const int n, const int amountOfSensors)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<RecordDataStruct> records;
    const char* sql = "SELECT Timestamp, SensorName, Data FROM SensorData JOIN SensorsID ON SensorData.SensorID = SensorsID.SensorID ORDER BY Timestamp DESC LIMIT (?) ";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return records;
    }

    sqlite3_bind_int(stmt, 1, n * amountOfSensors); 

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        RecordDataStruct record{};
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.SensorName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record.Data = sqlite3_column_double(stmt, 2);
        records.push_back(record);
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
    }
    return records;
}

std::vector<RecordDataStruct> SensorDataManager::getReadingsInTimeRange(const time_t start_from, const time_t endWhen)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<RecordDataStruct> records;
    const char* sql = "SELECT Timestamp, SensorName, Data FROM SensorData JOIN SensorsID ON SensorData.SensorID = SensorsID.SensorID WHERE Timestamp BETWEEN (?) AND (?) ORDER BY Timestamp;";
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
        RecordDataStruct record{};
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.SensorName = reinterpret_cast<const char*> (sqlite3_column_text(stmt, 1));
        record.Data = sqlite3_column_double(stmt, 2) ;
        records.push_back(record);
    }
    sqlite3_finalize(stmt);

    return records;
}

int SensorDataManager::amountOfSensors()
{
    int amountOfSensors = 0;
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* sql = "SELECT COUNT(*) FROM SensorsID";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr)!= SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return amountOfSensors;
    }

    sqlite3_bind_int(stmt, 1, amountOfSensors);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        amountOfSensors = sqlite3_column_int(stmt, 0);
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
    }
    return amountOfSensors;
}
