#ifndef SENSOR_MANAGER_HPP
#define SENSOR_MANAGER_HPP

#include <map>
#include <Arduino.h>
#include "Constants.h"
#include "SensorWrapper.hpp"
#include "DhtSensorWrap.hpp"

class SensorManager {
public:
  // Constructor
  SensorManager()
    : sensors() {
    createSensores();
  }


  // Método para agregar un sensor al mapa
  void createSensores() {
    addSensor("soil", new SensorWrapper(POW_SOIL_MOISTURE_PIN, SOIL_MOISTURE_PIN, true));
    addSensor("light", new SensorWrapper(POW_PHOTORES_PIN, PHOTORES_PIN));
    addSensor("dht", new DhtSensorWrap(POW_DHTPIN, DHTPIN, DHTTYPE));
    addSensor("waterLvl", new SensorWrapper(POW_WATER_LVL_PIN, WATER_LVL_PIN));
  }

  void beginAll() {
    for (auto &sensor : sensors) {
      sensor.second->begin();
    }
  }

  // Método para encender todos los sensores
  void turnOnAllSensors() {
    for (auto &sensor : sensors) {
      sensor.second->turnOnSensor();
    }
  }

  // Método para apagar todos los sensores
  void turnOffAllSensors() {
    for (auto &sensor : sensors) {
      sensor.second->turnOffSensor();
    }
  }

  // Método para obtener un sensor específico
  SensorWrapper *getSensor(const char *name) {
    if (sensors.find(name) != sensors.end()) {
      return sensors[name];
    }
    return nullptr;
  }


  float readAverageTemperature(String unit) {
    SensorWrapper *sensor = getSensor("dht");
    if (sensor) {
      DhtSensorWrap *dhtSensor = static_cast<DhtSensorWrap *>(sensor);
      float sumTemp = 0;
      for (int i = 0; i < MEASURE_TIMES; i++) {
        sumTemp += dhtSensor->readTemperature(unit);
        delay(100);  // Pequeño delay entre mediciones para evitar lecturas repetidas idénticas
      }
      return sumTemp / MEASURE_TIMES;
    } else {
      return 0.0;
    }
  }

  float readAverageHumidity() {
    SensorWrapper *sensor = getSensor("dht");
    if (sensor) {
      DhtSensorWrap *dhtSensor = static_cast<DhtSensorWrap *>(sensor);

      float sumHumidity = 0;
      for (int i = 0; i < MEASURE_TIMES; i++) {
        sumHumidity += dhtSensor->readHumidity();
        delay(100);
      }
      return sumHumidity / MEASURE_TIMES;
    } else {
      return 0.0;
    }
  }

  float computeHeatIndex(float temp, float humidity, String unit) {
    SensorWrapper *sensor = getSensor("dht");
    if (sensor) {
      DhtSensorWrap *dhtSensor = static_cast<DhtSensorWrap *>(sensor);
      return dhtSensor->computeHeatIndex(temp, humidity, unit);
    } else {
      return 0.0;
    }
  }

  int readSoilMoisture() {
    SensorWrapper *sensor = getSensor("soil");
    return sensor->readSensor();
  }

  int readPhotoResistor() {
    SensorWrapper *sensor = getSensor("light");
    return sensor->readSensor();
  }


private:
  std::map<const char *, SensorWrapper *> sensors;  // Mapa para almacenar los sensores

  void addSensor(const char *name, SensorWrapper *sensor) {
    sensors[name] = sensor;
  }
};

#endif
