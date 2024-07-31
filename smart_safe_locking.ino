#include "includes/config.hpp"
#include "includes/web_server.hpp"
#include "includes/firebase_operations.hpp"
#include <Arduino.h>
#include <LittleFS.h>

WebServer server;
FirebaseOperations firebase;

void initializeRelays()
{
  for (const auto &i : _GPIO_PINS_)
  {
    pinMode(i.second, OUTPUT);
    digitalWrite(i.second, LOW);
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);
  initializeRelays();

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Failed!");
    Serial.println("Restarting . . . ");
    ESP.restart();
    return;
  }

  // The order of the below 3 methods
  // should not be changed.
  // Otherwise the code will throw
  // runtime errors . . .
  // because the STA mode was setup in
  // [setupSTAMode] method
  // and for settting the WiFi mode
  // it should be at the beginning...
  //
  // https://github.com/espressif/arduino-esp32/issues/8661#issuecomment-1731752332
  server.setupSTAMode();
  server.start();
  server.setupRoutes();

  if (server.isConnectedToWiFi())
  {
    firebase.configure();
  }
}

void loop() {}
