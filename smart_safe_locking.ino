#include "includes/config.hpp"
#include "includes/web_server.hpp"
#include "includes/firebase_operations.hpp"
#include <Arduino.h>
#include <LittleFS.h>

WebServer webServer;
FirebaseOperations firebase;

void setup()
{
  Serial.begin(BAUD_RATE);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  if (!LittleFS.begin())
  {
    Serial.println("Failed to mount LittleFS!");
    return;
  }

  webServer.init();
  webServer.establishSTAConnections();
  webServer.setupRoutes();

  if (webServer.isConnectedToWiFi())
  {
    firebase.configure();
  }
}

void loop()
{
  webServer.requestHandler();

  if (webServer.isConnectedToWiFi())
  {
    firebase.listen();
  }
}
