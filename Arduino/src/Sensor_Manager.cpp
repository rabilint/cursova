//
// Created by rabilint on 23.11.25.
//

#include "Sensor_Manager.h"
#include <Arduino.h>

SensorManager::SensorManager() : lastSensorReadTime(0), sensorsAssigned(0) {
}


void SensorManager::addSensor(const std::shared_ptr<ISensor>& sensor) {
    if (sensor && sensor->initialize()) {
        sensors[0] = sensor;
        sensorsAssigned++;
    }
}

std::array<sensorStruct, maxSensors>SensorManager::readAllSensors() {
    std::array<sensorStruct, maxSensors> data;
    updateLastReadTime();
    for (int i = 0; i < sensors.size(); i++)
    {
        const sensorStruct senorData = sensors[i]->read();
        data[i] = senorData;
    }
    return data;
}

bool SensorManager::shouldReadSensors() const {
    const unsigned long currentTime = millis();
    return (currentTime - lastSensorReadTime >= sensorReadInterval);
}

std::array<sensorStruct, maxSensors> SensorManager::getSensorInfo() const {
    std::array<sensorStruct, maxSensors> info;
    for (int i = 0; i < sensors.size(); i++)
    {
        sensorStruct senorData;
        senorData.SensorID = sensors[i]->getSensorID();
        senorData.SensorName = sensors[i]->getSensorName();
        senorData.Data = 0;
        info[i] = senorData;
    }

    return info;
}

void SensorManager::updateLastReadTime() {
    lastSensorReadTime = millis();
}
