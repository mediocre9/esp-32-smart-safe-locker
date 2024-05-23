/*
* @TODO:
* Implement the Session mechanism for route protections .  .. 
*/


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <map>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


// Cloud Configuration . . .
#define FIREBASE_WEB_API_KEY ""
#define FIREBASE_RTDB_REFERENCE_URL ""
#define NODEMCU_EMAIL ""
#define NODEMUC_PWD ""
#define ORGANIZATION ""

// Macro Flags For resgistering/signing in the nodemcu on cloud . . .
#define CREATE_NODEMCU_ACCOUNT false
#define SIGIN_NODEMCU_ACCOUNT !CREATE_NODEMCU_ACCOUNT

// Soft AP Network Configuration . . . .
#define NODEMCU_SSID "FAR-CRY"
#define NODEMCU_PWD "1116equj5"

// Configuration Files . . ...
#define ROOT_CFG_FILE "/root.cfg"        // for Soft AP
#define NETWORK_CFG_FILE "/network.cfg"  // For Home Networks
#define USERS_CFG_FILE "/users.cfg"
#define BAUD_RATE 9600

// File System Modes . . .
#define FILE_MODE_WRITE "w"
#define FILE_MODE_READ "r"
#define FILE_MODE_WRITE_READ "w+"
#define FILE_MODE_READ_WRITE "r+"


template<typename T>
class Database {
public:
  virtual bool write(const String& filename, const T& data) = 0;
  virtual T read(const String& filename) = 0;
};

class CfgDatabase : public Database<std::map<String, String>> {
public:
  bool write(const String& filename, const std::map<String, String>& data) override {
    File file = LittleFS.open(filename.c_str(), FILE_MODE_WRITE);

    if (!file) {
      Serial.println("Failed to open " + filename);
      return false;
    }

    for (const auto& pair : data) {
      String value = pair.first + "=" + pair.second + "\n";
      file.print(value);
    }

    file.close();
    return true;
  }

  std::map<String, String> read(const String& filename) override {
    std::map<String, String> data;
    File file = LittleFS.open(filename.c_str(), FILE_MODE_READ);

    if (!file) {
      Serial.println("Failed to open " + filename);
      return data;
    }

    while (file.available()) {
      String line = file.readStringUntil('\n');
      int separatorIndex = line.indexOf('=');
      if (separatorIndex != -1) {
        String key = line.substring(0, separatorIndex);
        String value = line.substring(separatorIndex + 1);
        data[key] = value;
      }
    }

    file.close();
    return data;
  }
};


bool authorized = true;

class FirebaseOperations {
public:
  void configure() {
    Serial.println("Firebase is being started!");
    _config.api_key = FIREBASE_WEB_API_KEY;
    _config.database_url = FIREBASE_RTDB_REFERENCE_URL;

#if SIGIN_NODEMCU_ACCOUNT
    _auth.user.email = NODEMCU_EMAIL;
    _auth.user.password = NODEMCU_PWD;
#endif

#if CREATE_NODEMCU_ACCOUNT
    if (Firebase.signUp(&_config, &_auth, NODEMCU_EMAIL, NODEMCU_PWD)) {
      Serial.print("sign up ok");
    } else {
      Serial.print(_config.signer.signupError.message.c_str());
    }
    _config.token_status_callback = tokenStatusCallback;
#endif

    Firebase.begin(&_config, &_auth);
    Firebase.reconnectWiFi(true);

#if SIGIN_NODEMCU_ACCOUNT
    while ((_auth.token.uid) == "") {
      Serial.print('.');
      delay(50);
    }
#endif
    Serial.println("Cloud: connected!");
  }

  void listen() {
    // Serial.println("Listening .., ... .");
    if (Firebase.RTDB.getBool(&_data, "/organizations/" + String(ORGANIZATION) + "/authorized")) {
      if (_data.boolData()) {
        authorized = true;
        digitalWrite(BUILTIN_LED, LOW);
      } else {
        authorized = false;
        digitalWrite(BUILTIN_LED, HIGH);
      }
    }
  }

private:
  FirebaseData _data;
  FirebaseAuth _auth;
  FirebaseConfig _config;
};


FirebaseOperations firebase;
bool isInternetConnected = false;

const uint8_t SAFE_RELAY_PINS[] = {
  D1,  //5
  D2,  // 4
  D5,  //14
  D6,  // 12
  D7   // 13
};

class WebServer {
public:
  WebServer()
    : _server(80) {}

  void init() {
    this->users = _database.read(USERS_CFG_FILE);

    Serial.println("init");
    for (const auto& i : this->users) {
      Serial.println(i.first + " " + i.second);
    }
  }

