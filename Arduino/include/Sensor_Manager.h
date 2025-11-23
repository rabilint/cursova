//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_SENSOR_MANAGER_H
#define ARDUINO_SENSOR_MANAGER_H

#include <array>
#include "ISensor.h"
#include <memory>

class SensorManager  {
private:
    std::array<std::shared_ptr<ISensor>, maxSensors> sensors;
    unsigned long lastSensorReadTime;
    int sensorsAssigned;
    constexpr static long sensorReadInterval = 150000;

public:
    SensorManager();
    void addSensor(const std::shared_ptr<ISensor>& sensor);
    std::array<sensorStruct, maxSensors> readAllSensors();
    bool shouldReadSensors() const;
    std::array<sensorStruct, maxSensors> getSensorInfo() const;
    void updateLastReadTime();
};

#endif //ARDUINO_SENSOR_MANAGER_H