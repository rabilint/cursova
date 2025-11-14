//
// Created by rabilint on 18.09.25.
//
#include "SerialCommunicator.h"
#include <iostream>

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



