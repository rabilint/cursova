#include <Arduino.h>
#include "RTC.h"
#include "Arduino_LED_Matrix.h"
#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include "/home/rabilint/CLionProjects/cursova/Arduino/lib/Adafruit_Sensor-master/Adafruit_Sensor.h"
#include "/home/rabilint/CLionProjects/cursova/Arduino/lib/Adafruit_BME680-master/Adafruit_BME680.h"

constexpr byte numChars = 124;
char receivedChars[numChars];
boolean newData = false;
constexpr long sensorReadInterval = 150000;
unsigned long lastSensorReadTime = 0;


#define SEALEVELPRESSURE_HPA (1002)
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

struct sensorStruct
{
    int SensorID{};
    String SensorName{};
    double Data{};
};

std::array<sensorStruct, 4> sensorsArray = {{
    {1, "Temperature", 0},
    {2, "Humidity", 0},
    {3, "Pressure", 0},
    {4, "Gas", 0}
}};

constexpr int yellowLedPin = 2;
constexpr int redLedPin = 4;
// constexpr int greenLedPin = 8;

struct ActuatorStruct
{
    String name{};
    int pin{};
    int State{};

    //functoins to turn on and off
    void (*on_func)(int){};
    void (*off_func)(int){};
};

Servo servo;

void turnOnServo(const int pin)
{
    if (!servo.attached())
    {
        servo.attach(pin);
    }
    servo.write(90);
}

void turnOffServo(const int pin)
{
    if (!servo.attached())
    {
        servo.attach(pin);
    }
    servo.write(0);
}

void turnOnLED(const int pin)
{
    digitalWrite(pin, HIGH);
}

void turnOffLED(const int pin)
{
    digitalWrite(pin, LOW);
}

ActuatorStruct* actuatorsArray = nullptr;
int numActuators = 0;
int actuatorsReceived = 0;

enum class HandshakeState { WAITING_FOR_SIZE, WAITING_FOR_DATA, COMPLETE };

enum class SensorHandshakeState { GIVED_SENORS, COMPLETE };

HandshakeState currentState = HandshakeState::WAITING_FOR_SIZE;
SensorHandshakeState currentSenorHandShakeState = SensorHandshakeState::GIVED_SENORS;

void receiveSerialData()
{
    static byte ndx = 0;
    char endMarker = '\n';
    char receivedChar;

    while (Serial.available() > 0 && newData == false)
    {
        receivedChar = Serial.read();

        if (receivedChar == '\r')
        {
            continue;
        }

        if (receivedChar != endMarker)
        {
            receivedChars[ndx] = receivedChar;
            ndx++;
            if (ndx >= numChars)
            {
                ndx = numChars - 1;
            }
        }
        else
        {
            receivedChars[ndx] = '\0';
            ndx = 0;
            newData = true;
        }
    }
}

void handleInitialStatus()
{
    String line = String(receivedChars);
    if (!line.startsWith("Take: ")) return;

    if (currentState == HandshakeState::WAITING_FOR_SIZE)
    {
        // Serial.print("HANDSHAKE: Processing size... | ");
        int startPos = line.indexOf('*');
        int endPos = line.lastIndexOf('*');

        if (startPos != -1 && endPos > startPos)
        {
            String sizeStr = line.substring(startPos + 1, endPos);

            numActuators = sizeStr.toInt();
            // Serial.print("HANDSHAKE: Expecting " + String(numActuators) + " actuators. | ");

            if (numActuators > 0)
            {
                actuatorsArray = new ActuatorStruct[numActuators];
                currentState = HandshakeState::WAITING_FOR_DATA;
            }
            else
            {
                currentState = HandshakeState::COMPLETE;
            }
        }
    }
    else if (currentState == HandshakeState::WAITING_FOR_DATA)
    {
        int startPos = line.indexOf('#');
        int endPos = line.lastIndexOf('#');

        if (startPos != -1 && endPos > startPos)
        {
            String data = line.substring(startPos + 1, endPos);
            int spaceIndex = data.indexOf(' ');

            if (spaceIndex != -1)
            {
                String name = data.substring(0, spaceIndex);
                bool state = data.substring(spaceIndex + 1).toInt() == 1;

                actuatorsArray[actuatorsReceived].name = name;
                actuatorsArray[actuatorsReceived].State = state;

                if (name.equalsIgnoreCase("LED_RED"))
                {
                    actuatorsArray[actuatorsReceived].pin = redLedPin;
                    actuatorsArray[actuatorsReceived].on_func = turnOnLED;
                    actuatorsArray[actuatorsReceived].off_func = turnOffLED;
                }
                if (name.equalsIgnoreCase("SERVO"))
                {
                    actuatorsArray[actuatorsReceived].pin = 8;
                    servo.attach(actuatorsArray[actuatorsReceived].pin);
                    actuatorsArray[actuatorsReceived].on_func = turnOnServo;
                    actuatorsArray[actuatorsReceived].off_func = turnOffServo;
                }

                // Serial.print("HANDSHAKE: Received actuator " + String(actuatorsReceived + 1) + "/" + String(numActuators) + ": " + name + " | ");
                actuatorsReceived++;
            }
        }

        if (actuatorsReceived >= numActuators)
        {
            // Serial.print("HANDSHAKE: Complete. Setting initial states. | ");
            for (int i = 0; i < numActuators; i++)
            {
                actuatorsArray[i].State
                    ? actuatorsArray->on_func(actuatorsArray[i].pin)
                    : actuatorsArray->off_func(actuatorsArray[i].pin);
                // виконуємо вмикання або вимикання actuator-a за допомогою вбудованої функції
                // актуатор[і].Стан ? так ->  функція_ввімкнення(актуатор[i].пін) : ні -> функція_вимкнення(актуатор[i].пін)
            }
            currentState = HandshakeState::COMPLETE;
        }
    }
}

