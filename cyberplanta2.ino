#include <EasyButton.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include "MyWebServerWrapper.hpp"
#include "Constants.h"

WiFiManager wm;  // global wm instance
MyWebServerWrapper myWebServerWrapper(SERVER_PORT);
EasyButton ledButton(TRIGGER_BUTTON);

void configLeds() {
  pinMode(INFO_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(POW_DHTPIN, OUTPUT);
  pinMode(POW_SOIL_MOISTURE_PIN, OUTPUT);

  digitalWrite(INFO_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(POW_DHTPIN, LOW);
  digitalWrite(POW_SOIL_MOISTURE_PIN, LOW);
}

void setup() {
  // put your setup code here, to run once:
  configLeds();
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  digitalWrite(BLUE_LED, HIGH);
  delay(3000);
  digitalWrite(BLUE_LED, LOW);
  Serial.println("\n Starting");

  ledButton.begin();
  ledButton.onPressed(handleOnPressed);
  ledButton.onPressedFor(DURATION, handleOnPressedWithDuration);

  std::vector<const char *> menu = { "wifi", "info", "param", "sep", "restart", "exit" };
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
    myWebServerWrapper.begin();
  }
}

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


void loop() {
  // put your main code here, to run repeatedly:
  ledButton.read();
}
