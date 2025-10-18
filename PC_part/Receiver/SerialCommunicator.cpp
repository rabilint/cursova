//
// Created by rabilint on 18.09.25.
//
#include "SerialCommunicator.h"
#include <iostream>
#include <thread>
#include "DatabaseManager.h"
#include <atomic>

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
            if (line == "GiveActuatorInfo") //Ось тут частина де відправляються дані на ардуїно
            {

                std::vector<ActuatorStruct> actuators;
                actuators = DB.actuatorManager().listActuators(); //Отримуємо vector.
                serial.writeLine("Take: *" + std::to_string(actuators.size()) + "*");
                for (const ActuatorStruct& actuator : actuators)
                {
                    serial.writeLine("Take: #"  + actuator.ActuatorName + " " + std::to_string(actuator.State) + "#");
                }
            }else
            {
                size_t temp_start_pos = line.find("T:") + 2;
                size_t temp_end_pos = line.find(';');
                std::string temp = line.substr(temp_start_pos, temp_end_pos - temp_start_pos);

                size_t hum_start_pos = line.find("H:") + 2;
                size_t hum_end_pos = line.find(';');
                std::string hum = line.substr(hum_start_pos, hum_end_pos - temp_start_pos);


                try
                {
                    float temperature = std::stof(temp);
                    float humidity = std::stof(hum);


                    if (DB.sensorManager().insertData(temperature, humidity))
                    {
                        // std::cout<< "success" << std::endl;
                    }else
                    {
                        std::cout<< "failed to insert data" << std::endl;
                    }
                }catch (const std::exception& e)
                {
                    std::cerr << "Error in inserting data: " << e.what() << std::endl;
                }
            }
        }
    }
    std::cout << "[Serial Thread] Exiting." << std::endl;
}