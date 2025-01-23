#include "../includes/config.hpp"
#include "../includes/web_server.hpp"
#include "../includes/web_socket.hpp"
#include "../includes/database.hpp"
#include "../includes/firebase_operations.hpp"

#include <UUID.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <CustomJWT.h>
#include <Arduino_JSON.h>

#include <ctime>
#include <string>

AsyncWebServer WebServer::server(80);
bool WebServer::isInternetConnected = false;
User WebServer::users;
String WebServer::sessionID;
CfgDatabase WebServer::database;
FirebaseOperations WebServer::firebase;

TaskHandle_t firebaseTask = nullptr;

inline void lockAllLockers()
{
    for (const auto &i : _GPIO_PINS_)
    {
        digitalWrite(i.second, HIGH);
    }
}

void firebaseListenerTask(void *parameter)
{
    unsigned long previousMillisClient = 0;
    unsigned long previousMillisFirebase = 0;
    const unsigned long CLIENT_INTERVAL = 1000;         // 1 sec . . .
    const unsigned long FIREBASE_INTERVAL = 120 * 1000; // 120 sec . . .

    for (;;)
    {
        unsigned long currentMillis = millis();
        if ((currentMillis - previousMillisFirebase) >= FIREBASE_INTERVAL)
        {
            previousMillisFirebase = currentMillis;

            if (WiFi.status() == WL_CONNECTED)
            {
                if (WebServer::firebase.isAuthenticated())
                {
                    WebServer::firebase.listenForAuthorizationStatus();
                    String authorizationStatus = WebServer::firebase.isOrganizationAuthorized()
                                                     ? "Authorized " + String(ORGANIZATION)
                                                     : "Unauthorized " + String(ORGANIZATION);
                    LOGLN(authorizationStatus);
                }
            }
        }

        // WebSocket client operations
        if ((currentMillis - previousMillisClient) >= CLIENT_INTERVAL)
        {
            previousMillisClient = currentMillis;

            ClientManager &manager = webSocket.getClients();
            for (auto &i : manager)
            {
                if (i.second.timeout > 0)
                {
                    i.second.timeout -= CLIENT_INTERVAL;
                    ClientID id = i.first;
                    webSocket.getWebSocketInstance().text(id, String(i.second.timeout));
                }

                if (i.second.timeout <= 0)
                {
                    digitalWrite(i.second.gpio, HIGH);
                    webSocket.getWebSocketInstance().close(i.first);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void WebServer::start()
{
    users = database.read(USERS_CFG_FILE);

    server.begin();

#if _PROD_MODE_
    // [firebase.listen()] is a synchronous
    // code and we need to execute it independently
    // in separate task . . .
    // running on CPU-1
    const int CORE_ID = 1;
    const int TASK_STACK_SIZE = 10000; // 10kb . . . .
    const int TASK_PRIORITY_LEVEL = 1;
    xTaskCreatePinnedToCore(
        firebaseListenerTask,
        "firebase-listener-task",
        TASK_STACK_SIZE,
        nullptr,
        TASK_PRIORITY_LEVEL,
        &firebaseTask,
        CORE_ID);
#endif
}

bool WebServer::isConnectedToInternet()
{
    return isInternetConnected;
}

void WebServer::setupDeviceNetworkModes()
{
    WiFi.mode(WIFI_AP_STA);

    Cfg deviceConfig = database.read(DEVICE_WIFI_CFG_FILE);
    auto deviceSsid = deviceConfig.find("device_ssid");
    auto devicePassword = deviceConfig.find("device_pwd");

    String deviceCurrentSsid = ESP_SSID;
    String deviceCurrentPassword = ESP_PWD;

    if (deviceSsid != deviceConfig.end() && devicePassword != deviceConfig.end())
    {
        String ssid = deviceSsid->second;
        String pwd = devicePassword->second;
        ssid.trim();
        pwd.trim();

        if (!ssid.isEmpty() && !pwd.isEmpty())
        {
            deviceCurrentSsid = ssid;
            deviceCurrentPassword = pwd;
        }
    }

    WiFi.softAP(deviceCurrentSsid.c_str(), deviceCurrentPassword.c_str());
    LOG("Access Point started. IP Address: ");
    LOGLN(WiFi.softAPIP());

#if _PROD_MODE_
    Cfg homeConfig = database.read(HOME_WIFI_CFG_FILE);

    auto homeSsid = homeConfig.find("home_ssid");
    auto homePwd = homeConfig.find("home_pwd");

    if ((homeSsid == homeConfig.end()) && (homePwd == homeConfig.end()))
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
    LOGLN("Establishing connection to the internet!");
    WiFi.begin(ssid.c_str(), pwd.c_str());
    while (WiFi.status() != WL_CONNECTED && timeout > 0)
    {
        LOG(".");
        delay(50);
        timeout--;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        LOGLN("\nInternet connection failed! Please check your ssid and password.");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP);
        return;
    }

#endif
    isInternetConnected = true;
    LOGLN("Connected to Internet!");
}

void WebServer::sendHTML(AsyncWebServerRequest *request, const String &filename)
{
    File file = LittleFS.open(filename.c_str(), FILE_MODE_READ);

    if (!file)
    {
        LOGLN("Failed to open " + filename);
        request->send(404, "text/plain", "File not found");
        return;
    }

    LOGLN("File size: " + String(file.size()));

    AsyncWebServerResponse *response = request->beginResponse(file, filename, "text/html");
    response->addHeader("Content-Length", String(file.size()));
    request->send(response);
    file.close();
}

String WebServer::createSession()
{
    UUID uuid;
    long seed = 9999999;
    uuid.seed(random(seed), random(seed));
    uuid.generate();
    return String(uuid.toCharArray());
}

bool WebServer::isAuthorized(AsyncWebServerRequest *request)
{
    if (!request->hasHeader("Cookie"))
    {
        LOGLN("Cookie Header not found!");
        return false;
    }

    String cookie = request->header("Cookie");
    if (cookie.indexOf("SESSIONID=" + sessionID) == -1)
    {
        LOGLN("Authentication Failed");
        return false;
    }

    LOGLN("Authentication Successful");
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

    server.on("/reboot", HTTP_GET, rebootDeviceHandler_GET);

    // for mobile / client side application . . .
    server.on("/api/connect", HTTP_GET, lockController_GET);
}

void WebServer::loginHandler_GET(AsyncWebServerRequest *request)
{
    sendHTML(request, "/index.html");
}

void WebServer::loginHandler_POST(AsyncWebServerRequest *request)
{
    sessionID = createSession();

    String usernameArgument = request->arg("username");
    String passwordArgument = request->arg("password");

    usernameArgument.trim();
    passwordArgument.trim();

    String currentPassword = ESP_LOCAL_WEB_AUTH_PWD;
    Cfg loginPassword = database.read(LOGIN_CFG_FILE);

    auto password = loginPassword.find("password");
    if (password != loginPassword.end())
    {
        String customPassword = password->second;
        customPassword.trim();

        if (!customPassword.isEmpty())
        {
            currentPassword = customPassword;
        }
    }

    if (usernameArgument == ESP_LOCAL_WEB_AUTH_USERNAME && passwordArgument == currentPassword)
    {
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Location", "/home-wifi");
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Set-Cookie", "SESSIONID=" + sessionID + "; Path=/; HttpOnly");
        request->send(response);
        return;
    }

    LOGLN("Login failed!");
    request->redirect("/");
}

void WebServer::usersHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/users.html");
}

void WebServer::usersHandler_POST(AsyncWebServerRequest *request)
{
    LOGLN("Users Configuration!");

    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }

    Cfg userEmails;
    const int totalArgs = request->args();
    for (int i = 0; i < totalArgs; i++)
    {
        String email = request->arg("email_" + String(i + 1));
        email.trim();

        if (!email.isEmpty())
        {
            LOGLN("Saved email: " + email);
            userEmails.insert(std::make_pair(String(_GPIO_PINS_.at(i)), email));
        }
    }

    bool saved = database.write(USERS_CFG_FILE, userEmails);
    if (!saved)
    {
        LOGLN("Failed to save users!");
        sendHTML(request, "/users.html");
        return;
    }

    LOGLN("Users have been saved!");
    sendHTML(request, "/restart.html");
    WiFi.disconnect(true);
    ESP.restart();
}

void WebServer::deviceWifiHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/device-wifi.html");
}

void WebServer::deviceWifiHandler_POST(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }

    String deviceSsid = request->arg("device_ssid");
    String devicePassword = request->arg("device_pwd");

    deviceSsid.trim();
    devicePassword.trim();

    Cfg deviceWifiConfig = {
        {"device_ssid", deviceSsid},
        {"device_pwd", devicePassword},
    };

    bool saved = database.write(DEVICE_WIFI_CFG_FILE, deviceWifiConfig);

    if (!saved)
    {
        LOGLN("Failed to save device wifi configuration");
        request->send(500, "text/plain", "Failed to save device wifi configuration");
        return;
    }

    LOGLN("Device Wifi configuration saved successfully");
    sendHTML(request, "/restart.html");
    WiFi.disconnect(true);
    ESP.restart();
}

