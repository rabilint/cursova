//
// Created by rabilint on 22.11.25.
//

#ifndef ARDUINO_ISENSOR_H
#define ARDUINO_ISENSOR_H
#include "Arduino.h"

struct sensorStruct
{
    int SensorID{};
    String SensorName{};
    double Data{};
};

constexpr int maxSensors = 20;

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool initialize() = 0;
    virtual sensorStruct read() = 0;
    virtual int getSensorID() const = 0;
    virtual String getSensorName() const = 0;
};

#endif //ARDUINO_ISENSOR_H