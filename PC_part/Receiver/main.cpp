#include <iostream>
#include <thread>
#include <atomic>
#include "SerialCommunicator.h"
#include "vendor/serial/include/serial/serial.h"

// serial::Serial my_serial;

SerialCommunicator my_cerial("/dev/ttyACM0",9600);

extern std::atomic<bool> running;
void serialReaderThread(SerialCommunicator& serial);

int main()
{
    SerialCommunicator my_cerial("/dev/ttyACM0",9600);
    if (!my_cerial.isConnected())
    {
        return -1;
    }
    std::thread readerTread(serialReaderThread, std::ref(my_cerial));
    std::cout << "Serial reader thread started." << std::endl;
    std::cout << "Enter the commands" << std::endl;
    std::string command;

    while(running)
    {
        std::cout << "> ";
        std::cin >> command;
        if (command == "exit")
        {
            running = false;
        } else if ( command == "LED_RED_ON")
        {
            my_cerial.writeLine("LED_RED_ON");
            std::cout << "LED_RED_ON" << std::endl;
        }
        else if (command == "LED_RED_OFF")
        {
            my_cerial.writeLine("LED_RED_OFF");
            std::cout << "LED_RED_OFF" << std::endl;
        }
    }

    readerTread.join();
    std::cout << "Program terminated." << std::endl;
    return 0;
};