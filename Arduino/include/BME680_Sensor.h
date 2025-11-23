//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_BME680_SENSOR_H
#define ARDUINO_BME680_SENSOR_H

#define SEALEVELPRESSURE_HPA (1002)
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#include "ISensor.h"
#include <Arduino.h>
#include "/home/rabilint/CLionProjects/cursova/Arduino/lib/Adafruit_BME680-master/Adafruit_BME680.h"
#include "Serial_Handler.h"


enum class BME680DataType {
    TEMPERATURE,
    HUMIDITY,
    PRESSURE,
    GAS
};



class BME680Manager
{
private:
    Adafruit_BME680 bme;
    bool initialized;
    float lastTemperature;
    float lastHumidity;
    uint32_t lastPressure;
    uint32_t lastGas;

public:
    BME680Manager();
    bool initialize();
    bool performReading();
    float getTemperature() const;
    float getHumidity() const;
    uint32_t getPressure() const;
    uint32_t getGas() const;
};




class BME680_Sensor final : public ISensor {
private:
    BME680Manager* manager;
    int sensorID;
    String sensorName;
    BME680DataType dataType;

public:
    BME680_Sensor(BME680Manager* bmeManager, int id, const char* name, BME680DataType type);
    bool initialize() override;
    sensorStruct read() override;
    int getSensorID() const override;
    String getSensorName() const override;
};



#endif //ARDUINO_BME680_SENSOR_H