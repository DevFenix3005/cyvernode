#ifndef DHT_SENSOR_WRAPPER_HPP
#define DHT_SENSOR_WRAPPER_HPP

#include <DHT.h>
#include "Constants.h"
#include "SensorWrapper.hpp"

class DhtSensorWrap : public SensorWrapper {
public:
  DhtSensorWrap(int powerPin, int sensorPin, int dhtType)
    : SensorWrapper(powerPin, sensorPin), dht(sensorPin, dhtType) {}

  void begin() override {
    dht.begin();
  }

  int readSensor() override {
    return 0;
  }

  float readTemperature(String unit) {
    return (unit == CELSIUS) ? dht.readTemperature() : dht.readTemperature(true);
  }

  float readHumidity() {
    return dht.readHumidity();
  }

  float computeHeatIndex(float temp, float humidity, String unit) {
    return dht.computeHeatIndex(temp, humidity, unit == FAHRENHEIT);
  }


private:
  DHT dht;
};


#endif