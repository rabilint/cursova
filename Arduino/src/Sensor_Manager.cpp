//
// Created by rabilint on 23.11.25.
//

#include "Sensor_Manager.h"
#include <Arduino.h>

SensorManager::SensorManager() : lastSensorReadTime(0), sensorsAssigned(0) {
}


void SensorManager::addSensor(const std::shared_ptr<ISensor>& sensor)
{
    if (sensorsAssigned >= sensors.size()) {
        Serial.print(F("[ERROR] SensorManager is full!"));
        return;
    }

    if (sensor && sensor->initialize()) {
        sensors[sensorsAssigned] = sensor;
        sensorsAssigned++;
    } else {
        Serial.println(F("[ERROR] Failed to init sensor"));
    }
}

std::array<sensorStruct, maxSensors>SensorManager::readAllSensors() {
    std::array<sensorStruct, maxSensors> data;
    updateLastReadTime();

    for (int i = 0; i < sensors.size(); i++)
    {
        if (sensors[i])
        {
            data[i] = sensors[i]->read();
        }else
        {
            data[i] = sensorStruct{};
        }
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
        if (sensors[i] != nullptr)
        {
            sensorStruct senorData;
            senorData.SensorID = sensors[i]->getSensorID();
            senorData.SensorName = sensors[i]->getSensorName();
            senorData.Data = 0;
            info[i] = senorData;
        }else
        {
            info[i].SensorID = -1;
            info[i].SensorName = "EMPTY";
        }
    }
    return info;
}

void SensorManager::updateLastReadTime() {
    lastSensorReadTime = millis();
}
