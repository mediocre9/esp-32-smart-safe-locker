#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* NETWORK_NAME     = "It & Robotics - (Node-MCU)";
const char* PASSWORD         = "1116equj5";
const char* ON_SIGNAL_VAL    = "1";
const char* OFF_SIGNAL_VAL   = "0";
unsigned BAUD_RATE = 9600;
short PORT = 80;

void initialize();
void routes();
void hasClientConnected();
void onSignal();
void offSignal();

ESP8266WebServer server(PORT);


void setup() {
  initialize();
  routes();
}

void loop() {
  server.handleClient();
}

void initialize() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, 1);
  delay(1000);
  Serial.begin(BAUD_RATE);
  WiFi.softAP(NETWORK_NAME, PASSWORD);
  
  Serial.println("===============================================");
  Serial.print("IP: ");
  Serial.print(WiFi.softAPIP());
}


void routes() {
  server.on("/", hasClientConnected);
  server.on("/on_signal", onSignal);
  server.on("/off_signal", offSignal);
  server.begin();
}


void hasClientConnected(){
  server.send(200, "text/plain", "Connected");
}


void onSignal() {
  digitalWrite(BUILTIN_LED, 0);
  server.send(200, "text/plain", ON_SIGNAL_VAL);
  Serial.println("Bit Status: 1");
}

void offSignal() {
  digitalWrite(BUILTIN_LED, 1);
  server.send(200, "text/plain", OFF_SIGNAL_VAL);
  Serial.println("Bit Status: 0");
}



