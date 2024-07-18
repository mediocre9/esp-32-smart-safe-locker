#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "database.hpp"
#include "firebase_operations.hpp"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>

using User = std::map<String, String>;

class WebServer
{
public:
    static void start();
    static void setupRoutes();
    static void setupSTAMode();
    static bool isConnectedToWiFi();

    static void loginHandler_GET(AsyncWebServerRequest *request);
    static void loginHandler_POST(AsyncWebServerRequest *request);
    static void usersHandler_GET(AsyncWebServerRequest *request);
    static void usersHandler_POST(AsyncWebServerRequest *request);
    static void deviceWifiHandler_GET(AsyncWebServerRequest *request);
    static void deviceWifiHandler_POST(AsyncWebServerRequest *request);
    static void homeWifiHandler_GET(AsyncWebServerRequest *request);
    static void homeWifiHandler_POST(AsyncWebServerRequest *request);
    static void changePasswordHandler_GET(AsyncWebServerRequest *request);
    static void changePasswordHandler_POST(AsyncWebServerRequest *request);
    static void lockController_POST(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

    static void sendHTML(AsyncWebServerRequest *request, const String &filename);
    static String generateSessionUUID();
    static bool isAuthorized(AsyncWebServerRequest *request);

public:
    static User users;
    static String sessionID;
    static CfgDatabase database;
    static bool isWiFiConnected;
    static AsyncWebServer server;
    static FirebaseOperations firebase;
};

#endif
