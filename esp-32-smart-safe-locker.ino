#if __cplusplus < 201703L
#error This project requires a compiler of c++ 17 or above.
#endif

#include <Arduino.h>
#include <LittleFS.h>

#include "includes/Config.hpp"
#include "includes/HttpServer.hpp"
#include "includes/NetworkManager.hpp"
#include "includes/WebSocket.hpp"

#if EXPERIMENTAL_FEATURE
WebSocket webSocket("/locks");
#endif
WiFiNetworkManager network;

inline void initializeGPIOS();
void setupServers();

void setup() {
    Serial.begin(BAUD_RATE);
    initializeGPIOS();

    if (!LittleFS.begin()) {
        LOGLN("LittleFS Failed!");
        LOGLN("Restarting . . . ");
        ESP.restart();
        return;
    }

    setupServers();
}

void loop() {
#if EXPERIMENTAL_FEATURE
    if (network.isConnectedToInternet() && HttpServer::firebase.isAuthenticated()) {
        webSocket.getWebSocketInstance().cleanupClients();
    }
#endif
}

inline void initializeGPIOS() {
    for (uint16_t pinNo : GPIOS) {
        pinMode(pinNo, OUTPUT);
        digitalWrite(pinNo, HIGH);
    }
}

void setupServers() {
    network.setupNetworks();
    HttpServer::start();
    HttpServer::setupRoutes();

    if (!network.isConnectedToInternet()) {
        return;
    }

    HttpServer::firebase.authenticate();
    if (!HttpServer::firebase.isAuthenticated()) {
        return;
    }

#if EXPERIMENTAL_FEATURE
    webSocket.init();
    HttpServer::server.addHandler(&(webSocket.getWebSocketInstance()));
#endif
    HttpServer::firebase.listenForAuthorizationStatus();
}