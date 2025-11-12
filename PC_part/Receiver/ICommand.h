//
// Created by rabilint on 11.11.25.
//

#ifndef RECEIVER_ICOMMAND_H
#define RECEIVER_ICOMMAND_H


#include <string>
#include <iostream>

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual bool executeCommand(const std::string& command)  = 0 ;
    virtual void displayHelp() = 0;
};

#endif //RECEIVER_ICOMMAND_H