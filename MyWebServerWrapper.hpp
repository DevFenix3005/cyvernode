#ifndef MYWEBSERVERWRAPPER_HPP
#define MYWEBSERVERWRAPPER_HPP

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <functional>
#include <memory>  // Para std::unique_ptr
#include <DHT.h>
#include "Constants.h"
#include "PayloadWrap.hpp"

class MyWebServerWrapper {
public:
  MyWebServerWrapper(uint16_t serverPort)
    : port(serverPort), server(new AsyncWebServer(serverPort)), dhtSensor(new DHT(DHTPIN, DHTTYPE)) {
  }

  ~MyWebServerWrapper() = default;

  void begin() {
    setupRoutes();
    dhtSensor->begin();
    server->begin();
    Serial.println("Init webserver, port:" + String(port));
  }

  void end() {
    server->end();
    server.reset();  // Libera automáticamente el recurso
    dhtSensor.reset();
    Serial.println("Webserver ended...");
  }

  void restart() {
    Serial.println("Restarting webserver...");
    server.reset(new AsyncWebServer(port));  // Crea una nueva instancia
    dhtSensor.reset(new DHT(DHTPIN, DHTTYPE));
    begin();
  }

  void handleRoot(AsyncWebServerRequest *request) {
    digitalWrite(INFO_LED, HIGH);
    delay(2);
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    root["message"] = "Hello from ESP32!";
    response->setLength();
    response->setContentType("application/json");
    digitalWrite(INFO_LED, LOW);
    request->send(response);
  }

  void handleLed(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    PayloadWrap *payloadWrap = generatePayload(data, len);
    DeserializationError error = payloadWrap->getDeserializationError();

    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();

    if (error) {
      root["message"] = "Invalid JSON format!";
      root["cause"] = error.c_str();
      response->setCode(400);
    } else {
      auto payload = payloadWrap->getJsonDocument();
      if (payload->containsKey("status")) {
        String status = (*payload)["status"];
        if (status == SWAP) {
          status = digitalRead(BLUE_LED) ? OFF : ON;
        }
        if (status == ON) {
          ledTurnOn(root);
        } else if (status == OFF) {
          ledTurnOff(root);
        } else {
          root["message"] = "Invalid status! Use 'ON' or 'OFF' or 'SWAP'.";
          response->setCode(400);
        }
      } else {
        root["message"] = "Missing 'status' key in JSON.";
        response->setCode(400);
      }
    }
    response->setLength();
    response->setContentType("application/json");
    request->send(response);
    delete payloadWrap;
  }

  void handleDht(AsyncWebServerRequest *request) {
    digitalWrite(INFO_LED, HIGH);
    turnOnSensor();
    delay(500);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float humidity = dhtSensor->readHumidity();
    // Read temperature as Celsius (the default)
    float celsius = dhtSensor->readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float fahrenheit = dhtSensor->readTemperature(true);

    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(celsius) || isnan(fahrenheit)) {
      root["error"] = "Failed to read from DHT sensor!";
      response->setCode(500);
    } else {
      root["soil"] = readSoilMoisture();
      root["humidity"] = humidity;
      root["celsius"] = celsius;
      root["fahrenheit"] = fahrenheit;
      // Compute heat index in Celsius (isFahreheit = false)
      root["heat_index_celsius"] = dhtSensor->computeHeatIndex(celsius, humidity, false);
      // Compute heat index in Fahrenheit (the default)
      root["heat_index_fahrenheit"] = dhtSensor->computeHeatIndex(fahrenheit, humidity);
    }
    response->setLength();
    response->setContentType("application/json");
    digitalWrite(INFO_LED, LOW);
    turnOffSensor();
    request->send(response);
  }

  void handleNotFound(AsyncWebServerRequest *request) {
    digitalWrite(INFO_LED, HIGH);
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    response->setCode(400);
    root["message"] = "Path not found";
    root["url"] = request->url();
    root["method"] = request->methodToString();

    JsonDocument jsonArgs;
    for (uint8_t i = 0; i < request->args(); i++) {
      jsonArgs[request->argName(i)] = request->arg(i);
    }
    root["args"] = jsonArgs;
    digitalWrite(INFO_LED, LOW);
    response->setLength();
    response->setContentType("application/json");
    request->send(response);
  }

private:
  std::unique_ptr<AsyncWebServer> server;
  std::unique_ptr<DHT> dhtSensor;
  uint16_t port;

  void setupRoutes() {
    server->on("/", HTTP_GET, std::bind(&MyWebServerWrapper::handleRoot, this, std::placeholders::_1));
    server->on("/measurement", HTTP_GET, std::bind(&MyWebServerWrapper::handleDht, this, std::placeholders::_1));
    server->on(
      "/led", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
      std::bind(&MyWebServerWrapper::handleLed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    server->onNotFound(std::bind(&MyWebServerWrapper::handleNotFound, this, std::placeholders::_1));
  }

  PayloadWrap *generatePayload(uint8_t *data, size_t len) {
    // Convierte el array de datos en un String
    String body = String((char *)data).substring(0, len);
    Serial.println("Datos recibidos: " + body);

    // Crea un DynamicJsonDocument con un tamaño adecuado
    JsonDocument payload;  // Ajusta el tamaño según tus necesidades

    // Intenta deserializar el JSON
    DeserializationError error = deserializeJson(payload, body);

    // Crea un PayloadWrap directamente y lo retorna
    return new PayloadWrap(std::make_shared<JsonDocument>(payload), error);
  }

  void ledTurnOn(JsonObject root) {
    digitalWrite(BLUE_LED, HIGH);  // Encender LED
    root["message"] = "Datos procesados correctamente, Led encendido";
  }

  void ledTurnOff(JsonObject root) {
    digitalWrite(BLUE_LED, LOW);  // Apagar LED
    root["message"] = "Datos procesados correctamente, Led apagado";
  }

  int readSoilMoisture() {
    int measure = analogRead(SOIL_MOISTURE_PIN);  // Read the analog value from sensor
    int percentMeasure = map(measure, DRY, WET, 0, 100);
    return constrain(percentMeasure, 0, 100);
  }

  void turnOnSensor() {
    digitalWrite(POW_DHTPIN, HIGH);
    digitalWrite(POW_SOIL_MOISTURE_PIN, HIGH);
  }

  void turnOffSensor() {
    digitalWrite(POW_DHTPIN, LOW);
    digitalWrite(POW_SOIL_MOISTURE_PIN, LOW);
  }
};

#endif
