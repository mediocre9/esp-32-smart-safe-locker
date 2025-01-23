
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "../includes/database.hpp"
#include "../includes/firebase_operations.hpp"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>

using User = std::map<String, String>;

class WebServer
{
public:
    static void start();
    static void setupRoutes();
    static void setupDeviceNetworkModes();
    static bool isConnectedToInternet();
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
    static void rebootDeviceHandler_GET(AsyncWebServerRequest *request);
    static void lockController_GET(AsyncWebServerRequest *request);
    static void sendHTML(AsyncWebServerRequest *request, const String &filename);
    static String createSession();
    static bool isAuthorized(AsyncWebServerRequest *request);

public:
    static User users;
    static String sessionID;
    static CfgDatabase database;
    static bool isInternetConnected;
    static AsyncWebServer server;
    static FirebaseOperations firebase;
};

#endif
