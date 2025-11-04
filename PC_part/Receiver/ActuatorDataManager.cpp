//
// Created by rabilint on 18.10.25.
//

#include "ActuatorDataManager.h"

ActuatorDataManager::ActuatorDataManager(const std::string& filename)
{
    if (sqlite3_open(filename.c_str(), &db_handle))
    {
        std::cerr << "Can't open database " << filename << std::endl;
        return;
    }
    createActuatorTables();
    createActuatorsLib();
}

ActuatorDataManager::~ActuatorDataManager()
{
    if (db_handle)
    {
        sqlite3_close(db_handle);
    }
}

void ActuatorDataManager::createActuatorsLib()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* sql = "CREATE TABLE IF NOT EXISTS Actuator (ActuatorID INTEGER PRIMARY KEY AUTOINCREMENT, Name TEXT NOT NULL, State INTEGER NULL DEFAULT 0 )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_handle, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

void ActuatorDataManager::createActuatorTables()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* sql = "CREATE TABLE IF NOT EXISTS ActuatorEvents (EventID INTEGER PRIMARY KEY AUTOINCREMENT, ActuatorID INTEGER NOT NULL, State INTEGER NOT NULL, Timestamp INTEGER NOT NULL, FOREIGN KEY (ActuatorID) REFERENCES Actuators(ActuatorID))";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_handle, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

int ActuatorDataManager::getActuatorID(const std::string& Actuator) // Technical func
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

bool ActuatorDataManager::addEvent(const std::string& Actuator, const int& state )
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

bool ActuatorDataManager::deleteActuator(const std::string& ActuatorName)
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


bool ActuatorDataManager::addActuator(const std::string& Actuator)
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

std::vector<ActuatorStruct> ActuatorDataManager::listActuators()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt = nullptr;

    const char* sql = "SELECT * FROM Actuator ";
    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return std::vector<ActuatorStruct>{};
    }
    std::vector<ActuatorStruct> ActuatorList{};
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ActuatorStruct Actuator;
        Actuator.ActuatorID = sqlite3_column_int(stmt, 0); //id

        //name
        const char* name_ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        Actuator.ActuatorName = name_ptr ? std::string(name_ptr) : std::string{};

        //state
        Actuator.State = sqlite3_column_int(stmt, 2);

        ActuatorList.push_back(Actuator);
    }
    return ActuatorList;
}

bool ActuatorDataManager::updateActuatorState(const int& ActuatorID, const int& state)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE Actuator SET State = ? WHERE ActuatorID = ?";
    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, state);
    sqlite3_bind_int(stmt, 2, ActuatorID);
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

std::string ActuatorDataManager::getActuatorName(const int& ActuatorID) const
{
    // std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt = nullptr;
    std::string ActuatorName = "";
    const char* sql = "SELECT Name FROM Actuator WHERE ActuatorID = ?";

    if (sqlite3_prepare_v2(db_handle, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db_handle) << std::endl;
        return std::string{};
    }
    sqlite3_bind_int(stmt, 1, ActuatorID);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ActuatorName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
        return std::string{};
    }
    return ActuatorName;
}

std::vector<ActuatorDataStruct> ActuatorDataManager::getActuatorsData(int n)
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
        record.ActuatorName = getActuatorName(record.ActuatorID);
        records.push_back(record);
    }
    if (sqlite3_finalize(stmt) != SQLITE_OK)
    {
        std::cerr << "Finalize failed" << std::endl;
    }
    return records;
}