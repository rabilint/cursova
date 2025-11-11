//
// Created by rabilint on 18.09.25.
//
#include "SerialCommunicator.h"
#include "ActuatorDataManager.h"
#include "SensorDataManager.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <map>
#include <cmath>

enum class HandshakeState{WAITING_FOR_SIZE, WAITING_FOR_DATA, COMPARING, COMPLETE};



SerialCommunicator::SerialCommunicator(const std::string& port, uint32_t baudrate) {
    try {
        serial::Timeout timeout = serial::Timeout::simpleTimeout(1000);
        serial_port.setPort(port);
        serial_port.setBaudrate(baudrate);
        serial_port.setTimeout(timeout);

        serial_port.open();
    } catch (const serial::IOException& e) {
        std::cerr << "Error: Unable to open serial port " << port << ". " << e.what() << std::endl;
    }
}

SerialCommunicator::~SerialCommunicator() {
    if (serial_port.isOpen()) {
        serial_port.close();
    }
}

bool SerialCommunicator::isConnected() const {
    return serial_port.isOpen();
}

std::string SerialCommunicator::readLine() {
    if (!isConnected()) return "";
    return serial_port.readline();
}

void SerialCommunicator::writeLine(const std::string& line) {
    if (!isConnected()) return;
    serial_port.write(line + "\n");
}

std::atomic<bool> running = true;


//TODO: REMOVE PART BELOW
void serialReaderThread(SerialCommunicator& serial,SensorDataManager& SensorDBM, ActuatorDataManager& ActuatorDBM)
{
    HandshakeState currentHandshakeState = HandshakeState::WAITING_FOR_SIZE;
    std::map<int, std::string> arduinoSensors;
    extern int sensorsReceived;
    int numSensors = 0;
    serial.writeLine("RECOMMIT_SYNC");
    while (running)
    {
        if (currentHandshakeState != HandshakeState::COMPLETE)
        {
            serial.writeLine("RECOMMIT_SYNC");
            serial.writeLine("GIVE_SENSORS");
        }
        std::string line = serial.readLine();
        if (!line.empty())
        {
            if (line == "GiveActuatorInfo\n") //Ось тут частина де відправляються дані на ардуїно
            {
                std::cout << "Arduino send request for Synchronization" << std::endl;

                std::vector<ActuatorStruct> actuators = ActuatorDBM.listActuators(); //Отримуємо actuators vector.

                serial.writeLine("Take: *" + std::to_string(actuators.size()) + "*");
                //std::cout << "Take: *" << std::to_string(actuators.size()) + "*" << std::endl;
                for (const ActuatorStruct& actuator : actuators)
                {
                    serial.writeLine("Take: #"  + actuator.ActuatorName + " " + std::to_string(actuator.State) + "#");
                    //std::cout << "Take: #"  + actuator.ActuatorName + " " + std::to_string(actuator.State) + "#" << std::endl;
                }
            }

            else if (line == "#ERROR:UNSUPPORTED_ACTUATOR!")
            {
                std::cout << "Unable to make action!" << std::endl;
                std::cout << "Check if you've add correct actuator" << std::endl;
                std::cout << "or does your model of product support this actuator" << std::endl;
            }

            //Відправка на сервер списку сенсорів.


            else if (currentHandshakeState != HandshakeState::COMPLETE)
            {
                if (!line.starts_with("Take: ")) continue;

                if (currentHandshakeState == HandshakeState::WAITING_FOR_SIZE)
                {
                    const size_t startPos = line.find_first_of('*');
                    const size_t endPos = line.find_last_of('*');

                    if (startPos!= -1 && endPos > startPos)
                    {
                        std::string size = line.substr(startPos + 1 , endPos - startPos - 1);
                        numSensors = std::stoi(size);
                        if (numSensors > 0)
                        {
                            currentHandshakeState = HandshakeState::WAITING_FOR_DATA;
                        }else
                        {
                            currentHandshakeState = HandshakeState::COMPLETE;
                        }
                    }
                }
                else if (currentHandshakeState == HandshakeState::WAITING_FOR_DATA)
                {
                    size_t startPos = line.find_first_of('#');
                    size_t endPos = line.find_last_of('#');
                    if (startPos != -1 && endPos > startPos)
                    {
                        std::string data = line.substr(startPos + 1 , endPos - startPos - 1 );
                        int spaceIndex = data.find(' ');

                        if (spaceIndex != std::string::npos)
                        {
                            int sensorID = std::stoi(data.substr(0, spaceIndex));
                            std::string sensorName = data.substr(spaceIndex + 1, endPos - spaceIndex - 1);

                            arduinoSensors.insert({sensorID, sensorName});
                            sensorsReceived++;
                        }
                        if (sensorsReceived >= numSensors) {
                            currentHandshakeState = HandshakeState::COMPARING;
                        }
                    }
                }else if (currentHandshakeState == HandshakeState::COMPARING)
                {
                    SensorDBM.synchronizeSensors(arduinoSensors);
                    currentHandshakeState = HandshakeState::COMPLETE;
                    serial.writeLine("SENSORS_SYNCHRONIZED");
                }


            }else
            {
                //приймає sensor ID: Data;
                if (line.starts_with("ID:")){
                    size_t sensorID_start_pos = line.find('#');
                    size_t sensorID_end_pos = line.find('|');
                    size_t sensorData_end_pos = line.find_last_of('#');

                    std::string sensorId = line.substr(sensorID_start_pos + 1, sensorID_end_pos - 1);
                    std::string sensorData = line.substr(sensorID_end_pos + 1, sensorData_end_pos - 1);
                    //



                    try
                    {
                        const int sensorIdToInsert = std::stoi(sensorId);
                        const double DataToInsert = std::stod(sensorData);

                        if (SensorDBM.insertData(sensorIdToInsert, DataToInsert))                  {
                            // std::cout<< "success" << std::endl;
                        }else
                        {
                            std::cout<< "failed to insert data" << std::endl;
                        }
                    }catch (const std::exception& e)
                    {
                        std::cerr << sensorId << "; " << sensorData << std::endl;
                        std::cerr << line << std::endl;

                        std::cerr << "Error in inserting data: " << e.what() << std::endl;
                    }
                }
            }
        }
    }
    std::cout << "[Serial Thread] Exiting." << std::endl;
}