void WebServer::homeWifiHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/home-wifi.html");
}

void WebServer::homeWifiHandler_POST(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }

    String homeSsid = request->arg("home_ssid");
    String homePwd = request->arg("home_pwd");

    homeSsid.trim();
    homePwd.trim();

    Cfg homeWifiConfig = {
        {"home_ssid", homeSsid},
        {"home_pwd", homePwd},
    };

    bool saved = database.write(HOME_WIFI_CFG_FILE, homeWifiConfig);

    if (!saved)
    {
        LOGLN("Failed to save home wifi configuration");
        request->send(500, "text/plain", "Failed to save home wifi configuration");
        return;
    }

    LOGLN("Home Wifi configuration saved successfully");
    sendHTML(request, "/restart.html");
    WiFi.disconnect(true);
    ESP.restart();
}

void WebServer::changePasswordHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/change-password.html");
}

void WebServer::changePasswordHandler_POST(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }

    String password = request->arg("password");
    password.trim();

    Cfg loginCfg = {{"password", password}};

    bool saved = database.write(LOGIN_CFG_FILE, loginCfg);

    if (!saved)
    {
        LOGLN("Failed to save login password!");
        request->send(500, "text/plain", "Failed to change password!");
        return;
    }

    LOGLN("Login password changed saved successfully");
    sendHTML(request, "/restart.html");
    WiFi.disconnect(true);
    ESP.restart();
}

