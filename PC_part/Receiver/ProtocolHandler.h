//
// Created by rabilint on 12.11.25.
//

#ifndef RECEIVER_PROTOCOLHANDLER_H
#define RECEIVER_PROTOCOLHANDLER_H

#include "ISerialPort.h"
#include "SensorService.h"
#include "ActuatorService.h"
#include <memory>
#include <string>
#include <map>
#include <atomic>

enum class HandshakeState {
    WAITING_FOR_SIZE,
    WAITING_FOR_DATA,
    COMPARING,
    COMPLETE
};

class ProtocolHandler {
private:
    std::shared_ptr<ISerialPort> serialPort;
    std::shared_ptr<SensorService> sensorService;
    std::shared_ptr<ActuatorService> actuatorService;

    HandshakeState currentHandshakeState;
    std::map<int, std::string> arduinoSensors;
    int numSensors;
    int sensorsReceived;

    static bool parseSensorSize(const std::string& line, int& size);
    static bool parseSensorData(const std::string& line, int& sensorID, std::string& sensorName);
    static bool parseSensorReading(const std::string& line, int& sensorID, double& data);
    void handleActuatorRequest() const;
    void handleSensorHandshake(const std::string& line);
    void handleSensorReading(const std::string& line) const;

public:
    ProtocolHandler(const std::shared_ptr<ISerialPort>& serial,
                    const std::shared_ptr<SensorService>& sensorSvc,
                    const std::shared_ptr<ActuatorService>& actuatorSvc);
    void processLine(const std::string& line);
    [[nodiscard]] bool isHandshakeComplete() const;
    void requestSensorSync() const;
};


#endif //RECEIVER_PROTOCOLHANDLER_H