#ifndef ISERIALPORT_H
#define ISERIALPORT_H

#include <string>

class ISerialPort {
public:
    virtual ~ISerialPort() = default;
    virtual bool isConnected() const = 0;
    virtual std::string readLine() = 0;
    virtual void writeLine(const std::string& line) = 0;
};

#endif // ISERIALPORT_H