void WebServer::rebootDeviceHandler_GET(AsyncWebServerRequest *request)
{
    if (!isAuthorized(request))
    {
        LOGLN("Not authorized!");
        request->redirect("/");
        return;
    }
    sendHTML(request, "/restart.html");
    WiFi.disconnect(true);
    ESP.restart();
}

void WebServer::lockController_GET(AsyncWebServerRequest *request)
{
#if _PROD_MODE_
    if (!isConnectedToInternet())
    {
        request->send(403, "text/plain", "Unable to connect. Please contact the admin to configure the system's network settings.");
        return;
    }

    if (!firebase.isOrganizationAuthorized())
    {
        lockAllLockers();
        request->send(403, "text/plain", "Locker access is restricted. Contact Developers for further details.");
        return;
    }
#endif

    char token[PAYLOAD_SIZE] = {0};
    strncpy(token, request->header("X-Authorization").c_str(), sizeof(token) - 1);

    int result = jwt.decodeJWT(token);
    char email[PAYLOAD_SIZE] = {0};

    switch (result)
    {
    case 0:
    {
        const char *payload = jwt.payload;
        const char *start = strstr(payload, "\"email\":\"");
        if (start)
        {
            start += 9;
            const char *end = strchr(start, '\"');
            if (end)
            {
                size_t emailLength = end - start;
                if (emailLength < sizeof(email))
                {
                    strncpy(email, start, emailLength);
                    email[emailLength] = '\0';
                }
                else
                {
                    request->send(400, "text/plain", "Invalid email format.");
                    return;
                }
            }
            else
            {
                request->send(400, "text/plain", "Invalid token format.");
                return;
            }
        }
        else
        {
            request->send(400, "text/plain", "Invalid token format.");
            return;
        }
    }
    break;

    case 2:
        request->send(401, "text/plain", "Unauthorized! Code - 2");
        return;

    case 3:
        request->send(401, "text/plain", "Unauthorized! Code - 3");
        return;

    default:
        return;
    }

    for (const auto &user : users)
    {
        if (strcmp(email, user.second.c_str()) == 0)
        {
            int gpio = user.first.toInt();
            const char *emailPtr = user.second.c_str();

            bool isNew = webSocket.addClient(gpio, emailPtr);

            if (isNew)
            {
                request->send(200, "text/plain", "Locker " + String(user.first) + " has been unlocked!");
                return;
            }

            request->send(409, "text/plain", "Websocket connection is already established!");
            return;
        }
    }

    request->send(403, "text/plain", "Access Denied. Please contact the admin to gain access.");
}

