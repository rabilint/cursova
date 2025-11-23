//
// Created by rabilint on 23.11.25.
//

#ifndef ARDUINO_SERIAL_HANDLER_H
#define ARDUINO_SERIAL_HANDLER_H
#include "IDataTransferHandler.h"


class Serial_Handler : public IDataTransferHandler
{
public:

    bool available() override;
    String read() override;
    void sendLine(const String& line) override;
    void initialize() override;
};


#endif //ARDUINO_SERIAL_HANDLER_H