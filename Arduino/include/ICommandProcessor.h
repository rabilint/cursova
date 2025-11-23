//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_ICOMMANDPROCESSOR_H
#define ARDUINO_ICOMMANDPROCESSOR_H
#include "Arduino.h"
class ICommandProcessor {
public:
    virtual ~ICommandProcessor() = default;
    virtual bool processCommand(const String& command) = 0;
};
#endif //ARDUINO_ICOMMANDPROCESSOR_H