  void setupRoutes() {
    _server.begin();

    _server.on("/", HTTP_GET, [this]() {
      sendHtml("/index.html");
    });

    _server.on("/connect", HTTP_POST, [this]() {
      String email = _server.arg("email");
      bool matched = false;
      String PIN_NO;
      for (const auto& i : this->users) {
        Serial.println(i.first + " " + i.second);
        if (email == i.second) {
          matched = true;
          Serial.println("Matched!");
          PIN_NO = i.first;
          break;
        }
      }

      if (!isInternetConnected) {
        Serial.println("No Internet Connection!");
        _server.send(503, "text/plain", "Access Denied! Please contact support.");
      }

      if (!authorized) {
        _server.send(403, "text/plain", "Access Denied! Please contact support.");
      }

      if (matched) {
        _server.send(200, "text/plain", PIN_NO);  // the user will send the pin to access its repective locker
      } else {
        _server.send(403, "text/plain", "You do not have permission to access it.");
      }
    });


    /*
    * @TODO: 
    * Implement the PIN Controll mechanism 'on client and 
    * and at Micocontroller Server Side 
    */
    _server.on("/unlock", HTTP_POST, [this]() {
      _server.send(200, "text/plain", "unlocked");
    });

    _server.on("/lock", HTTP_POST, [this]() {
      _server.send(200, "text/plain", "locked");
    });

    _server.on("/login", HTTP_POST, [this]() {
      String username = _server.arg("username");
      String password = _server.arg("password");

      username.trim();
      password.trim();

      std::map<String, String> rootConfig = _database.read(ROOT_CFG_FILE);
      auto rootUser = rootConfig.find("user");
      auto rootPwd = rootConfig.find("pwd");

      if (rootUser != rootConfig.end() && rootPwd != rootConfig.end()) {
        String storedUser = rootUser->second;
        String storedPwd = rootPwd->second;

        storedUser.trim();
        storedPwd.trim();

        Serial.println("Received Username: " + username);
        Serial.println("Received Password: " + password);
        Serial.println("Stored Username: " + storedUser);
        Serial.println("Stored Password: " + storedPwd);

        if (username == storedUser && password == storedPwd) {
          Serial.println("Login successful");
          sendHtml("/network-configuration.html");
        } else {
          Serial.println("Login failed");
          sendHtml("/index.html");
        }
      } else {
        Serial.println("User credentials not found in config");
        sendHtml("/index.html");
      }
    });

    _server.on("/network-configuration", HTTP_POST, [this]() {
      String networkSsid = _server.arg("network_ssid");
      String networkPwd = _server.arg("network_password");

      networkSsid.trim();
      networkPwd.trim();

      std::map<String, String> networkConfiguration{ { "ssid", networkSsid }, { "pwd", networkPwd } };
      bool saved = _database.write(NETWORK_CFG_FILE, networkConfiguration);

      if (saved) {
        Serial.println("Network configuration saved:");
        Serial.println("SSID: " + networkSsid);
        Serial.println("Password: " + networkPwd);
        sendHtml("/network-configuration.html");
        ESP.restart();
      } else {
        Serial.println("Failed to save network configuration.");
        sendHtml("/network-configuration.html");
      }
    });

    _server.on("/network-configuration.html", HTTP_GET, [this]() {
      sendHtml("/network-configuration.html");
    });

    _server.on("/users-configuration.html", HTTP_GET, [this]() {
      sendHtml("/users-configuration.html");
    });

    _server.on("/users-configuration", HTTP_POST, [this]() {
      const int SIZE = _server.args();

      std::map<String, String> userEmails;
      for (int i = 0; i < SIZE; i++) {
        String email = _server.arg("email" + String(i + 1));
        email.trim();
        Serial.println("Saved email:" + email);
        userEmails.insert(std::make_pair(String(SAFE_RELAY_PINS[i]), email));
      }

      bool saved = _database.write(USERS_CFG_FILE, userEmails);

      if (saved) {
        Serial.println("Users have beed saved!");
        sendHtml("/users-configuration.html");
        ESP.restart();
      } else {
        Serial.println("Failed to save users!");
        sendHtml("/users-configuration.html");
      }
    });
  }

  void
  requestHandler() {
    _server.handleClient();
  }

  void establishConnections() {
    WiFi.mode(WIFI_STA);
    WiFi.softAP(NODEMCU_SSID, NODEMCU_PWD);
    Serial.print("Access Point started. IP Address: ");
    Serial.println(WiFi.softAPIP());

    std::map<String, String> networkConfig = _database.read(NETWORK_CFG_FILE);

    auto networkSsid = networkConfig.find("ssid");
    auto networkPwd = networkConfig.find("pwd");

    if (networkSsid != networkConfig.end() && networkPwd != networkConfig.end()) {
      String ssid = networkSsid->second;
      String pwd = networkPwd->second;

      ssid.trim();
      pwd.trim();

      if (!ssid.isEmpty() && !pwd.isEmpty()) {
        Serial.println("Establishing connection to the internet!");
        WiFi.begin(ssid.c_str(), pwd.c_str());
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          delay(50);
        }
        Serial.println("Connected to Internet!");
        isInternetConnected = true;
      }
    }
  }

private:
  void sendHtml(const String& filename) {
    File file = LittleFS.open(filename.c_str(), FILE_MODE_READ);

    if (!file) {
      Serial.print("Failed to open " + filename);
      _server.send(404, "text/plain", "File not found");
      return;
    }

    _server.streamFile(file, "text/html");
    file.close();
  }

private:
  CfgDatabase _database;
  std::map<String, String> users;
  ESP8266WebServer _server;
};

WebServer webServer;


void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS!");
  } else {
    Serial.println("LittleFS mounted!");
  }

  webServer.init();
  webServer.establishConnections();
  webServer.setupRoutes();

  if (isInternetConnected) {
    firebase.configure();
  }
}

void loop() {
  webServer.requestHandler();
  if (isInternetConnected) {
    firebase.listen();
  }
}
