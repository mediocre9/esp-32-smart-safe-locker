#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <map>

// Cloud Configuration . . .
#define FIREBASE_WEB_API_KEY ""
#define FIREBASE_RTDB_REFERENCE_URL ""
#define NODEMCU_EMAIL ""
#define NODEMUC_PWD ""
#define ORGANIZATION ""

// Macro Flags For resgistering and signing the nodemcu on firebase cloud . . .
#define CREATE_NODEMCU_ACCOUNT false
#define SIGIN_NODEMCU_ACCOUNT !CREATE_NODEMCU_ACCOUNT

// Soft AP Network Configuration . . . .
#define NODEMCU_SSID "FAR-CRY"
#define NODEMCU_PWD "1116equj5"

// For authentication . . .
#define NODEMCU_AUTH_USERNAME "admin"
#define NODEMCU_AUTH_PWD "12345678"

// Configuration Files . . ...
#define NETWORK_CFG_FILE "/network.cfg" // For Home Networkss
#define USERS_CFG_FILE "/users.cfg"
#define BAUD_RATE 9600

// Little FS File System Modes . . .
#define FILE_MODE_WRITE "w"
#define FILE_MODE_READ "r"
#define FILE_MODE_WRITE_READ "w+"
#define FILE_MODE_READ_WRITE "r+"

// relay pins defned .. . .
const std::map<short, uint8_t> _GPIO_PINS_ = {
    {0, D0},
    {1, D1},
    {2, D2},
    {3, D3},
    {4, D4},
    {5, D5},
    {6, D6},
    {7, D7},
    {8, D8},
};

#endif