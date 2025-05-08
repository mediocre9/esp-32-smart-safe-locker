#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "../includes/Database.hpp"
#include "../includes/FirebaseOperations.hpp"

using UserToGPIO = std::map<String, String>;

class HttpServer {
public:
    static void start();
    static void setupRoutes();

    static AsyncWebServer server;
    static FirebaseOperations firebase;

private:
    static void loginHandler_GET(AsyncWebServerRequest* request);
    static void loginHandler_POST(AsyncWebServerRequest* request);
    static void usersHandler_GET(AsyncWebServerRequest* request);
    static void usersHandler_POST(AsyncWebServerRequest* request);
    static void deviceWifiHandler_GET(AsyncWebServerRequest* request);
    static void deviceWifiHandler_POST(AsyncWebServerRequest* request);
    static void homeWifiHandler_GET(AsyncWebServerRequest* request);
    static void homeWifiHandler_POST(AsyncWebServerRequest* request);
    static void changePasswordHandler_GET(AsyncWebServerRequest* request);
    static void changePasswordHandler_POST(AsyncWebServerRequest* request);
    static void rebootDeviceHandler_GET(AsyncWebServerRequest* request);
    static void servePage(AsyncWebServerRequest* request, const String& filename);
    static void notFoundHandler_GET(AsyncWebServerRequest* request);

    // APIs
    static void unlockHandler_GET(AsyncWebServerRequest* request);
    static void healthEndpointHandler_GET(AsyncWebServerRequest* request);
    static void rebootEndpointHandler_GET(AsyncWebServerRequest* request);
    static void lockController_POST(AsyncWebServerRequest* request, uint8_t* data, size_t length, size_t index, size_t total);

    static UserToGPIO users;
    static CfgDatabase database;
};

#endif