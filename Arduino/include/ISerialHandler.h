//
// Created by rabilint on 22.11.25.
//

#ifndef ARDUINO_ISERIALHANDLER_H
#define ARDUINO_ISERIALHANDLER_H
#include <Arduino.h>

class ISerialHandler {
public:
    virtual ~ISerialHandler() = default;
    virtual bool available() = 0;
    virtual String readLine() = 0;
    virtual void writeLine(const String& line) = 0;
    virtual void begin(unsigned long baudrate) = 0;
};


#endif //ARDUINO_ISERIALHANDLER_H