// #define HEADER_SIZE 50
// #define PAYLOAD_SIZE 256
// #define SIGNATURE_SIZE 50
// #define OUTPUT_SIZE 400

// char header[HEADER_SIZE];
// char payload[PAYLOAD_SIZE];
// char signature[SIGNATURE_SIZE];
// char out[OUTPUT_SIZE];
// char key[] = "6equj5";

// CustomJWT jwt(
//     key,
//     header, sizeof(header),
//     payload, sizeof(payload),
//     signature, sizeof(signature),
//     out, sizeof(out));

// void WebServer::lockController_GET(AsyncWebServerRequest *request)
// {
// #if _PROD_MODE_
//     if (!isConnectedToInternet())
//     {
//         request->send(403, "text/plain", "Unable to connect. Please contact the admin to configure the system's network settings.");
//         return;
//     }

//     if (!firebase.isOrganizationAuthorized())
//     {
//         lockAllLockers();
//         request->send(403, "text/plain", "Locker access is restricted. Contact CUSIT Makerspace R&D Lab for details.");
//         return;
//     }
// #endif

//     String email;
//     String token = request->header("X-Authorization");
//     Serial.println(token);
//     int result = jwt.decodeJWT(const_cast<char *>(token.c_str()));
//     switch (result)
//     {
//     case 0:
//     {
//         String decodedToken = String(jwt.payload);
//         String substring = decodedToken.substring(10);
//         substring.replace("\"}", "");
//         email = substring;
//         Serial.println(email);
//     }
//     break;

//     case 2:
//     {
//         Serial.println("Invalid JWT Code - Err Code: 2");
//         request->send(401, "text/plain", "Unauthorized!");
//         return;
//     }
//     break;

//     case 3:
//     {
//         Serial.println("Invalid JWT Signature - Err Code: 3");
//         request->send(401, "text/plain", "Unauthorized!");
//         return;
//     }
//     break;

//     default:
//         Serial.println("Memory Not Allocated - Err Code: 1");
//         return;
//     }

//     for (const auto &user : users)
//     {
//         if (email == user.second)
//         {
//             int gpio = user.first.toInt();
//             String email = user.second;

//             bool isNew = webSocket.addClient(gpio, email);
//             email.clear();

//             if (isNew)
//             {
//                 // on successful auth initiate websocket connection from client . . .
//                 request->send(200, "text/plain", "Locker " + String(user.first) + " has been unlocked!");
//                 return;
//             }

//             request->send(409, "text/plain", "Websocket connection is already established!");
//             return;
//         }
//     }

//     email.clear();
//     request->send(403, "text/plain", "Access Denied. Please contact the admin to gain access.");
//     return;
// }