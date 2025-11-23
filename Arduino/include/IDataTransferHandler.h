//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_IDATATRANSFERHANDLER_H
#define ARDUINO_IDATATRANSFERHANDLER_H
#include <api/String.h>


class IDataTransferHandler
{
    public:
    virtual ~IDataTransferHandler() = default;
    virtual bool available();
    virtual String read();
    virtual void sendLine(const String& line) = 0;
    virtual void initialize() = 0;
};


#endif //ARDUINO_IDATATRANSFERHANDLER_H