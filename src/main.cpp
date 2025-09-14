#include <Arduino.h>
#include "RTC.h"
#include "Sensor.h"

Sensor mySensor;

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  RTC.begin();
  RTCTime startTime(24, Month::JUNE, 2025, 0, 0, 00, DayOfWeek::WEDNESDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(startTime);

}

void loop() {

  SensorData currentData = mySensor.readData();
  Serial.print("Temperature: " + String(currentData.temperature) + " C");
  Serial.println("");
  Serial.print("Humidity: " + String(currentData.humidity) );
  Serial.println("");

delay(5000);
}