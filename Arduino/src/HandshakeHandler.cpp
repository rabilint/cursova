//
// Created by rabilint on 22.11.25.
//

#include "HandshakeHandler.h"

HandshakeHandler::HandshakeHandler(Serial_Handler* handler) :
    actuators{},
    currentActuatorsHandshakeState(HandshakeState::WAITING_FOR_SIZE),
    currentSensorHandshakeState(SensorHandshakeState::COMPLETE),
    serialHandler(handler),
    actuatorsReceived(0),
    expectedActuatorCount(0),
    lastRequestTime(0)
{}

void HandshakeHandler::setSensors(const std::array<sensorStruct, maxSensors>& sensorsList)
{
    sensors = sensorsList;
}

bool HandshakeHandler::parseActuatorSize(const String& line, int& size) {
    if (!line.startsWith("Take: ")) return false;

    int startPos = line.indexOf('*');
    int endPos = line.lastIndexOf('*');

    if (startPos != -1 && endPos > startPos) {
        String sizeStr = line.substring(startPos + 1, endPos);
        size = sizeStr.toInt();
        return true;
    }
    return false;
}

bool HandshakeHandler::parseActuatorData(const String& line, char name[26], int& state) {
    if (!line.startsWith("Take: ")) return false;

    int startPos = line.indexOf('#');
    int endPos = line.lastIndexOf('#');

    if (startPos != -1 && endPos > startPos) {
        String data = line.substring(startPos + 1, endPos);
        int spaceIndex = data.indexOf(' ');

        if (spaceIndex != -1) {
            auto sub_name = data.substring(0, spaceIndex);
            if ((sub_name.length() + 1 ) >= 26 )
            {
                strncpy(name, sub_name.c_str(), 26);
            }
            state = data.substring(spaceIndex + 1).toInt();
            return true;
        }
    }
    return false;
}


int HandshakeHandler::getPinForActuator(const String& name) {
    constexpr int redLedPin = 4;
    constexpr int yellowLedPin = 2;
    constexpr int servoPin = 8;

    if (name.equalsIgnoreCase("LED_RED")) {
        return redLedPin;
    } else if (name.equalsIgnoreCase("LED_YELLOW")) {
        return yellowLedPin;
    } else if (name.equalsIgnoreCase("SERVO")) {
        return servoPin;
    }
    return -1;
}


bool HandshakeHandler::handleActuatorHandshake(const String& line) {
    if (currentActuatorsHandshakeState == HandshakeState::WAITING_FOR_SIZE) {
        int size = 0;
        if (parseActuatorSize(line, size)) {
            expectedActuatorCount = size;
            if (size > 0) {
                actuatorsReceived = 0;
                currentActuatorsHandshakeState = HandshakeState::WAITING_FOR_DATA;
            } else {
                currentActuatorsHandshakeState = HandshakeState::COMPLETE;
            }
            return true;
        }
    } else if (currentActuatorsHandshakeState == HandshakeState::WAITING_FOR_DATA) {
        char name[26];
        int state;
        if (parseActuatorData(line, name, state)) {
            if (actuatorsReceived < MAX_ACTUATORS)
            {
                int pin = getPinForActuator(name);

                if (pin != -1) {
                    auto actuatorOpt = ActuatorFactory::createActuator(name, pin, state, *serialHandler);
                    if (actuatorOpt.has_value()) {
                        actuators[actuatorsReceived] = actuatorOpt;
                        actuatorsReceived++;
                    }
                }

                if (actuatorsReceived >= expectedActuatorCount || actuatorsReceived >= MAX_ACTUATORS) {
                    currentActuatorsHandshakeState = HandshakeState::COMPLETE;
                }
            }
            return true;
        }
    }
    return false;
}



bool HandshakeHandler::handleSensorHandshake(const String& line) {
    if (line.equalsIgnoreCase("RECOMMIT_SYNC")) {
        currentSensorHandshakeState = SensorHandshakeState::GIVEN_SENSORS;
        return true;
    } else if (line.equalsIgnoreCase("SENSORS_SYNCHRONIZED")) {
        currentSensorHandshakeState = SensorHandshakeState::COMPLETE;
        return true;
    }
    return false;
}

HandshakeState HandshakeHandler::getActuatorHandshakeState() const {
    return currentActuatorsHandshakeState;
}

SensorHandshakeState HandshakeHandler::getSensorHandshakeState() const {
    return currentSensorHandshakeState;
}

std::array<std::optional<std::shared_ptr<IActuator>>, MAX_ACTUATORS> HandshakeHandler::getActuators()
{
    return actuators;
}


void HandshakeHandler::requestActuatorInfo() {
    if (serialHandler) {
        serialHandler->sendLine("GiveActuatorInfo");
        lastRequestTime = millis();
    }
}

bool HandshakeHandler::shouldRequestActuatorInfo() const {
    if (currentActuatorsHandshakeState == HandshakeState::COMPLETE) {
        return false;
    }
    unsigned long currentTime = millis();
    return (currentTime - lastRequestTime >= requestInterval);
}
