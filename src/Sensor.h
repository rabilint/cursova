//
// Created by rabilint on 13.09.25.
//

#ifndef UNTITLED3_SENSOR_H
#define UNTITLED3_SENSOR_H
#include <RTC.h>


struct SensorData
{
    float temperature;
    float humidity;
};

class Sensor
{
    public:
    Sensor();
    SensorData readData();
    ~Sensor() = default;
private:
    time_t lastCheckedUnixTime;
    SensorData cacheData{};

    void generateNewData();
};



#endif //UNTITLED3_SENSOR_H