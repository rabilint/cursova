//
// Created by rabilint on 12.11.25.
//

#include "ProtocolHandler.h"
#include <iostream>
#include <chrono>
#include <thread>

ProtocolHandler::ProtocolHandler(const std::shared_ptr<ISerialPort>& serial,
                    const std::shared_ptr<SensorService>& sensorSvc,
                    const std::shared_ptr<ActuatorService>& actuatorSvc) :
    serialPort(serial),
    sensorService(sensorSvc),
    actuatorService(actuatorSvc),
    currentHandshakeState(HandshakeState::WAITING_FOR_SIZE),
    numSensors(0),
    sensorsReceived(0)
{
}



bool ProtocolHandler::parseSensorSize(const std::string& line, int& size)
{
    size_t startPos = line.find_first_of('*');
    size_t endPos = line.find_first_of('*');
    if (startPos != std::string::npos && endPos > startPos)
    {
        std::string sizeStr = line.substr(startPos + 1 , endPos - startPos - 1);
        try
        {
            size = std::stoi(sizeStr);
            return true;
        }catch (const std::exception&)
        {
            return false;
        }
    }
    return false;
}

bool ProtocolHandler::parseSensorData(const std::string& line, int& sensorID, std::string& sensorName)
{
    size_t startPos = line.find_first_of('#');
    size_t endPos = line.find_first_of('#');
    if (startPos != std::string::npos && endPos > startPos)
    {
        std::string data = line.substr(startPos + 1 , endPos - startPos - 1);
        int spaceIndex = static_cast<int>(data.find(' '));

        if (spaceIndex != std::string::npos)
        {
            sensorID = std::stoi(data.substr(0, spaceIndex));
            sensorName = data.substr(spaceIndex + 1, endPos - spaceIndex - 1);
            return true;
        }else
        {
            return false;
        }
    }
    return false;
}

bool ProtocolHandler::parseSensorReading(const std::string& line, int& sensorID, double& reading)
{
    if (line.find("ID:#") == std::string::npos)
    {
        return false;
    }
    size_t sensorID_start_pos = line.find_first_of('#');
    size_t sensorID_end_pos = line.find('|');
    size_t sensorData_end_pos = line.find_last_of('#');

    if (sensorID_start_pos != std::string::npos &&
        sensorID_end_pos != std::string::npos &&
        sensorData_end_pos != std::string::npos)
    {
        try
        {
            std::string sensorIDStr = line.substr(sensorID_start_pos + 1, sensorID_end_pos - 1);
            std::string sensorDataStr = line.substr(sensorID_end_pos + 1, sensorData_end_pos - 1);

            sensorID = std::stoi(sensorIDStr);
            reading = std::stod(sensorDataStr);
            return true;
        }catch (const std::exception&)
        {
            return false;
        }
    }
    return false;
}

void ProtocolHandler::handleActuatorRequest() const
{
    std::cout << "[TECH LOG] Arduino sent request for Actuators Synchronization" << std::endl;

    const auto actuators = actuatorService->getActuatorsForHandshake();

    serialPort->writeLine("Take: #" + std::to_string(actuators.size()));

    for (const auto& actuator : actuators)
    {
        serialPort->writeLine("Take: #" + actuator.ActuatorName + " " + std::to_string(actuator.State) + "#");
    }
}

void ProtocolHandler::handleSensorHandshake(const std::string& line)
{
    if (currentHandshakeState == HandshakeState::WAITING_FOR_SIZE)
    {
        int size = 0;
        if (parseSensorSize(line, size))
        {
            numSensors = size;
            if (numSensors > 0)
            {
                arduinoSensors.clear();
                sensorsReceived = 0;
                currentHandshakeState = HandshakeState::WAITING_FOR_DATA;
            } else
            {
                currentHandshakeState = HandshakeState::COMPLETE;
            }
        }
    }

    else if (currentHandshakeState == HandshakeState::WAITING_FOR_DATA)
    {
        int sensorID = 0;
        std::string sensorName;
        if (parseSensorData(line, sensorID, sensorName))
        {
            arduinoSensors[sensorID] = sensorName;
            sensorsReceived++;

            if (sensorsReceived >= numSensors)
            {
                currentHandshakeState = HandshakeState::COMPARING;
            }
        }
    }

    else if (currentHandshakeState == HandshakeState::COMPARING)
    {
        sensorService->synchronizeSensors(arduinoSensors);
        currentHandshakeState = HandshakeState::COMPLETE;
        std::cout << ("[TECH LOG] Sensors Synchronized, server ready to work") << std::endl;
        serialPort->writeLine("SENSORS_SYNCHRONIZED");
    }
}

void ProtocolHandler::handleSensorReading(const std::string& line) const
{
    int sensorID = 0;
    double Data = 0.0;
    if (parseSensorReading(line,sensorID, Data ))
    {
        if (sensorService)
        {
            if (sensorService->insertData(sensorID, Data)) {}
            else
            {
                std::cerr << "[ERR] Sensor data insert error" << std::endl;
            }
        }
        else
        {
            std::cerr << "[ERR] Sensor reading error" << std::endl;
        }
    }
}

void ProtocolHandler::processLine(const std::string& line)
{
    if (line.empty()) return;

    if (line == "GiveActuatorInfo\n" || line == "GiveActuatorInfo")
    {
        handleActuatorRequest();
    }

    else if (line == "#ERROR:UNSUPPORTED_ACTUATOR!")
    {
        std::cout << "Unable to make action!" << std::endl;
        std::cout << "Check if you've add correct actuator" << std::endl;
        std::cout << "or does your model of product support this actuator" << std::endl;
    }
    else if (currentHandshakeState != HandshakeState::COMPLETE)
    {
        if (line.find("Take: ") != std::string::npos)
        {
            handleSensorHandshake(line);
        }
        return;
    }
    else
    {
        if (line.find("ID:") != std::string::npos)
        {
            handleSensorReading(line);
        }
    }
}

bool ProtocolHandler::isHandshakeComplete() const
{
    return currentHandshakeState == HandshakeState::COMPLETE;
}

void ProtocolHandler::requestSensorSync() const
{
    if (serialPort && serialPort->isConnected() && currentHandshakeState != HandshakeState::COMPLETE) {
        static auto lastRequestTime = std::chrono::steady_clock::now();
        const auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastRequestTime).count();

        // Request sync every 2 seconds if not complete
        if (elapsed >= 2) {
            serialPort->writeLine("RECOMMIT_SYNC");
            serialPort->writeLine("GIVE_SENSORS");
            lastRequestTime = currentTime;
        }
    }
}


