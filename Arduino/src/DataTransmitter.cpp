//
// Created by rabilint on 23.11.25.
//

#include "DataTransmitter.h"

DataTransmitter::DataTransmitter(Serial_Handler* handler) : serialHandler(handler)
{}

void DataTransmitter::transmitSensorReadings(const std::array<sensorStruct, maxSensors>& sensorsData) const
{
    if (!serialHandler) return;

    for (const auto& sensor : sensorsData) {
        String message = "ID:#";
        message += String(sensor.SensorID);
        message += "|";
        message += String(sensor.Data);
        message += "#";
        serialHandler->sendLine(message);
    }
}

void DataTransmitter::transmitSensorData(const std::array<sensorStruct, maxSensors>& sensors) const
{
    if (!serialHandler) return;

    int realCount = 0;
    for (const auto& sensor : sensors) {
        if (sensor.SensorID != -1 && sensor.SensorID != 0) {
            realCount++;
        }
    }

    String countMessage = "Take: *";
    countMessage += String(realCount);
    countMessage += "*";
    serialHandler->sendLine(countMessage);

    // Send each sensor info
    for (const auto& sensor : sensors) {
        if (sensor.SensorID == -1 || sensor.SensorID == 0) continue;

        String message = "Take: #";
        message += String(sensor.SensorID);
        message += " ";
        message += sensor.SensorName;
        message += "#";
        serialHandler->sendLine(message);
    }
}


