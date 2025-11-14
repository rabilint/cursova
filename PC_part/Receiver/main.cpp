#include <algorithm>
#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <cstdio>
#include "SerialCommunicator.h"
#include "ProtocolHandler.h"
#include <memory>
#include  "CommandHandler.h"
#include "SensorDataManager.h"
#include "ActuatorDataManager.h"
#include "vendor/serial/include/serial/serial.h"



SerialCommunicator my_cerial("/dev/ttyACM0",9600);


//sensors config

//

std::vector<RecordDataStruct> records;

std::atomic<bool> running = true;

void serialReaderThread(const std::shared_ptr<SerialCommunicator>& my_serial,
                const std::shared_ptr<ProtocolHandler>& protocol)
{
    protocol->requestSensorSync();
    while (running)
    {
        std::string line = my_serial->readLine();
        if (!line.empty())
        {
            protocol->processLine(line);
        }

        if (!protocol->isHandshakeComplete())
        {
            protocol->requestSensorSync();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[TECH LOG] SERIAL READER THREAD TERMINATE" << std::endl;
}

int main()
{

    auto my_serial = std::make_shared<SerialCommunicator>("/dev/ttyACM0",9600);

    if (!my_serial->isConnected())
    {
        std::cout << "[TECH LOG] SERIAL CONNECT FAILED" << std::endl;
        return -1;
    }

    auto sensorRepository = std::make_shared<SensorDataManager>("../SensorData.db");
    auto actuatorRepository = std::make_shared<ActuatorDataManager>("../ActuatorEvents.db");

    auto sensorService = std::make_shared<SensorService>(sensorRepository);
    auto actuatorService = std::make_shared<ActuatorService>(actuatorRepository, my_serial);

    auto protocol_handler = std::make_shared<ProtocolHandler>(my_serial, sensorService, actuatorService);

    CommandHandler command_handler(sensorService, actuatorService);

    std::thread readerTread(serialReaderThread, my_serial , protocol_handler );
    std::cout << "Serial reader thread started." << std::endl;
    std::cout << "Enter the commands" << std::endl;
    std::string command;
    command_handler.displayHelp();

    while(running)
    {
        std::cout << "> ";
        std::getline(std::cin, command);
        if (!command_handler.executeCommand(command))
        {
            running = false;
        }
    }

    readerTread.join();
    std::cout << "Program terminated." << std::endl;
    return 0;
};