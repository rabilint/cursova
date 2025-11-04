#include <algorithm>
#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <cstdio>
#include <limits>
#include "SerialCommunicator.h"
#include <map>
#include "SensorDataManager.h"
#include "ActuatorDataManager.h"
#include "vendor/serial/include/serial/serial.h"


const std::map<std::string, std::string> commands = {
    {"History", "display records from and to custom time."},
    {"Check_DB", "display last 10 records."},
    {"Exit","Close Program."},
    {"Add_actuator","Add new actuator."},
    {"Delete_actuator","Delete actuator."},
    {"Check_last_actuator_events","Check actuator events history."},
    {"Make_action","Managing actuator"},
};

SerialCommunicator my_cerial("/dev/ttyACM0",9600);

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

//sensors config

//

extern std::atomic<bool> running;
std::vector<RecordDataStruct> records;
void serialReaderThread(SerialCommunicator& serial, SensorDataManager& SensorDBM, ActuatorDataManager& ActuatorDBM);

int main()
{
    SerialCommunicator my_serial("/dev/ttyACM0",9600);
    SensorDataManager sensorDBM("../SensorData.db");
    ActuatorDataManager actuatorDBM("../ActuatorEvents.db");





    if (!my_serial.isConnected())
    {
        return -1;
    }
    std::thread readerTread(serialReaderThread, std::ref(my_serial), std::ref(sensorDBM), std::ref(actuatorDBM));
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
            for (std::vector<ActuatorStruct> ActuatorList = actuatorDBM.listActuators(); const auto& [ActuatorID, ActuatorName, State] : ActuatorList)
            {
                std::cout << "| ID: " << ActuatorID;
                std::cout << " | Name: " << ActuatorName;
                std::cout << " | State: " << State;
                std::cout <<" |" << std::endl;
            }
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
                std::string ActuatorName = actuatorDBM.getActuatorName(input_index);
                if (ActuatorName.empty())
                {
                    std::cerr << "ERR: Actuator with ID " << input_index << " not found." << std::endl;
                }else
                {
                    std::string action;
                    std::getline(std::cin, action);
                    std::ranges::transform(action, action.begin() , ::toupper);

                    if (input_index > 0)
                    {
                        std::string commandSuffix;
                        int newState = -1;
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
                            if (actuatorDBM.updateActuatorState(input_index, newState))
                            {
                                std::string commandToSend = ActuatorName + commandSuffix;
                                my_serial.writeLine(commandToSend);
                                actuatorDBM.addEvent(ActuatorName, newState);
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

            records = sensorDBM.getLastNReadings(10);
            std::cout << "All data: " << std::endl;
            for (int i =0; i < records.size(); i++)
            {
                tm* gmt_time_info = gmtime(&records[i].timestamp);
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", gmt_time_info);

                if (i > 0 && records[i].timestamp == records[i-1].timestamp)
                {
                    std::cout << " | " << records[i].SensorName << " : " << records[i].Data;
                }else
                {
                    std::cout<< std::endl;
                    std::cout << buffer << " | : " <<  records[i].SensorName << " : "<< records[i].Data ;
                }
            }
            std::cout << std::endl;
        }else if (command == "Add_actuator"){
            std::cout << "Write Name of new actuator." << std::endl;
            std::string actuator;
            std::getline(std::cin, actuator);
            std::ranges::replace(actuator,' ', '_');
            std::ranges::transform(actuator, actuator.begin(), ::toupper);
            actuatorDBM.addActuator(actuator);
        }else if (command == "Delete_actuator")
        {
            std::cout << "Write name of actuator you want to delete." << std::endl;
            std::string actuator;
            std::getline(std::cin, actuator);
            std::ranges::replace(actuator, ' ', '_');
            std::ranges::transform(actuator, actuator.begin(), ::toupper);
            actuatorDBM.deleteActuator(actuator);
        }else if (command == "Check_last_actuator_events")
        {
            int n = 10;
            std::vector <ActuatorDataStruct> actuator_events;
            actuator_events.reserve(n);
            std::cout << "How long log you want?" << std::endl;
            std::cin >> n;
            actuator_events = actuatorDBM.getActuatorsData(n);
            for (const auto& [ActuatorID, State, ActuatorName, timestamp] : actuator_events)
            {
                tm* gmt_time_info = gmtime(&timestamp);
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", gmt_time_info);
                std::cout << "| ID: " << ActuatorID << " | Name: " << ActuatorName << " | State: " << State << " | Time: " << buffer << std::endl;
            }
        }
        else if (command == "History")
        {

            struct tm;

            char buffer[256];
            time_t from_time;

            std::cout << "Write from what time: like 27.09.2025|17:26" << std::endl;
            std::cin >> buffer;
            from_time = parseUserInputTime(buffer);

            if (from_time == -1)
            {
                std::cerr << "Error: Invalid 'from' time format." << std::endl;
            }
            else
            {
                time_t to_time;
                std::cout << "Write To what time: like 27.09.2025|19:47" << std::endl;
                std::cin >> buffer;
                to_time = parseUserInputTime(buffer);
                if (to_time == -1) {
                    std::cout << "Error: Invalid 'to' time format." << std::endl;
                } else
                {
                    std::cout << from_time << " " << to_time << std::endl;
                    records = sensorDBM.getReadingsInTimeRange( from_time, to_time);
                    for (int i = 0; i < records.size(); i++)
                    {
                        struct ::tm* gmt_time_info = gmtime(&records[0].timestamp);
                        char TimeBuffer[80];
                        strftime(TimeBuffer, sizeof(TimeBuffer), "%Y-%m-%d %H:%M:%S UTC", gmt_time_info);
                        if (i > 0 && records[i].timestamp == records[i-1].timestamp)
                        {
                            std::cout << " | " << records[i].SensorName << " : " << records[i].Data;
                        }else
                        {
                            std::cout << TimeBuffer << " | : " <<  records[i].SensorName << " | : "<< records[i].Data << std::endl;

                        }
                    }
                }
            }
        }else if (command == "Help")
        {
            std::cout << "All commands and describe to them" << std::endl;
            for (const auto& [fst, snd] : commands) {
                std::cout << fst << std::endl;
                std::cout << "   " << snd << std::endl;
            }
        }else
        {
            std::cout << "ERR: Invalid command." << std::endl;
        }
    }

    readerTread.join();
    std::cout << "Program terminated." << std::endl;
    return 0;
};