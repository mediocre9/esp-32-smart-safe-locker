
#include "../includes/web_server.hpp"
#include "../includes/config.hpp"
#include <UUID.h>
#include <LittleFS.h>

ESP8266WebServer WebServer::server = 80;
bool WebServer::isWiFiConnected = false;
User WebServer::users;
String WebServer::sessionID;
CfgDatabase WebServer::database;
FirebaseOperations WebServer::firebase;

void WebServer::init()
{
    Serial.println("Loading users data . . . ");

    users = database.read(USERS_CFG_FILE);
    for (const auto &i : users)
    {
        Serial.println(i.first + " " + i.second);
    }

    const char *headers[] = {"Cookie"};
    size_t headerSize = sizeof(headers) / sizeof(char *);
    server.collectHeaders(headers, headerSize);
    server.begin();
}

bool WebServer::isConnectedToWiFi()
{
    return isWiFiConnected;
}

void WebServer::requestHandler()
{
    server.handleClient();
}

void WebServer::establishSTAConnections()
{
    WiFi.mode(WIFI_STA);
    WiFi.softAP(NODEMCU_SSID, NODEMCU_PWD);
    Serial.print("Access Point started. IP Address: ");
    Serial.println(WiFi.softAPIP());

    Cfg networkConfig = database.read(NETWORK_CFG_FILE);

    auto networkSsid = networkConfig.find("ssid");
    auto networkPwd = networkConfig.find("pwd");

    if (networkSsid == networkConfig.end() &&
        networkPwd == networkConfig.end())
    {
        return;
    }

    String ssid = networkSsid->second;
    String pwd = networkPwd->second;
    ssid.trim();
    pwd.trim();

    if (ssid.isEmpty() || pwd.isEmpty())
    {
        return;
    }

    int timeout = 400;
    Serial.println("Establishing connection to the internet!");
    WiFi.begin(ssid.c_str(), pwd.c_str());
    while (WiFi.status() != WL_CONNECTED && timeout > 0)
    {
        Serial.print(".");
        delay(50);
        timeout--;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nInternet connection failed! Please check your SSID and PASSWORD.");
    }
    isWiFiConnected = true;
    Serial.println("Connected to Internet!");
}

void WebServer::sendHTML(const String &filename)
{
    File file = LittleFS.open(filename.c_str(), FILE_MODE_READ);

    if (!file)
    {
        Serial.println("Failed to open " + filename);
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

char *WebServer::generateSessionUUID()
{
    UUID uuid;
    uuid.seed(random(9999999), random(9999999));
    uuid.generate();
    return uuid.toCharArray();
}

bool WebServer::isAuthorized()
{
    if (!server.hasHeader("Cookie"))
    {
        Serial.println("Cookie Header not found!");
        return false;
    }

    String cookie = server.header("Cookie");
    if (cookie.indexOf("SESSIONID=" + sessionID) == -1)
    {
        Serial.println("Authentication Failed");
        return false;
    }

    Serial.println("Authentication Successful");
    return true;
}

void WebServer::setupRoutes()
{
    server.on("/", HTTP_GET, defaultHandler);
    server.on("/login", HTTP_POST, loginHandler_POST);
    server.on("/network-configuration", HTTP_GET, networkConfigurationHandler_GET);
    server.on("/network-configuration", HTTP_POST, networkConfigurationHandler_POST);
    server.on("/users-configuration", HTTP_GET, userConfigurationHandler_GET);
    server.on("/users-configuration", HTTP_POST, userConfigurationHandler_POST);

    /*
     * @todo Add firebase operations [isAccessBlocked] method
     * that if allowed then only these routes will service
     * otherwise no.
     */
    server.on("/connect", HTTP_POST, []() {});
    server.on("/lock-controller", HTTP_POST, []() {});
}

void WebServer::defaultHandler()
{
    sendHTML("/index.html");
}

void WebServer::loginHandler_POST()
{
    sessionID = generateSessionUUID();

    if (server.arg("username") == NODEMCU_AUTH_USERNAME &&
        server.arg("password") == NODEMCU_AUTH_PWD)
    {
        server.sendHeader("Location", "/network-configuration");
        server.sendHeader("Cache-Control", "no-cache");
        server.sendHeader("Set-Cookie", "SESSIONID=" + sessionID + "; Path=/; HttpOnly");
        server.send(302);
        return;
    }

    Serial.println("Login failed, redirecting to /");
    server.sendHeader("Location", "/");
    server.send(302);
}

void WebServer::networkConfigurationHandler_GET()
{
    if (!isAuthorized())
    {
        Serial.println("Not authorized!");
        server.sendHeader("Location", "/");
        server.send(302);
        return;
    }
    sendHTML("/network-configuration.html");
}

void WebServer::networkConfigurationHandler_POST()
{
    if (!isAuthorized())
    {
        Serial.println("Not authorized!");
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
        return;
    }

    String networkSsid = server.arg("network_ssid");
    String networkPwd = server.arg("network_password");

    networkSsid.trim();
    networkPwd.trim();

    std::map<String, String> networkConfiguration = {
        {"ssid", networkSsid},
        {"pwd", networkPwd},
    };
    bool saved = database.write(NETWORK_CFG_FILE, networkConfiguration);

    if (!saved)
    {
        Serial.println("Failed to save network configuration.");
        sendHTML("/network-configuration.html");
        return;
    }

    Serial.println("Network configuration saved:");
    Serial.println("SSID: " + networkSsid);
    Serial.println("Password: " + networkPwd);
    server.sendHeader("Location", "/restart");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "SESSIONID=0");
    server.send(302, "text/plain", "");
    sendHTML("/restart.html");
    delay(4000);
    ESP.restart();
}

void WebServer::userConfigurationHandler_GET()
{
    if (!isAuthorized())
    {
        Serial.println("Not authorized!");
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
        return;
    }
    sendHTML("/users-configuration.html");
}

void WebServer::userConfigurationHandler_POST()
{
    Serial.println("Users Configuration!");
    if (!isAuthorized())
    {
        Serial.println("Not authorized!");
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
        return;
    }

    const int SIZE = server.args();
    std::map<String, String> userEmails;
    for (int i = 0; i < SIZE; i++)
    {
        String email = server.arg("email" + String(i));
        email.trim();
        if (!email.isEmpty())
        {
            Serial.println("Saved email:" + email);
            userEmails.insert(std::make_pair(String(_GPIO_PINS_.at(i)), email));
        }
    }

    bool saved = database.write(USERS_CFG_FILE, userEmails);
    if (saved)
    {
        Serial.println("Users have beed saved!");
        server.sendHeader("Location", "/restart");
        server.sendHeader("Cache-Control", "no-cache");
        server.sendHeader("Set-Cookie", "SESSIONID=0");
        server.send(302, "text/plain", "");
        sendHTML("/restart.html");
        delay(4000);
        ESP.restart();
    }
    else
    {
        Serial.println("Failed to save users!");
        sendHTML("/users-configuration.html");
    }
}