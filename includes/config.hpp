#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <map>

// Cloud Configuration . . .
#define FIREBASE_WEB_API_KEY "-----Your Firebase API key-----"
#define FIREBASE_RTDB_REFERENCE_URL "-----Your Firebase RTDB Reference URL-----"
#define NODEMCU_CLOUD_EMAIL "-----ESP32 Firebase Auth Email-----"
#define NODEMCU_CLOUD_PWD "-----ESP32 Firebase Auth Password-----"
#define ORGANIZATION "------any organization------"

// Macro Flags For resgistering and signing the nodemcu on firebase cloud . . .
#define CREATE_NODEMCU_ACCOUNT false
#define SIGNIN_NODEMCU_ACCOUNT !CREATE_NODEMCU_ACCOUNT

// Soft AP Network Configuration . . . .
#define NODEMCU_SSID "SafeLock"
#define NODEMCU_PWD "1Orion_const"

// For authentication . . .
#define NODEMCU_AUTH_USERNAME "admin"
#define NODEMCU_AUTH_PWD "loop11147"

// Configuration Files . . ...
#define HOME_WIFI_CFG_FILE "/home-wifi.cfg"
#define DEVICE_WIFI_CFG_FILE "/device-wifi.cfg"
#define LOGIN_CFG_FILE "/login.cfg"
#define USERS_CFG_FILE "/users.cfg"
#define BAUD_RATE 9600

// Little FS File System Modes . . .
#define FILE_MODE_WRITE "w"
#define FILE_MODE_READ "r"
#define FILE_MODE_WRITE_READ "w+"
#define FILE_MODE_READ_WRITE "r+"

// For Prod/Dev Evironments. . ...
#define _PROD_MODE_ true

// relay pins defned and mapped for 12 lockers.. . .
#define GPIO4 4
#define GPIO13 13
#define GPIO18 18
#define GPIO19 19
#define GPIO21 21
#define GPIO22 22
#define GPIO23 23
#define GPIO25 25
#define GPIO26 26
#define GPIO27 27
#define GPIO32 32
#define GPIO33 33

const std::map<short, uint8_t> _GPIO_PINS_ = {
    {0, GPIO4},
    {1, GPIO13},
    {2, GPIO18},
    {3, GPIO19},
    {4, GPIO21},
    {5, GPIO22},
    {6, GPIO23},
    {7, GPIO25},
    {8, GPIO26},
    {9, GPIO27},
    {10, GPIO32},
    {11, GPIO33},
};

#endif