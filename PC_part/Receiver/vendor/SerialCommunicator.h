//
// Created by rabilint on 18.09.25.
//

#ifndef RECEIVER_SERIALCOMMUNICATOR_H
#define RECEIVER_SERIALCOMMUNICATOR_H

#include <serial/serial.h>
#include <string>

class SerialCommunicator {
public:
    SerialCommunicator(const std::string& port, uint32_t baudrate);
    ~SerialCommunicator();

    bool isConnected() const;
    std::string readLine();
    void writeLine(const std::string& line);

private:
    serial::Serial serial_port;
};

#endif //RECEIVER_SERIALCOMMUNICATOR_H