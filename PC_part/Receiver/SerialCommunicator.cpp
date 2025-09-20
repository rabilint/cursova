//
// Created by rabilint on 18.09.25.
//
#include "SerialCommunicator.h"
#include <iostream>
#include <thread>
#include <atomic>

SerialCommunicator::SerialCommunicator(const std::string& port, uint32_t baudrate) {
    try {
        // Налаштовуємо таймаут: чекати до 1 секунди на дані
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
    // readline() ідеально підходить для нашого протоколу
    return serial_port.readline();
}

void SerialCommunicator::writeLine(const std::string& line) {
    if (!isConnected()) return;
    // Записуємо рядок і додаємо символ нового рядка
    serial_port.write(line + "\n");
}

std::atomic<bool> running = true;

void serialReaderThread(SerialCommunicator& serial)
{
    while (running)
    {
        std::string line = serial.readLine();
        if (line.length() > 0)
        {
            std::cout << line << std::endl;
        }
    }
    std::cout << "[Serial Thread] Exiting." << std::endl;
}