#include "../includes/config.hpp"
#include "../includes/web_server.hpp"
#include <LittleFS.h>
#include <UUID.h>

AsyncWebServer WebServer::server(80);
bool WebServer::isWiFiConnected = false;
User WebServer::users;
String WebServer::sessionID;
CfgDatabase WebServer::database;
FirebaseOperations WebServer::firebase;

TaskHandle_t firebaseTask = NULL;

void lockAllLockers()
{
    for (const auto &i : _GPIO_PINS_)
    {
        digitalWrite(i.second, LOW);
    }
}

/**
 * Polling every 20 secs to check the
 * status of firmware organizational
 * revocation status.
 */
void firebaseListenerTask(void *parameter)
{
    for (;;)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            WebServer::firebase.listen();
            lockAllLockers();
        }
        vTaskDelay(pdMS_TO_TICKS(20000)); // 20 secs
    }
}

void WebServer::start()
{
    Serial.println("Loading users data . . . ");

    users = database.read(USERS_CFG_FILE);
    for (const auto &i : users)
    {
        Serial.println(i.first + " " + i.second);
    }

    server.begin();

#if _PROD_MODE_
    // [firebase.listen()] is a synchronous
    // code and we need to execute it independently
    // in separate task . . .
    // running on CPU-1
    xTaskCreatePinnedToCore(
        firebaseListenerTask,
        "firebase-listener-task",
        10000,
        NULL,
        1,
        &firebaseTask,
        1);
#endif
}

bool WebServer::isConnectedToWiFi()
{
    return isWiFiConnected;
}

void WebServer::setupSTAMode()
{
    WiFi.mode(WIFI_STA);

    Cfg deviceConfig = database.read(DEVICE_WIFI_CFG_FILE);
    auto deviceSsid = deviceConfig.find("device_ssid");
    auto devicePwd = deviceConfig.find("device_pwd");

    String deviceCurrentSsid = ESP_SSID;
    String deviceCurrentPwd = ESP_PWD;

    if (deviceSsid != deviceConfig.end() && devicePwd != deviceConfig.end())
    {
        String ssid = deviceSsid->second;
        String pwd = devicePwd->second;
        ssid.trim();
        pwd.trim();

        if (!ssid.isEmpty() && !pwd.isEmpty())
        {
            deviceCurrentSsid = ssid;
            deviceCurrentPwd = pwd;
        }
    }

    WiFi.disconnect(true);
    WiFi.softAP(deviceCurrentSsid.c_str(), deviceCurrentPwd.c_str());
    Serial.print("Access Point started. IP Address: ");
    Serial.println(WiFi.softAPIP());

#if _PROD_MODE_
    Cfg homeConfig = database.read(HOME_WIFI_CFG_FILE);

    auto homeSsid = homeConfig.find("home_ssid");
    auto homePwd = homeConfig.find("home_pwd");

    if (homeSsid == homeConfig.end() && homePwd == homeConfig.end())
    {
        return;
    }

    String ssid = homeSsid->second;
    String pwd = homePwd->second;
    ssid.trim();
    pwd.trim();

    if (ssid.isEmpty() || pwd.isEmpty())
    {
        return;
    }

    int timeout = 50;
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
        Serial.println("\nInternet connection failed! Please check your ssid and password.");
        return;
    }

#endif
    isWiFiConnected = true;
    Serial.println("Connected to Internet!");
}

void WebServer::sendHTML(AsyncWebServerRequest *request, const String &filename)
{
    File file = LittleFS.open(filename.c_str(), FILE_MODE_READ);

    if (!file)
    {
        Serial.println("Failed to open " + filename);
        request->send(404, "text/plain", "File not found");
        return;
    }
    request->send(file, filename, "text/html");
    file.close();
}

String WebServer::generateSessionUUID()
{
    UUID uuid;
    uuid.seed(random(9999999), random(9999999));
    uuid.generate();
    return String(uuid.toCharArray());
}

bool WebServer::isAuthorized(AsyncWebServerRequest *request)
{
    if (!request->hasHeader("Cookie"))
    {
        Serial.println("Cookie Header not found!");
        return false;
    }

    String cookie = request->header("Cookie");
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
    server.on("/", HTTP_GET, loginHandler_GET);
    server.on("/login", HTTP_POST, loginHandler_POST);

    server.on("/users", HTTP_GET, usersHandler_GET);
    server.on("/users", HTTP_POST, usersHandler_POST);

    server.on("/device-wifi", HTTP_GET, deviceWifiHandler_GET);
    server.on("/device-wifi", HTTP_POST, deviceWifiHandler_POST);

    server.on("/home-wifi", HTTP_GET, homeWifiHandler_GET);
    server.on("/home-wifi", HTTP_POST, homeWifiHandler_POST);

    server.on("/change-password", HTTP_GET, changePasswordHandler_GET);
    server.on("/change-password", HTTP_POST, changePasswordHandler_POST);

    server.onRequestBody(lockController_POST);
}

void WebServer::loginHandler_GET(AsyncWebServerRequest *request)
{
    sendHTML(request, "/index.html");
}

