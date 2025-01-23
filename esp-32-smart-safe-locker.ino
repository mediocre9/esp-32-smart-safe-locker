#include "includes/config.hpp"
#include "includes/web_server.hpp"
#include "includes/firebase_operations.hpp"
#include "includes/web_socket.hpp"
#include <LittleFS.h>
#include <Arduino.h>

WebSocket webSocket("/lock-controller");

inline void initializeLockers()
{
  for (const auto &i : _GPIO_PINS_)
  {
    pinMode(i.second, OUTPUT);
    digitalWrite(i.second, HIGH);
  }
}

void setup()
{
  WebServer webServer;

  Serial.begin(BAUD_RATE);
  initializeLockers();

  if (!LittleFS.begin())
  {
    LOGLN("LittleFS Failed!");
    LOGLN("Restarting . . . ");
    ESP.restart();
    return;
  }

  // The order of the below 3 methods
  // should not be changed.
  // Otherwise the code will throw
  // runtime errors . . .
  // because the STA and AP modes were used in
  // [setupDeviceNetworkModes] method
  // and these modes should be setup before starting the server...
  //
  // https://github.com/espressif/arduino-esp32/issues/8661#issuecomment-1731752332
  webServer.setupDeviceNetworkModes();
  webServer.start();
  webServer.setupRoutes();

  if (webServer.isConnectedToInternet())
  {
    webServer.firebase.startAuthentication();
    if (webServer.firebase.isAuthenticated())
    {
      webSocket.init();
      webServer.server.addHandler(&(webSocket.getWebSocketInstance()));
      webServer.firebase.listenForAuthorizationStatus();
    }
  }
}

void loop()
{
  webSocket.getWebSocketInstance().cleanupClients();
}