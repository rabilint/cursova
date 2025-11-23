//
// Created by rabilint on 22.11.25.
//

#ifndef ARDUINO_HANDSHAKEHANDLER_H
#define ARDUINO_HANDSHAKEHANDLER_H

#include "ActuatorFactory.h"
#include "ISerialHandler.h"
#include "ISensor.h"
#include <memory>
#include "IDataTransferHandler.h"
#include "IActuator.h"

enum class HandshakeState {
    WAITING_FOR_SIZE,
    WAITING_FOR_DATA,
    COMPLETE
};

enum class SensorHandshakeState {
    GIVEN_SENSORS,
    COMPLETE
};


class HandshakeHandler
{
    private:
    std::array<std::optional<std::shared_ptr<IActuator>>, MAX_ACTUATORS> actuators;
    HandshakeState currentActuatorsHandshakeState;
    SensorHandshakeState currentSensorHandshakeState;
    Serial_Handler* serialHandler;
    std::array<sensorStruct, maxSensors> sensors;
    int actuatorsReceived;
    int expectedActuatorCount;
    unsigned long lastRequestTime;
    constexpr static long requestInterval = 2000;

    static bool parseActuatorSize(const String& line, int& size);
    static bool parseActuatorData(const String& line, char name[], int& state);
    static int getPinForActuator(const String& name);
    public:

    explicit HandshakeHandler(Serial_Handler* handler);
    void setSensors(const std::array<sensorStruct, maxSensors>& sensorsList);
    bool handleActuatorHandshake(const String& line);
    bool handleSensorHandshake(const String& line);
    [[nodiscard]] HandshakeState getActuatorHandshakeState() const;
    [[nodiscard]] SensorHandshakeState getSensorHandshakeState() const;
    std::array<std::optional<std::shared_ptr<IActuator>>, MAX_ACTUATORS> getActuators();
    void requestActuatorInfo();
    [[nodiscard]] bool shouldRequestActuatorInfo() const;


};


#endif //ARDUINO_HANDSHAKEHANDLER_H