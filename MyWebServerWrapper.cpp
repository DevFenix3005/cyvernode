#include "MyWebServerWrapper.h"

MyWebServerWrapper::MyWebServerWrapper(uint16_t serverPort, SensorManager &sensorManager)
  : port(serverPort), server(new AsyncWebServer(serverPort)), sensorManager(sensorManager) {}

MyWebServerWrapper::~MyWebServerWrapper() = default;

void MyWebServerWrapper::begin() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ Error: No hay conexión WiFi. Servidor no iniciado.");
    return;
  }
  setupRoutes();
  server->begin();
  Serial.println("✅ WebServer iniciado en puerto: " + String(port));
}

void MyWebServerWrapper::end() {
  server->end();
  server.reset();
  Serial.println("🔴 Webserver finalizado...");
}

void MyWebServerWrapper::restart() {
  Serial.println("🔄 Reiniciando WebServer...");
  server.reset(new AsyncWebServer(port));
  begin();
}

void MyWebServerWrapper::handleRoot(AsyncWebServerRequest *request) {
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

void MyWebServerWrapper::handleMeasurement(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  digitalWrite(INFO_LED, HIGH);
  sensorManager.turnOnAllSensors();  // 🔥 Encender sensores antes de medir
  delay(333);
  auto payloadWrap = generatePayload(data, len);
  auto response = new AsyncJsonResponse();
  JsonObject root = response->getRoot();

  if (payloadWrap->getDeserializationError()) {
    root["error"] = "❌ Formato JSON inválido.";
    root["cause"] = payloadWrap->getDeserializationError().c_str();
    response->setCode(400);
  } else {
    auto payload = payloadWrap->getJsonDocument();
    if (!payload->containsKey("unit")) {
      root["error"] = "❌ Falta la clave 'unit' en JSON.";
      response->setCode(400);
    } else {
      String unit = (*payload)["unit"];
      if (unit != CELSIUS && unit != FAHRENHEIT) {
        root["error"] = "❌ Unidad inválida. Usa 'CELSIUS' o 'FAHRENHEIT'.";
        response->setCode(400);
      } else {
        float temp = sensorManager.readAverageTemperature(unit);
        float humidity = sensorManager.readAverageHumidity();

        if (isnan(temp) || isnan(humidity)) {
          root["error"] = "⚠️ Error al leer el sensor DHT.";
          response->setCode(500);
        } else {
          root["temperature_unit"] = unit;
          root["temperature"] = temp;
          root["humidity"] = humidity;
          root["heat_index"] = sensorManager.computeHeatIndex(temp, humidity, unit);
          root["light"] = sensorManager.readPhotoResistor();
          root["soil"] = sensorManager.readSoilMoisture();
        }
      }
    }
  }

  response->setLength();
  response->setContentType("application/json");
  digitalWrite(INFO_LED, LOW);
  sensorManager.turnOffAllSensors();  // 🔥 Apagar sensores después de medir
  request->send(response);
}

void MyWebServerWrapper::handleNotFound(AsyncWebServerRequest *request) {
  digitalWrite(INFO_LED, HIGH);
  auto response = new AsyncJsonResponse();
  JsonObject root = response->getRoot();
  response->setCode(404);

  root["message"] = "❌ Ruta no encontrada.";
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

void MyWebServerWrapper::setupRoutes() {
  server->on("/", HTTP_GET, std::bind(&MyWebServerWrapper::handleRoot, this, std::placeholders::_1));
  server->on(
    "/measurement", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    std::bind(&MyWebServerWrapper::handleMeasurement, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  server->onNotFound(std::bind(&MyWebServerWrapper::handleNotFound, this, std::placeholders::_1));
}

std::unique_ptr<PayloadWrap> MyWebServerWrapper::generatePayload(uint8_t *data, size_t len) {
  String body = String((char *)data).substring(0, len);
  Serial.println("📩 Datos recibidos: " + body);

  JsonDocument payload;
  DeserializationError error = deserializeJson(payload, body);

  return std::unique_ptr<PayloadWrap>(new PayloadWrap(std::make_shared<JsonDocument>(payload), error));
}