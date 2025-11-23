//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_DATATRANSMITTER_H
#define ARDUINO_DATATRANSMITTER_H

#include "Serial_Handler.h"
#include "ISensor.h"
class DataTransmitter
{
private:
    Serial_Handler* serialHandler;

public:
    explicit DataTransmitter(Serial_Handler* handler);
    void transmitSensorData(const std::array<sensorStruct, maxSensors>& sensors) const;
    void transmitSensorReadings(const std::array<sensorStruct, maxSensors>& sensorsData) const;
};



#endif //ARDUINO_DATATRANSMITTER_H