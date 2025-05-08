#ifndef CONFIG_H
#define CONFIG_H

#include <WString.h>
#include <array>
#include "./Secret.hpp"

// Serial Communication
#define BAUD_RATE 9600
#define FIRMWARE_VERSION "0.2.0"

// Prod/Dev Mode Flags
#define PROD_MODE false
#define DEV_MODE !(PROD_MODE)
#define ENABLE_LOGGING !(PROD_MODE)
#define EXPERIMENTAL_FEATURE false

#if ENABLE_LOGGING
#define LOG(val) Serial.print(val)
#define LOGLN(val) Serial.println(val)
#else
#define LOG(val)
#define LOGLN(val)
#endif

// Flags for Registering and Signing up the ESP on Firebase Cloud
#define LOGIN_ESP_ON_FIREBASE true
#define REGISTER_ESP_ON_FIREBASE !(LOGIN_ESP_ON_FIREBASE)

#define FIREBASE_WEB_API_KEY (WEB_API_KEY_SECRET)
#define FIREBASE_RTDB_REFERENCE_URL (REFERENCE_URL_SECRET)

#if DEV_MODE
#define ESP_FIREBASE_AUTH_EMAIL (AUTH_DEV_EMAIL_SECRET)
#define ESP_FIREBASE_AUTH_PWD (AUTH_DEV_PASSWORD_SECRET)
#else
#define ESP_FIREBASE_AUTH_EMAIL (AUTH_PROD_EMAIL_SECRET)
#define ESP_FIREBASE_AUTH_PWD (AUTH_PROD_PASSWORD_SECRET)
#endif

// Network Configuration (defaults) . . . .
#define ESP_DEFAULT_SSID (AP_SSID_SECRET)
#define ESP_DEFAULT_PWD (AP_PASSWORD_SECRET)

// Local Web Interface Authentication (Defaults)
#define ESP_ADMIN_WEB_AUTH_USERNAME (LOGIN_USER_SECRET)
#define ESP_ADMIN_WEB_AUTH_PWD (LOGIN_PASSWORD_SECRET)

// for 12 Lockers...
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

const std::array<uint16_t, 12> GPIOS = {
    GPIO4, GPIO13, GPIO18,
    GPIO19, GPIO21, GPIO22,
    GPIO23, GPIO25, GPIO26,
    GPIO27, GPIO32, GPIO33};

enum StatusCode {
    OK_CODE = 200,
    FOUND = 302,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    CONFLICT = 409,
    TOO_MANY_REQUESTS = 429,
    INTERNAL_SERVER_ERROR = 500,
};

// LittleFS File System Modes...
struct LittleFSFileMode {
    static inline constexpr const char* WRITE = "w";
    static inline constexpr const char* READ = "r";
    static inline constexpr const char* WRITE_READ = "w+";
    static inline constexpr const char* READ_WRITE = "r+";
};

// Configuration Files path...
struct CfgFilePath {
    static inline constexpr const char* LOGIN = "/login.cfg";
    static inline constexpr const char* USERS = "/users.cfg";
    static inline constexpr const char* HOME = "/home_wifi.cfg";
    static inline constexpr const char* DEVICE = "/device_wifi.cfg";
};

// Endpoint protection Static API-Keys....
struct AuthKeys {
    static inline constexpr const char* ADMIN = ADMIN_API_KEY_SECRET;
    static inline constexpr const char* CLIENT = CLIENT_API_KEY_SECRET;
};

struct RouteFilePath {
    static inline constexpr const char* INDEX = "/index.html";
    static inline constexpr const char* USERS = "/users.html";
    static inline constexpr const char* HOME_WIFI = "/home-wifi.html";
    static inline constexpr const char* CHANGE_PASSWORD = "/change-password.html";
    static inline constexpr const char* DEVICE_WIFI = "/device-wifi.html";
    static inline constexpr const char* RESTART = "/restart.html";
    static inline constexpr const char* NOT_FOUND = "/404.html";
};

struct ContentType {
    static inline constexpr const char* PLAIN = "text/plain";
    static inline constexpr const char* HTML = "text/html";
};

struct ResponseMessage {
    static inline constexpr const char* EMPTY_BODY = "";
    static inline constexpr const char* NETWORK_CONFIGURTION_REQUIRED = "Unable to connect. Please contact the admin to configure the system's network settings.";
    static inline constexpr const char* LOCKER_ACCESS_RESTRICTED = "Locker access is restricted. Contact Developers for further details.";
    static inline constexpr const char* UNAUTHORIZED_LOCKER_ACCESS = "Access Denied. Please contact the admin to gain access.";
    static inline constexpr const char* WEBSOCKET_CONNECTION_EXISTS = "Websocket connection is already established!";
    static inline constexpr const char* INVALID_API_KEY = "Invalid API-Key.";
    static inline constexpr const char* API_KEY_NOT_FOUND = "API-Key was not provided!";
    static inline constexpr const char* ALLOWED_CONTENT_TYPE = "Allowed Content-Type is text/plain";
    static inline constexpr const char* INVALID_URI = "Invalid URI!";
    static inline constexpr const char* EMAIL_QUERY_PARAM_REQUIRED = "email query param is required!";
    static inline constexpr const char* TOKEN_QUERY_PARAM_REQUIRED = "token query param is required!";
    static inline constexpr const char* NOT_FOUND = "404! Not Found";
    static inline constexpr const char* USER_NOT_FOUND = "User not Found";
    static inline constexpr const char* TOO_MANY_REQUESTS = "Too many requests. Try again Later!";
    static inline constexpr const char* REBOOT = "Rebooted!";
};

/**
 * @brief Defines the organization for the firmware authorization.
 *
 * The firmware can be blocked by setting the authorized field to false
 * in the organization's data on Firebase. This allows control over the
 * authorization level for security purposes.
 *
 * Current General Schema:
 * <your-firebase-RTDB-reference-url>:
 *      > organizations
 *        >>   org-1
 *              >>>  address: String
 *              >>>  authorized: Boolean (Defaults to True)
 *              >>>  deviceId: String
 *              >>>  contact: Number | String
 *        >>   org-2
 *              >>>  address: String
 *              >>>  deviceId: String
 *              >>>  authorized: Boolean (Defaults to True)
 *              >>>  contact: Number | String
 */
#define ORGANIZATION "cusit"
#define RTDB_PATH "/organizations/" + String(ORGANIZATION) + "/authorized"

#endif
