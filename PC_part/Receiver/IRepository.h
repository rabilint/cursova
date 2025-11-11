//
// Created by rabilint on 11.11.25.
//

#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <string>
#include <vector>
#include <map>

struct RecordDataStruct {
    time_t timestamp{};
    std::string SensorName{};
    double Data{};
};

struct ActuatorStruct {
    int ActuatorID;
    std::string ActuatorName;
    int State;
};

struct ActuatorDataStruct {
    int ActuatorID;
    int State;
    std::string ActuatorName;
    time_t timestamp;
};

class ISensorRepository {
public:
    virtual ~ISensorRepository() = default;
    virtual bool insertData(int sensorID, double data) = 0;
    virtual std::vector<RecordDataStruct> getLastNReadings(int n) = 0;
    virtual std::vector<RecordDataStruct> getReadingsInTimeRange(time_t startFrom, time_t endWhen) = 0;
    virtual void synchronizeSensors(const std::map<int, std::string>& arduinoSensors) = 0;
};

class IActuatorRepository {
public:
    virtual ~IActuatorRepository() = default;
    virtual bool addEvent(const std::string& actuator, const int& state) = 0;
    virtual int getActuatorID(const std::string& actuator) = 0;
    virtual std::vector<ActuatorDataStruct> getActuatorsData(int n) = 0;
    virtual bool deleteActuator(const std::string& actuatorName) = 0;
    virtual bool addActuator(const std::string& actuator) = 0;
    virtual std::vector<ActuatorStruct> listActuators() = 0;
    virtual std::string getActuatorName(const int& actuatorID) const = 0;
    virtual bool updateActuatorState(const int& actuatorID, const int& state) = 0;
};

#endif // IREPOSITORY_H

