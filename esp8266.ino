#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* NETWORK_NAME = "It & Robotics";
const char* PASSWORD = "1116equj5";
const char* ON_SIGNAL_VALUE = "1";
const char* OFF_SIGNAL_VALUE = "0";
const String CLIENT_EMAIL = " "; 
unsigned BAUD_RATE = 9600;
short PORT = 80;

class SmartLocker {
public: 
    SmartLocker(): _server(PORT) {}

    void init() {
        pinMode(BUILTIN_LED, OUTPUT);
        digitalWrite(BUILTIN_LED, 1);
        delay(1000);
        Serial.begin(BAUD_RATE);

        if (!WiFi.softAP(NETWORK_NAME, PASSWORD)) {
            Serial.print("Failed to connect!");
            return;
        }

        Serial.println("===============================================");
        Serial.print("IP: ");
        Serial.print(WiFi.softAPIP());
    }

    void setupRoutes() {
        _server.on("/", HTTP_POST, [this]() {
          String payload = _server.arg("plain");
          if (payload != CLIENT_EMAIL) {
             _server.send(403, "text/plain", "You do not have permission to open this locker.");
             Serial.println("Resource Forbidden: 403");
             Serial.println(payload);
             return;
          } 
          _server.send(200, "text/plain", "Connected");
        });

        _server.on("/on_signal", HTTP_GET, [this]() {
            digitalWrite(BUILTIN_LED, 0);
            _server.send(200, "text/plain", ON_SIGNAL_VALUE);
            Serial.println("Bit Status: 1");
        });

        _server.on("/off_signal", HTTP_GET, [this]() {
            digitalWrite(BUILTIN_LED, 1);
            _server.send(200, "text/plain", OFF_SIGNAL_VALUE);
            Serial.println("Bit Status: 0");
        });

        _server.begin();
    }

    void requestHandler() {
        _server.handleClient();
    }

private: 
    ESP8266WebServer _server;
};

SmartLocker sm;

void setup() {
    sm.init();
    sm.setupRoutes();
}

void loop() {
    sm.requestHandler();
}


