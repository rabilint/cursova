#include <Arduino.h>
#include "RTC.h"
#include "Sensor.h"

Sensor mySensor;
constexpr byte numChars = 124;
char receivedChars[numChars];
boolean newData = false;
constexpr long sensorReadInterval = 5000;
unsigned long lastSensorReadTime = 0;

constexpr int yellowLedPin = 2;
constexpr int redLedPin = 4;
constexpr int greenLedPin = 8;

struct ActuatorStruct
{
  String name{};
  int pin;
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

void setup() {

  Serial.begin(9600);
  while (!Serial) {}
  RTC.begin();
  RTCTime startTime(24, Month::JUNE, 2025, 0, 0, 00, DayOfWeek::WEDNESDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(startTime);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  Serial.print("GiveActuatorInfo");

}

void loop() {
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
  unsigned long currentTime = millis();
  if (currentTime - lastSensorReadTime >= sensorReadInterval) {
    lastSensorReadTime = currentTime;
    SensorData currentData = mySensor.readData();
    Serial.print("T:");
    Serial.print(currentData.temperature);
    Serial.print(";H:");
    Serial.print(currentData.humidity);
    Serial.print("\n");
  };
}
