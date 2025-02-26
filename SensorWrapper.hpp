#ifndef SENSOR_WRAPPER_HPP
#define SENSOR_WRAPPER_HPP

#include <ArduinoJson.h>
class SensorWrapper {
public:
  SensorWrapper(int powerPin, int sensorPin, bool invert = false)
    : powerPin(powerPin), sensorPin(sensorPin), invert(invert) {}

  virtual void begin() {
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, LOW);
  }

  void turnOnSensor() {
    if (powerPin == 0) return;
    digitalWrite(powerPin, HIGH);
  }

  void turnOffSensor() {
    if (powerPin == 0) return;
    digitalWrite(powerPin, LOW);
  }

  virtual int readSensor() {
    int min = invert ? 4095 : 0;
    int max = invert ? 0 : 4095;
    return constrain(map(analogRead(sensorPin), min, max, 0, 100), 0, 100);
  }


protected:
  int powerPin;
  int sensorPin;
  bool invert;
};

#endif