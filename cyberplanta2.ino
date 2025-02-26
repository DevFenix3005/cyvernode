#include <unordered_map>
#include <EasyButton.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <TaskScheduler.h>
#include "SensorManager.hpp"
#include "MyWebServerWrapper.h"
#include "Constants.h"

void wateringThePlants();
// Scheduled task
Task wateringTask(TIME_TO_WATERING, TASK_FOREVER, &wateringThePlants);

WiFiManager wm;
Scheduler scheduler;
SensorManager sensorManager;
EasyButton wifiConfigurationButton(TRIGGER_BUTTON);
MyWebServerWrapper myWebServerWrapper(SERVER_PORT, sensorManager);

void handleOnPressed() {
  // start portal w delay
  Serial.println("Starting config portal");
  myWebServerWrapper.end();
  MDNS.end();
  wm.setConfigPortalTimeout(120);

  if (!wm.startConfigPortal("OnDemandAP", "password")) {
    Serial.println("failed to connect or hit timeout");
    delay(3000);
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
  if (MDNS.begin(DNS_HOST)) {
    Serial.println("Inicinado en la direccion http:\\\\" + String(DNS_HOST));
  }
  myWebServerWrapper.restart();
}

void handleOnPressedWithDuration() {
  Serial.println("Button Held");
  Serial.println("Erasing Config, restarting");
  wm.resetSettings();
  ESP.restart();
}

void wateringThePlants() {
  sensorManager.turnOnAllSensors();
  SensorWrapper* waterLvlSensor = sensorManager.getSensor("waterLvl");
  if (!waterLvlSensor) return;

  int waterLvl = waterLvlSensor->readSensor();
  if (waterLvl <= 10) {
    sensorManager.turnOffAllSensors();
    Serial.print("Nivel del agua muy bajo");
    return;
  }

  Serial.print("Regando: ");
  delay(500);
  float temp = sensorManager.readAverageTemperature(CELSIUS);
  float humidity = sensorManager.readAverageHumidity();
  int light = sensorManager.readPhotoResistor();
  int soilMoisturePercentage = sensorManager.readSoilMoisture();
  sensorManager.turnOffAllSensors();

  // 📌 Imprimir valores en la consola serie
  Serial.println("📊 Datos de los sensores:");
  Serial.print("🌡️ Temp: ");
  Serial.print(temp);
  Serial.println("°C");
  Serial.print("💧 Humedad: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("☀️ Luz: ");
  Serial.print(light);
  Serial.println("%");
  Serial.print("🌱 Humedad del suelo: ");
  Serial.print(soilMoisturePercentage);
  Serial.println("%");

  // 📌 Evaluación de riego (sin activar la bomba)
  Serial.print("🔎 Evaluación de riego: ");
  if (soilMoisturePercentage < DRY_THRESHOLD) {                                     // Suelo seco
    if ((temp >= TEMP_MIN && temp <= TEMP_MAX) || humidity < HUMIDITY_THRESHOLD) {  // Rango de temperatura válido o baja humedad del aire
      if (light < LIGHT_THRESHOLD) {                                                // Evitar regar en pleno sol
        Serial.println("💧 Se recomienda regar.");
        waterPlantUntilOptimal();
      } else {
        Serial.println("⛔ Demasiada luz solar. No se recomienda regar. Mover la planta si es necesario");
      }
    } else {
      Serial.println("⛔ Temperatura fuera del rango adecuado. No se recomienda regar.");
    }
  } else {
    Serial.println("🌱 Suelo aún húmedo. No se necesita riego.");
  }
  sensorManager.turnOffAllSensors();
  Serial.println("------------------------------------------------");
}

void waterPlantUntilOptimal() {
  Serial.println("🚰 Iniciando riego automático...");
  SensorWrapper* sensor = sensorManager.getSensor("soil");
  if (sensor) {
    sensor->turnOnSensor();
    // Encender la bomba de agua
    digitalWrite(WATER_PUMP_PIN, HIGH);

    while (true) {
      // Leer la humedad del suelo en porcentaje
      int soilMoisturePercentage = sensor->readSensor();

      Serial.print("💧 Humedad actual del suelo: ");
      Serial.print(soilMoisturePercentage);
      Serial.println("%");

      // Si la humedad del suelo supera el 85%, detenemos el riego
      if (soilMoisturePercentage >= 85) {
        Serial.println("✅ Humedad óptima alcanzada. Deteniendo riego.");
        break;
      }

      delay(1000);  // Esperamos 1 segundo antes de volver a medir
    }

    // Apagar la bomba de agua
    sensor->turnOffSensor();
    digitalWrite(WATER_PUMP_PIN, LOW);
    Serial.println("🚰 Riego completado. Volviendo a la operación normal.");
  }
}

void configureWifiButton() {
  wifiConfigurationButton.begin();
  wifiConfigurationButton.onPressed(handleOnPressed);
  wifiConfigurationButton.onPressedFor(DURATION, handleOnPressedWithDuration);
}

void configScheduler() {
  scheduler.init();
  Serial.println("Initialized scheduler");

  scheduler.addTask(wateringTask);
  Serial.println("added wateringTask");
  delay(5000);

  wateringTask.enable();
  Serial.println("Enabled t1");
}

void configSensores() {
  pinMode(WATER_PUMP_PIN, OUTPUT);
  digitalWrite(WATER_PUMP_PIN, LOW);

  sensorManager.beginAll();
  sensorManager.turnOnAllSensors();
  digitalWrite(WATER_PUMP_PIN, HIGH);

  delay(3000);
  sensorManager.turnOffAllSensors();
  digitalWrite(WATER_PUMP_PIN, LOW);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("\n Starting");
  configSensores();
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP

  std::vector<const char*> menu = { "wifi", "info", "param", "sep", "restart", "exit" };
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");
  wm.setConfigPortalTimeout(30);

  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    if (MDNS.begin(DNS_HOST)) {
      Serial.println("Inicinado en la direccion http:\\\\" + String(DNS_HOST));
    }
    configScheduler();
    configureWifiButton();
    myWebServerWrapper.begin();
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  scheduler.execute();
  wifiConfigurationButton.read();
}
