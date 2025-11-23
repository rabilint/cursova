#include <Arduino.h>
#include "RTC.h"
#include "Arduino_LED_Matrix.h"
#include <Wire.h>
#include <SPI.h>
#include <memory>
#include <Servo.h>
#include "../lib/Adafruit_Sensor-master/Adafruit_Sensor.h"
#include "/home/rabilint/CLionProjects/cursova/Arduino/lib/Adafruit_BME680-master/Adafruit_BME680.h"
#include "BME680_Sensor.h"
#include "CommandProcessor.h"
#include "DataTransmitter.h"
#include "HandshakeHandler.h"
#include "Sensor_Manager.h"

constexpr byte numChars = 124;
char receivedChars[numChars];
constexpr long sensorReadInterval = 150000;
unsigned long lastSensorReadTime = 0;

std::shared_ptr<BME680_Sensor> temperatureSensor;
std::shared_ptr<BME680_Sensor> humiditySensor;
std::shared_ptr<BME680_Sensor> pressureSensor;
std::shared_ptr<BME680_Sensor> gasSensor;


Serial_Handler serialHandler;
BME680Manager bme680Manager;
SensorManager sensorManager;
HandshakeHandler handshakeHandler(&serialHandler);
CommandProcessor commandProcessor;
DataTransmitter data_transmitter(&serialHandler);
ArduinoLEDMatrix matrix;


constexpr int yellowLedPin = 2;
constexpr int redLedPin = 4;

unsigned long lastRequestTime = 0;
constexpr long requestInterval = 2000;

void setup()
{
    pinMode(yellowLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);

    temperatureSensor = std::make_shared<BME680_Sensor>(&bme680Manager, 1, "Temperature", BME680DataType::TEMPERATURE);
    humiditySensor = std::make_shared<BME680_Sensor>(&bme680Manager, 2, "Humidity", BME680DataType::HUMIDITY);
    pressureSensor = std::make_shared<BME680_Sensor>(&bme680Manager, 3, "Pressure", BME680DataType::PRESSURE);
    gasSensor = std::make_shared<BME680_Sensor>(&bme680Manager, 4, "Gas", BME680DataType::GAS);

    serialHandler.initialize();

    matrix.loadSequence(LEDMATRIX_ANIMATION_HEARTBEAT);
    matrix.begin();
    matrix.play(true);


    lastRequestTime = millis();
    handshakeHandler.requestActuatorInfo();
}

unsigned long previousMillis = 0;
constexpr long interval = 5000;

void loop()
{
    if (serialHandler.available())
    {
        String line = serialHandler.read();
        if (!line.isEmpty())
        {
            if (handshakeHandler.handleSensorHandshake(line))
            {
            }

            else if (handshakeHandler.getActuatorHandshakeState() != HandshakeState::COMPLETE)
            {
                handshakeHandler.handleActuatorHandshake(line);
            }
            else
            {
                commandProcessor.processCommand(line);
            }
        }
    }else
    {
        Serial.println(serialHandler.available());
    }

    if (handshakeHandler.shouldRequestActuatorInfo())
    {
        handshakeHandler.requestActuatorInfo();
    }


    if (handshakeHandler.getSensorHandshakeState() == SensorHandshakeState::GIVEN_SENSORS)
    {
        Serial.println("3");
        auto sensorInfo = sensorManager.getSensorInfo();
        data_transmitter.transmitSensorData(sensorInfo);
    }

    if (handshakeHandler.getActuatorHandshakeState() == HandshakeState::COMPLETE &&
        handshakeHandler.getSensorHandshakeState() == SensorHandshakeState::COMPLETE
    )
    {
        Serial.println("4");
        static bool actuatorsUpdated = false;
        if (!actuatorsUpdated) {
            const auto actuators = handshakeHandler.getActuators();
            commandProcessor.setActuators(actuators);
            actuatorsUpdated = true;
        }

        // Perform sensor reading
        if (sensorManager.shouldReadSensors()) {
            bme680Manager.performReading(); // Read once for all BME680 sensors
            auto sensorData = sensorManager.readAllSensors();
            data_transmitter.transmitSensorData(sensorData);
        }
    }
};
