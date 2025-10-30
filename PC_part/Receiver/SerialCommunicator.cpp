//
// Created by rabilint on 18.09.25.
//
#include "SerialCommunicator.h"
#include <iostream>
#include <thread>
#include "DatabaseManager.h"
#include <atomic>
#include <cmath>

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

void serialReaderThread(SerialCommunicator& serial, DBManager& DB)
{
    while (running)
    {
        std::string line = serial.readLine();
        if (!line.empty())
        {
            if (line == "GiveActuatorInfo\n") //Ось тут частина де відправляються дані на ардуїно
            {
                //TODO: Додати відправку sensor-ів та їхніх ID врахувати, що сенсор може називатись temperature2 etc.
                std::cout  << "In Get line section: "<< line << std::endl;


                std::vector<ActuatorStruct> actuators = DB.actuatorManager().listActuators(); //Отримуємо actuators vector.

                serial.writeLine("Take: *" + std::to_string(actuators.size()) + "*");
                //std::cout << "Take: *" << std::to_string(actuators.size()) + "*" << std::endl;
                for (const ActuatorStruct& actuator : actuators)
                {
                    serial.writeLine("Take: #"  + actuator.ActuatorName + " " + std::to_string(actuator.State) + "#");
                    //std::cout << "Take: #"  + actuator.ActuatorName + " " + std::to_string(actuator.State) + "#" << std::endl;
                }
            }else
            {

                //приймає sensor ID: Data;
                size_t sensorData_start_pos = line.find(':');
                size_t sensorData_end_pos = line.find(';');

                std::string sensorId = line.substr(0, sensorData_start_pos + 1);
                std::string sensorData = line.substr(sensorData_start_pos, sensorData_end_pos - sensorData_start_pos);
                //



                try
                {


                    const int sensorID = std::stoi(sensorId);
                    const double Data = std::stod(sensorData);



                    if (DB.sensorManager().insertData(sensorID, Data))                  {
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
    std::cout << "[Serial Thread] Exiting." << std::endl;
}