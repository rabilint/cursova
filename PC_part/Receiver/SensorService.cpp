//
// Created by rabilint on 11.11.25.
// Бізнес-логіка сенсорів
//
#include "SensorService.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <utility>

SensorService::SensorService(std::shared_ptr<ISensorRepository> repository): repository(std::move(repository))
{}

bool SensorService::insertData(const int sensorID, const double data) const
{
    return repository->insertData(sensorID, data);
}

std::vector<RecordDataStruct> SensorService::getLastNReadings(const int n) const
{
    return repository -> getLastNReadings(n, repository->amountOfSensors());
}

std::vector<RecordDataStruct> SensorService::getReadingsInTimeRange(const time_t start_from, const time_t end_when) const
{
    return repository -> getReadingsInTimeRange(start_from, end_when);
}

void SensorService::synchronizeSensors(const std::map<int, std::string>& arduinoSensors) const
{
    repository -> synchronizeSensors(arduinoSensors);
}

void SensorService::displayLastNReadings(const int n) const
{
    const auto records = getLastNReadings(n);
    std::cout << "All data: " << std::endl;

    for (size_t i =0; i < records.size(); i++)
    {
        tm* gmt_time_info = gmtime(&records[i].timestamp);
        char TimeBuffer[80];
        strftime(TimeBuffer, sizeof(TimeBuffer), "%Y-%m-%d %H:%M:%S UTC", gmt_time_info);

        if (i > 0 && records[i].timestamp == records[i-1].timestamp)
        {
            std::cout << " | " << records[i].SensorName << " : " << records[i].Data;
        }else
        {
            std::cout<< std::endl;
            std::cout << TimeBuffer << " | : " <<  records[i].SensorName << " : "<< records[i].Data ;
        }
    }
    if (!records.empty()) std::cout << std::endl;
}

void SensorService::displayReadingsInTimeRange(const time_t start_from, const time_t end_when) const
{
    const auto records = getReadingsInTimeRange( start_from, end_when);
    if (records.empty())
    {
        std::cerr << "No reading found" << std::endl;
        std::cerr << "Make sure you write diapason correctly" << std::endl;
        return;
    }

    for (int i = 0; i < records.size(); i++)
    {
        const struct ::tm* gmt_time_info = gmtime(&records[0].timestamp);
        char TimeBuffer[80];
        strftime(TimeBuffer, sizeof(TimeBuffer), "%Y-%m-%d %H:%M:%S UTC", gmt_time_info);
        if (i > 0 && records[i].timestamp == records[i-1].timestamp)
        {
            std::cout << " | " << records[i].SensorName << " : " << records[i].Data;
        }else
        {
            std::cout<< std::endl;
            std::cout << TimeBuffer << " | : " <<  records[i].SensorName << " : "<< records[i].Data ;
        }

    }
    if (!records.empty()) std::cout << std::endl;
}




