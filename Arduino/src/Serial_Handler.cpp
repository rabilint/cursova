//
// Created by rabilint on 23.11.25.
//

#include "Serial_Handler.h"
#include <Arduino.h>


void Serial_Handler::initialize()
{
    Serial.begin(9600);
    while (!Serial)
    {
    }
}

bool Serial_Handler::available()
{
    return true;
}

void Serial_Handler::sendLine(const String& line)
{
    Serial.print(line);
    Serial.print("\n");
}

String Serial_Handler::read()
{
    constexpr byte numChars = 124;
    static char receivedChars[numChars];
    static byte ndx = 0;
    char endMarker = '\n';
    char receivedChar;
    String result = "";

    while (Serial.available() > 0) {
        receivedChar = Serial.read();

        if (receivedChar == '\r') {
            continue;
        }

        if (receivedChar != endMarker) {
            receivedChars[ndx] = receivedChar;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        } else {
            receivedChars[ndx] = '\0';
            result = String(receivedChars);
            ndx = 0;
            return result;
        }
    }
    return result;
}




