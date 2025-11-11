//
// Created by rabilint on 11.11.25.
//

#ifndef RECEIVER_SENSORSERVICE_H
#define RECEIVER_SENSORSERVICE_H
#include <memory>
#include "IRepository.h"
#include <vector>
#include <map>


class SensorService
{
    private:
    std::shared_ptr<ISensorRepository> repository;

    public:
    explicit SensorService(std::shared_ptr<ISensorRepository> repository);
    bool insertData(int sensorID, double data) const;
    std::vector<RecordDataStruct> getLastNReadings(int n) const;
    std::vector<RecordDataStruct> getReadingsInTimeRange(time_t start_from, time_t end_when) const;
    void synchronizeSensors(const std::map<int, std::string>& arduinoSensors) const;
    void displayLastNReadings(int n) const;
    void displayReadingsInTimeRange(time_t start_from, time_t end_when) const;

};


#endif //RECEIVER_SENSORSERVICE_H