void processCommand()
{
    String command = String(receivedChars);
    if (command.startsWith("SENSORS_SYNCHRONIZED"))
    {
        currentSenorHandShakeState = SensorHandshakeState::COMPLETE;
        return;
    }

    // Serial.print("This command ! : "+command + " " + numActuators); //отладка
    for (int i = 0; i < numActuators; i++)
    {
        if (command.equalsIgnoreCase(actuatorsArray[i].name + "_ON"))
        {
            actuatorsArray[i].on_func(actuatorsArray[i].pin);
            actuatorsArray[i].State = true;
            break;
        }
        if (command.equalsIgnoreCase(actuatorsArray[i].name + "_OFF"))
        {
            actuatorsArray[i].off_func(actuatorsArray[i].pin);
            actuatorsArray[i].State = false;
            break;
        }
    }
}

ArduinoLEDMatrix matrix;

unsigned long lastRequestTime = 0;
constexpr long requestInterval = 2000;

void setup()
{
    servo.attach(8);
    servo.write(270);
    //Adafruit Sensor BME680

    Serial.print(F("BME680 async test"));

    if (!bme.begin())
    {
        Serial.print(F("Could not find a valid BME680 sensor, check wiring!"));
        for (;;);
    }

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    //

    Serial.begin(9600);
    while (!Serial)
    {
    }
    RTC.begin();
    RTCTime startTime(24, Month::JUNE, 2025, 0, 0, 00, DayOfWeek::WEDNESDAY, SaveLight::SAVING_TIME_ACTIVE);
    RTC.setTime(startTime);
    pinMode(yellowLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);
    // pinMode(greenLedPin, OUTPUT);

    matrix.loadSequence(LEDMATRIX_ANIMATION_HEARTBEAT);
    matrix.begin();
    matrix.play(true);


    Serial.print("GiveActuatorInfo\n");
    lastRequestTime = millis();
}

unsigned long previousMillis = 0;
constexpr long interval = 5000;

void loop()
{
    receiveSerialData();
    if (newData)
    {
        if (currentState != HandshakeState::COMPLETE)
        {
            // Serial.print("\n");
            handleInitialStatus();
        }
        else
        {
            // Serial.print("New data received");
            // Serial.print(receivedChars);
            processCommand();
        }
        newData = false;
    }

    if (currentState != HandshakeState::COMPLETE)
    //повторний запит у випадку якщо не вийшло отримати дані actuator-ів з першої спроби.
    {
        unsigned long currentTime = millis();
        if (currentTime - lastRequestTime >= requestInterval)
        {
            // Serial.print("No response, requesting info again...");
            Serial.print("GiveActuatorInfo\n");
            lastRequestTime = currentTime;
        }
    }

    else if (currentSenorHandShakeState == SensorHandshakeState::GIVED_SENORS )
    {
        Serial.print("Take: *");
        Serial.print(sensorsArray.size());
        Serial.println("*");

        for (const auto& sensor : sensorsArray) {
            Serial.print("Take: #");
            Serial.print(sensor.SensorID);
            Serial.print(' ');
            Serial.print(sensor.SensorName);
            Serial.print("#\n");
        }
    }


    if (currentState == HandshakeState::COMPLETE && currentSenorHandShakeState == SensorHandshakeState::COMPLETE)
    {
        bme.readAltitude(SEALEVELPRESSURE_HPA);
        unsigned long currentTime = millis();
        if (currentTime - lastSensorReadTime >= sensorReadInterval)
        {
            lastSensorReadTime = currentTime;

            sensorsArray[0].Data = bme.readTemperature();
            sensorsArray[1].Data = bme.readHumidity();
            sensorsArray[2].Data = bme.readPressure() / 100;
            sensorsArray[3].Data = bme.readGas() / 1000;
            //Відправка даних
            for (const auto& sensor : sensorsArray)
            {
                Serial.print("ID:#");
                Serial.print(sensor.SensorID);
                Serial.print('|');
                Serial.print(sensor.Data);
                Serial.print("#\n");

            }
        };

    }
};
