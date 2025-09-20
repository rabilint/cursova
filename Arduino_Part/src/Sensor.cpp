//
// Created by rabilint on 13.09.25.
//
#include <Arduino.h>
#include "Sensor.h"
#include <RTC.h>

Sensor::Sensor()
{


    lastCheckedUnixTime = 0;
    cacheData.temperature = 0.0f;
    cacheData.humidity = 50.0f;
}

SensorData Sensor::readData()
{
    RTCTime currentTime;
    RTC.getTime(currentTime);
    time_t currentTimeUnix = currentTime.getUnixTime();
    if ((currentTimeUnix - lastCheckedUnixTime) < 600)
    {
        return cacheData;
    } else {
        generateNewData(currentTime.getHour());
        lastCheckedUnixTime = currentTimeUnix;
        return cacheData;
    }
}
void Sensor::generateNewData(int currentHour)
{
    if (random(0,4) == 0)
    {
        float change = (random(0,2) == 0) ? -1.0f : 1.0f;
        cacheData.temperature = change + static_cast<float>(currentHour);
    }
    if (random(0,4) == 0)
    {
        if (cacheData.temperature > 100)
        {
            float change = (random(0,2) == 0) ? -10.0f : -20.0f;
            cacheData.humidity += change;
        }else
        {
            float change = (random(0,2) == 0) ? -10.0f : 10.0f;
            cacheData.humidity += change;
        }
    }
}