void WebServer::loginHandler_POST(AsyncWebServerRequest *request)
{
    sessionID = generateSessionUUID();

    String username = request->arg("username");
    String password = request->arg("password");

    username.trim();
    password.trim();

    String currentPassword = ESP_LOCAL_WEB_AUTH_PWD;
    Cfg loginPassword = database.read(LOGIN_CFG_FILE);

    auto p = loginPassword.find("password");
    if (p != loginPassword.end())
    {
        String customPassword = p->second;
        customPassword.trim();

        if (!customPassword.isEmpty())
        {
            currentPassword = customPassword;
        }
    }

    if (username == ESP_LOCAL_WEB_AUTH_USERNAME && password == currentPassword)
    {
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Location", "/home-wifi");
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Set-Cookie", "SESSIONID=" + sessionID + "; Path=/; HttpOnly");
        request->send(response);
        return;
    }

    Serial.println("Login failed!");
    request->redirect("/");
}

void WebServer::usersHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/users.html");
}

void WebServer::usersHandler_POST(AsyncWebServerRequest *request)
{
    Serial.println("Users Configuration!");

    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }

    std::map<String, String> userEmails;
    const int totalArgs = request->args();
    for (int i = 0; i < totalArgs; i++)
    {
        String email = request->arg("email_" + String(i + 1));
        email.trim();

        if (!email.isEmpty())
        {
            Serial.println("Saved email: " + email);
            userEmails.insert(std::make_pair(String(_GPIO_PINS_.at(i)), email));
        }
    }

    bool saved = database.write(USERS_CFG_FILE, userEmails);
    if (!saved)
    {
        Serial.println("Failed to save users!");
        sendHTML(request, "/users.html");
        return;
    }

    Serial.println("Users have been saved!");
    sendHTML(request, "/restart.html");
    request->send(204, "text/plain", "");
    ESP.restart();
}

void WebServer::deviceWifiHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/device-wifi.html");
}

void WebServer::deviceWifiHandler_POST(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }

    String deviceSsid = request->arg("device_ssid");
    String devicePwd = request->arg("device_pwd");

    deviceSsid.trim();
    devicePwd.trim();

    std::map<String, String> deviceWifiDetails = {
        {"device_ssid", deviceSsid},
        {"device_pwd", devicePwd},
    };

    bool saved = database.write(DEVICE_WIFI_CFG_FILE, deviceWifiDetails);

    if (!saved)
    {
        Serial.println("Failed to save device wifi configuration");
        request->send(500, "text/plain", "Failed to save device wifi configuration");
        return;
    }

    Serial.println("Device Wifi configuration saved successfully");
    sendHTML(request, "/restart.html");
    request->send(204, "text/plain", "");
    ESP.restart();
}

void WebServer::homeWifiHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/home-wifi.html");
}

void WebServer::homeWifiHandler_POST(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }

    String homeSsid = request->arg("home_ssid");
    String homePwd = request->arg("home_pwd");

    homeSsid.trim();
    homePwd.trim();

    std::map<String, String> homeWifiDetails = {
        {"home_ssid", homeSsid},
        {"home_pwd", homePwd},
    };

    bool saved = database.write(HOME_WIFI_CFG_FILE, homeWifiDetails);

    if (!saved)
    {
        Serial.println("Failed to save home wifi configuration");
        request->send(500, "text/plain", "Failed to save home wifi configuration");
        return;
    }

    Serial.println("Home Wifi configuration saved successfully");
    sendHTML(request, "/restart.html");
    request->send(204, "text/plain", "");
    ESP.restart();
}

void WebServer::changePasswordHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/change-password.html");
}

void WebServer::changePasswordHandler_POST(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        Serial.println("Not authorized!");
        request->redirect("/");
        return;
    }

    String password = request->arg("password");
    password.trim();

    std::map<String, String> loginDetails = {
        {"password", password},
    };

    bool saved = database.write(LOGIN_CFG_FILE, loginDetails);

    if (!saved)
    {
        Serial.println("Failed to save login password!");
        request->send(500, "text/plain", "Failed to change password!");
        return;
    }

    Serial.println("Login password changed saved successfully");
    sendHTML(request, "/restart.html");
    request->send(204, "text/plain", "");
    ESP.restart();
}

void WebServer::lockController_POST(AsyncWebServerRequest *request, uint8_t *data, size_t length, size_t index, size_t total)
{
#if _PROD_MODE_
    if (!isConnectedToWiFi())
    {
        request->send(403, "text/plain", "Locker is offline. Please ask the administrator to configure the internet connection.");
        return;
    }

    if (!firebase.isAuthorized())
    {
        lockAllLockers();
        request->send(403, "text/plain", "Access to all lockers is restricted. Please contact CUSIT Makerspace R&D Lab for assistance.");
        return;
    }
#endif

    String email = "";
    for (size_t i = 0; i < length; i++)
    {
        email += (char)data[i];
    }

    Serial.print(email);

    if (request->url() == "/connect")
    {
        for (const auto &i : users)
        {
            if (email == i.second)
            {
                email.clear();
                request->send(200, "text/plain", "Connection established!");
                return;
            }
        }
    }

    if (request->url() == "/unlock")
    {
        for (const auto &i : users)
        {
            if (email == i.second)
            {
                email.clear();
                digitalWrite(i.first.toInt(), HIGH);
                request->send(200, "text/plain", "Locker " + String(i.first) + " is now unlocked!");
                return;
            }
        }
    }

    email.clear();
    request->send(403, "text/plain", "Access restricted. The administrator must grant access.");
    return;
}