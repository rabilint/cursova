//
// Created by rabilint on 23.11.25.
//

#include "BME680_Sensor.h"

BME680Manager::BME680Manager()
    : bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK), initialized(false),
      lastTemperature(0), lastHumidity(0), lastPressure(0), lastGas(0) {
}

BME680_Sensor::BME680_Sensor(BME680Manager* bmeManager, int id, const char* name, BME680DataType type) :
    manager(bmeManager),sensorID(id),sensorName(name),dataType(type)
{}


bool BME680_Sensor::initialize()
{
    return manager != nullptr && manager->initialize();
}

bool BME680Manager::initialize()
{
    if (initialized) {return true;}

    if (!bme.begin())
    {
        for (;;);
    }

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);
    bme.readAltitude(SEALEVELPRESSURE_HPA);

    initialized = true;
    return true;
}


bool BME680Manager::performReading() {
    if (!initialized) {
        return false;
    }
    bme.readAltitude(SEALEVELPRESSURE_HPA);
    lastTemperature = bme.readTemperature();
    lastHumidity = bme.readHumidity();
    lastPressure = bme.readPressure();
    lastGas = bme.readGas();
    return true;
}


float BME680Manager::getTemperature() const
{
    return lastTemperature;
}

float BME680Manager::getHumidity() const
{
    return lastHumidity;
}

uint32_t BME680Manager::getPressure() const
{
    return lastPressure;
}

uint32_t BME680Manager::getGas() const
{
    return lastGas;
}


sensorStruct BME680_Sensor::read() {
    sensorStruct data;
    data.SensorID = sensorID;
    data.SensorName = sensorName;

    if (manager) {

        switch (dataType) {
        case BME680DataType::TEMPERATURE:
            data.Data = manager->getTemperature();
            break;
        case BME680DataType::HUMIDITY:
            data.Data = manager->getHumidity();
            break;
        case BME680DataType::PRESSURE:
            data.Data = manager->getPressure() / 100.0;
            break;
        case BME680DataType::GAS:
            data.Data = manager->getGas() / 1000.0;
            break;
        }
    }

    return data;
}

int BME680_Sensor::getSensorID() const {
    return sensorID;
}

String BME680_Sensor::getSensorName() const {
    return sensorName;
}



