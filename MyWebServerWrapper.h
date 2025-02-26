#ifndef MYWEBSERVERWRAPPER_HPP
#define MYWEBSERVERWRAPPER_HPP

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <functional>
#include <memory>
#include "Constants.h"
#include "PayloadWrap.hpp"
#include "SensorManager.hpp"

class MyWebServerWrapper {
public:
  MyWebServerWrapper(uint16_t serverPort, SensorManager &sensorManager);
  ~MyWebServerWrapper();

  void begin();
  void end();
  void restart();
  void handleRoot(AsyncWebServerRequest *request);
  void handleWaterPump(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleMeasurement(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleNotFound(AsyncWebServerRequest *request);

private:
  std::unique_ptr<AsyncWebServer> server;
  SensorManager &sensorManager;
  uint16_t port;

  void setupRoutes();
  std::unique_ptr<PayloadWrap> generatePayload(uint8_t *data, size_t len);
};

#endif  // MYWEBSERVERWRAPPER_HPP
