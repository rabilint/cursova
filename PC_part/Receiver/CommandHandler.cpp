//
// Created by rabilint on 11.11.25.
//

#include "CommandHandler.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <ctime>
#include "ProtocolHandler.h"

time_t parseUserInputTime(const char* buffer) {
    struct tm tm = {};
    int day = 0, month = 0, year = 0, hour = 0, minute = 0;

    if (int items_parsed = sscanf(buffer, "%d.%d.%d|%d:%d", &day, &month, &year, &hour, &minute); items_parsed != 5) {
        return -1;
    }

    tm.tm_year   = year - 1900;
    tm.tm_mon    = month - 1;
    tm.tm_mday   = day;
    tm.tm_hour   = hour;
    tm.tm_min    = minute;
    tm.tm_sec    = 0;

    return mktime(&tm);
}

CommandHandler::CommandHandler(const std::shared_ptr<SensorService>& sensorSvc, const std::shared_ptr<ActuatorService>& actuatorSvc)
    : sensorService(sensorSvc), actuatorService(actuatorSvc)
{
    commandDescriptions =  {
        {"History", "display records from and to custom time."},
        {"Check_sensors_records", "display last records."},
        {"Exit","Close Program."},
        {"Add_actuator","Add new actuator."},
        {"Delete_actuator","Delete actuator."},
        {"Check_last_actuator_events","Check actuator events history."},
        {"Make_action","Managing actuator"},
    };
}

bool CommandHandler::executeCommand(const std::string& command)
{
    std::string upperCommand = command;
    std::ranges::transform(upperCommand, upperCommand.begin(), ::toupper);
    if (upperCommand == "EXIT")
    {
        return false;
    }

    else if (upperCommand == "MAKE_ACTION")
    {
        if (!actuatorService)
        {
            std::cerr<<"ERROR: actuatorService is not available"<<std::endl;
            return true;
        }

        int input_index = -1;

        actuatorService->displayActuators();
        std::cout << "Select actuator by writing it's ID" << std::endl;
        std::cin >> input_index ;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (std::cin.fail() || input_index <=0)
        {
            std::cerr << "ERR: Invalid ID entered." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }else
        {
            std::cout << "You want to ON/OFF ?" << std::endl;
            std::string action;
            std::getline(std::cin, action);
            if (!actuatorService->controlActuator(input_index, action))
            {
                std::cerr << "ERR: Failed to control actuator." << std::endl;
            }else
            {
                std::cout << "Command sent successfully" << std::endl;
            }
        }
    } else if (upperCommand == "CHECK_SENSORS_RECORDS")
    {
        int input_index = 10;
        std::cout << "Select amount of last readings from DB: " << std::endl;
        std::cin >> input_index ;
        if (sensorService)
        {
            sensorService->displayLastNReadings(input_index);
        }
    } else if (upperCommand == "ADD_ACTUATOR")
    {
        if (!actuatorService)
        {
            std::cerr<<"ERROR: actuatorService is not available." << std::endl;
            return true;
        }
        std::cout << "Write Name of new actuator." << std::endl;
        std::string actuator;
        std::getline(std::cin, actuator);
        if (!actuatorService->addActuator(actuator))
        {
            std::cerr << "ERR: Failed to add actuator." << std::endl;
        }
    }else if (upperCommand == "DELETE_ACTUATOR")
    {
        if (!actuatorService)
        {
            std::cerr<<"ERROR: actuatorService is not available." << std::endl;
            return true;
        }
        std::cout << "Write Name of actuator you want to delete." << std::endl;
        std::string actuator;
        std::getline(std::cin, actuator);
        if (!actuatorService->deleteActuator(actuator))
        {
            std::cerr << "ERR: Failed to delete actuator." << std::endl;
        }
    }else if (upperCommand == "CHECK_LAST_ACTUATOR_EVENTS")
    {
        if (!actuatorService)
        {
            std::cerr<<"ERROR: actuatorService is not available." << std::endl;
            return true;
        }

        int n = 10;
        std::cout << "How long log you want?" << std::endl;
        std::cin >> n;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        actuatorService->displayActuatorEvents(n);
    }else if (upperCommand == "HISTORY")
    {
        if (!sensorService)
        {
            std::cerr<<"ERROR: Sensor Service is not available." << std::endl;
            return true;
        }


        char buffer[256];


        std::cout << "Write from what time: like 27.09.2025|17:26" << std::endl;
        std::cin >> buffer;

        if (const time_t from_time = parseUserInputTime(buffer); from_time == -1)
        {
            std::cerr << "Error: Invalid 'from' time format." << std::endl;
        }
        else
        {
            std::cout << "Write To what time: like 27.09.2025|19:47" << std::endl;
            std::cin >> buffer;
            if (time_t to_time = parseUserInputTime(buffer); to_time == -1) {
                std::cout << "Error: Invalid 'to' time format." << std::endl;
            } else
            {
                sensorService->displayReadingsInTimeRange(from_time, to_time);
            }
        }
    }else if (upperCommand == "HELP")
    {
        displayHelp();
    }else
    {
        std::cout<<"ERROR: Invalid command." << std::endl;
        std::cout << "See help for commands by \"HELP\" command." << std::endl;
    }

    return true;
}


void CommandHandler::displayHelp() {
    std::cout << "All commands and describe to them" << std::endl;
    for (const auto& [cmd, desc] : commandDescriptions) {
        std::cout << cmd << std::endl;
        std::cout << "   " << desc << std::endl;
    }
}


