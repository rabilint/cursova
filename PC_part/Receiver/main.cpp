#include <algorithm>
#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <cstdio>
#include <limits>
#include "SerialCommunicator.h"
#include <map>
#include "DatabaseManager.h"
#include "vendor/serial/include/serial/serial.h"


const std::map<std::string, std::string> commands = {
    {"History", "display records from and to custom time."},
    {"Check_DB", "display last 10 records."},
    {"Exit","Close Program."},
    {"Add_actuator","Add new actuator."},
    {"Delete_Actuator","Delete actuator."},
    {"Check_last_actuator_events","Check actuator events history."},
    {"Make_action","Managing actuator"},
};

SerialCommunicator my_cerial("/dev/ttyACM0",9600);

time_t parseUserInputTime(const char* buffer) {
    struct tm timeinfo = {};
    int day = 0, month = 0, year = 0, hour = 0, minute = 0;

    int items_parsed = sscanf(buffer, "%d.%d.%d|%d:%d", &day, &month, &year, &hour, &minute);

    if (items_parsed != 5) {
        return -1;
    }

    timeinfo.tm_year   = year - 1900;
    timeinfo.tm_mon    = month - 1;
    timeinfo.tm_mday   = day;
    timeinfo.tm_hour   = hour;
    timeinfo.tm_min    = minute;
    timeinfo.tm_sec    = 0;

    return mktime(&timeinfo);
}

extern std::atomic<bool> running;
std::vector<SensorDataStruct> records;
void serialReaderThread(SerialCommunicator& serial, DBManager& myDB);

int main()
{
    SerialCommunicator my_serial("/dev/ttyACM0",9600);
    DBManager myDB("../test.db");
    DBManager myActuator("../ActuatorEvents.db", true);


    if (!my_serial.isConnected())
    {
        return -1;
    }
    std::thread readerTread(serialReaderThread, std::ref(my_serial), std::ref(myDB));
    std::cout << "Serial reader thread started." << std::endl;
    std::cout << "Enter the commands" << std::endl;
    std::string command;

    while(running)
    {
        std::cout << "> ";
        std::getline(std::cin, command);
        if (command == "Exit")
        {
            running = false;
        }
        else if (command == "Make_action")
        {
            int input_index = -1;
            std::string action;
            myActuator.listActuators();
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
                std::string ActuatorName = myActuator.getActuatorName(input_index);
                if (ActuatorName.empty())
                {
                    std::cerr << "ERR: Actuator with ID " << input_index << " not found." << std::endl;
                }else
                {
                    std::getline(std::cin, action);
                    std::ranges::transform(action, action.begin() , ::toupper);

                    int newState = -1;
                    std::string commandSuffix;
                    if (input_index > 0)
                    {
                        if (action == "ON")
                        {
                            newState = 1;
                            commandSuffix = "_ON";
                        }else if (action == "OFF")
                        {
                            newState = 0;
                            commandSuffix = "_OFF";
                        }

                        if (newState != -1)
                        {
                            if (myActuator.UpdateActuatorState(input_index, newState))
                            {
                                std::string commandToSend = ActuatorName + commandSuffix;
                                my_serial.writeLine(commandToSend);
                                myActuator.addEvent(ActuatorName, newState);
                                std::cout << "Command sent: " << commandToSend << std::endl;
                            }
                        }else
                        {
                            std::cerr << "ERR: Failed to update database." << std::endl;
                        }
                    }else
                    {
                        std::cerr << "ERR: Invalid action. Please enter ON or OFF." << std::endl;
                    }
                }
            }


        }else if (command == "Check_DB")
        {
            records = myDB.getLastNReadings(10);
            std::cout << "All data: " << std::endl;
            for (SensorDataStruct& record : records)
            {
                std::cout << record.timestamp << " " << record.temperature << " " << record.humidity << std::endl;
            }
        }else if (command == "Add_actuator"){
            std::cout << "Write Name of new actuator." << std::endl;
            std::string actuator;
            std::getline(std::cin, actuator);
            std::ranges::replace(actuator,' ', '_');
            std::ranges::transform(actuator, actuator.begin(), ::toupper);
            myActuator.addActuator(actuator);
        }else if (command == "Delete_Actuator")
        {
            std::cout << "Write name of actuator you want to delete." << std::endl;
            std::string actuator;
            std::getline(std::cin, actuator);
            std::ranges::replace(actuator, ' ', '_');
            std::ranges::transform(actuator, actuator.begin(), ::toupper);
            myActuator.deleteActuator(actuator);
        }else if (command == "Check_last_actuator_events")
        {
            int n = 10;
            std::vector <ActuatorDataStruct> actuator_events;
            actuator_events.reserve(n);
            std::cout << "How long log you want?" << std::endl;
            std::cin >> n;
            actuator_events = myActuator.getActuatorsData(n);
            for (auto event : actuator_events)
            {
                std::cout << "| ID: " << event.ActuatorID << " | Name: " << event.ActuatorName << " | State: " << event.State << " | Time: " << event.timestamp << std::endl;
            }
        }
        else if (command.substr(0,13 ) == "History")
        {

            struct tm * timeinfo;

            char buffer[256];
            time_t from_time, to_time;

            std::cout << "Write from what time: like 27.09.2025|17:26" << std::endl;
            std::cin >> buffer;
            from_time = parseUserInputTime(buffer);

            if (from_time == -1)
            {
                std::cerr << "Error: Invalid 'from' time format." << std::endl;
            }
            else
            {
                std::cout << "Write To what time: like 27.09.2025|19:47" << std::endl;
                std::cin >> buffer;
                to_time = parseUserInputTime(buffer);
                if (to_time == -1) {
                    std::cout << "Error: Invalid 'to' time format." << std::endl;
                } else
                {
                    std::cout << from_time << " " << to_time << std::endl;
                    records = myDB.getReadingsInTimeRange( from_time, to_time);
                }
            }
        }else if (command == "Help")
        {
            std::cout << "All commands and discribe to them" << std::endl;
            for (const auto& pair : commands) {
                std::cout << pair.first << std::endl;
                std::cout << "   " << pair.second << std::endl;
            }
        }
    }

    readerTread.join();
    std::cout << "Program terminated." << std::endl;
    return 0;
};