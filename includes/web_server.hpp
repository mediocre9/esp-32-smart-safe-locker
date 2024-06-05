#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "database.hpp"
#include "firebase_operations.hpp"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <map>

using User = std::map<String, String>;

class WebServer
{
public:
    static void init();
    static void setupRoutes();
    static void establishSTAConnections();
    static void requestHandler();
    static bool isConnectedToWiFi();

    static void defaultHandler();
    static void loginHandler_POST();
    static void networkConfigurationHandler_GET();
    static void networkConfigurationHandler_POST();
    static void userConfigurationHandler_GET();
    static void userConfigurationHandler_POST();

    static void sendHTML(const String &filename);
    static char *generateSessionUUID();
    static bool isAuthorized();

public:
    static User users;
    static String sessionID;
    static CfgDatabase database;
    static bool isWiFiConnected;
    static ESP8266WebServer server;
    static FirebaseOperations firebase;
};

#endif