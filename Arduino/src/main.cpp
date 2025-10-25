#include <Arduino.h>
#include "RTC.h"
#include "Arduino_LED_Matrix.h"
#include <Wire.h>
#include <SPI.h>
#include "/home/rabilint/CLionProjects/cursova/Arduino/lib/Adafruit_Sensor-master/Adafruit_Sensor.h"
#include "/home/rabilint/CLionProjects/cursova/Arduino/lib/Adafruit_BME680-master/Adafruit_BME680.h"

constexpr byte numChars = 124;
char receivedChars[numChars];
boolean newData = false;
constexpr long sensorReadInterval = 5000;
unsigned long lastSensorReadTime = 0;


#define SEALEVELPRESSURE_HPA (1002)
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

constexpr int yellowLedPin = 2;
constexpr int redLedPin = 4;
constexpr int greenLedPin = 8;

struct ActuatorStruct
{
  String name{};
  int pin{};
  int State{};
};

ActuatorStruct* actuatorsArray = nullptr;
int numActuators = 0;
int actuatorsReceived = 0;

enum class HandshakeState { WAITING_FOR_SIZE, WAITING_FOR_DATA, COMPLETE };
HandshakeState currentState = HandshakeState::WAITING_FOR_SIZE;

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
    }else
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
  if (!line.startsWith("Take: "))
  {
    return;
  }
  if (currentState == HandshakeState::WAITING_FOR_SIZE)
  {
    int startPos = line.indexOf('*');
    int endPos = line.lastIndexOf('*');
    if (startPos != -1 && endPos > startPos )
    {
      String size = line.substring(startPos + 1, endPos);
      int numActuators = size.toInt();
      if (numActuators > 0)
      {
        actuatorsArray = new ActuatorStruct[numActuators];
        currentState = HandshakeState::WAITING_FOR_DATA;
      }else
      {
        currentState = HandshakeState::COMPLETE;
      }
    }
  }

  if (currentState == HandshakeState::WAITING_FOR_DATA)
  {
    int startPos = line.indexOf('#');
    int endPos = line.lastIndexOf('#');
    if (startPos != -1 && endPos > startPos )
    {
      String data = line.substring(startPos + 1, endPos);
      int spaceIndex = data.indexOf(' ');
      if (spaceIndex != -1 && spaceIndex < numActuators)
      {
        String name = data.substring(0, spaceIndex);
        bool state = data.substring(spaceIndex + 1).toInt() == 1;
        actuatorsArray[actuatorsReceived].name = name;
        actuatorsArray[actuatorsReceived].State = state;

        if (name.equalsIgnoreCase("LED_RED")) {
          actuatorsArray[actuatorsReceived].pin = redLedPin;
        } else if (name.equalsIgnoreCase("LED_GREEN")) {
          actuatorsArray[actuatorsReceived].pin = greenLedPin;
        }

        actuatorsReceived++;
      }
    }

    if (actuatorsReceived >= numActuators)
    {
      for (int i = 0; i < numActuators; i++)
      {
        digitalWrite(actuatorsArray[i].pin, actuatorsArray[i].State ? HIGH : LOW);

      }
      currentState = HandshakeState::COMPLETE;
    }
  }

}

void processCommand(  )
{
  String command = String(receivedChars);

  for (int i = 0; i < numActuators; i++)
  {
    if (command.equalsIgnoreCase(actuatorsArray[i].name + "_ON"))
    {
      digitalWrite(actuatorsArray[i].pin, HIGH);
      actuatorsArray[i].State = true;
      break;
    }
    if (command.equalsIgnoreCase(actuatorsArray[i].name + "_OFF"))
    {
      digitalWrite(actuatorsArray[i].pin, LOW);
      actuatorsArray[i].State = false;
      break;
    }
  }
}

ArduinoLEDMatrix matrix;

void setup() {

  //Adafruit Sensor BME680

  Serial.println(F("BME680 async test"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    for (;;);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  //

  Serial.begin(9600);
  while (!Serial) {}
  RTC.begin();
  RTCTime startTime(24, Month::JUNE, 2025, 0, 0, 00, DayOfWeek::WEDNESDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(startTime);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  matrix.loadSequence(LEDMATRIX_ANIMATION_HEARTBEAT);
  matrix.begin();
  matrix.play(true);


  Serial.print("GiveActuatorInfo");

}

unsigned long previousMillis = 0;
constexpr long interval = 5000;

void loop()
{
  unsigned long endTime = millis();
  if (endTime == 0)
  {
    Serial.println(F("Failed to begin readings: ( "));
    return;
  }

  // Serial.print(F("Reading completed at "));
  // Serial.println(millis());
  //
  // Serial.print(F("Temperature = "));
  // Serial.print(bme.temperature);
  // Serial.println(F(" *C"));
  //
  // Serial.print(F("Pressure = "));
  // Serial.print(bme.pressure / 100.0);
  // Serial.println(F(" hPa"));
  //
  // Serial.print(F("Humidity = "));
  // Serial.print(bme.humidity);
  // Serial.println(F(" %"));
  //
  // Serial.print(F("Gas = "));
  // Serial.print(bme.gas_resistance / 1000.0);
  // Serial.println(F(" KOhms"));
  //
  // Serial.print(F("Approx. Altitude = "));

  bme.readAltitude(SEALEVELPRESSURE_HPA);


  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    Serial.print("T:");
    Serial.print(bme.temperature);
    Serial.print(";H:");
    Serial.print(bme.humidity);
  }

  receiveSerialData();
  if (newData == true)
  {
    if (currentState == HandshakeState::COMPLETE)
    {
      // Serial.println("New data received");
      // Serial.println(receivedChars);
      processCommand();
    }else
    {
      handleInitialStatus();
    }
    newData = false;
  }
  delay(5000);
};

