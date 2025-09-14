#include <Arduino.h>
#include "RTC.h"
#include "Sensor.h"

Sensor mySensor;
constexpr byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;


constexpr int yellowLedPin = 2;
constexpr int redLedPin = 4;
constexpr int greenLedPin = 8;


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


void processCommand()
{
  Serial.print("processCommand: ");
  Serial.println(receivedChars);
  if (strcmp(receivedChars, "LED_RED_ON") == 0)
  {
    digitalWrite(redLedPin, HIGH);
    Serial.println("ACK: Red LED turned ON");
  }
  if (strcmp(receivedChars, "LED_RED_OFF") == 0)
  {
    digitalWrite(redLedPin, LOW);
    Serial.println("ACK: Red LED turned OFF");
  }
  if (strcmp(receivedChars, "LED_GREEN_ON") == 0)
  {
    digitalWrite(greenLedPin, HIGH);
    Serial.println("ACK: Green LED turned ON");
  }
  if (strcmp(receivedChars, "LED_GREEN_OFF") == 0)
  {
    digitalWrite(greenLedPin, LOW);
    Serial.println("ACK: Green LED turned OFF");
  }
  if (strcmp(receivedChars, "LED_YELLOW_ON") == 0)
  {
    digitalWrite(yellowLedPin, HIGH);
    Serial.println("ACK: Yellow LED turned ON");
  }
  if (strcmp(receivedChars, "LED_YELLOW_OFF") == 0)
  {
    digitalWrite(yellowLedPin, LOW);
    Serial.println("ACK: Yellow LED turned OFF");
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
}

void loop() {

  SensorData currentData = mySensor.readData();
  // Serial.print("T:");
  // Serial.print(currentData.temperature);
  // Serial.print(";H:");
  // Serial.print(currentData.humidity);
  // Serial.println();

  receiveSerialData();
  if (newData == true)
  {
    Serial.println("New data received");
    Serial.println(receivedChars);
    processCommand();
    newData = false;
  }


}