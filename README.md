# Smart Safe Locker System

> The firmware enables individuals, organizations, or homeowners to configure and control a Smart Safe Locker System using an ESP32. They can connect to the ESP32 hotspot and access a web page to authorize users via email. Authorized users can then unlock their respective lockers using the [Smart Link](https://github.com/mediocre9/smart-link) app.

## #Features

-   **Remote Setup and Control:** Configure the locker via web pages accessed through the ESP32 hotspot.
-   **Wi-Fi Configuration:** Enter home Wi-Fi details through the web interface.
-   **User Authorization:** Add authorized user emails for locker access.
-   **Performance:** Uses [ESP8266WebServer](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer) for efficient handling.
-   **Internet Required:** Needs an internet connection for full functionality.
-   **Security:** Secure login for web interface access.
-   **Customizable Login and Hotspot:** Change the web interface login password and ESP32 hotspot SSID and password.
-   **Firebase Integration:** Allows developers to block entire organizations, preventing users within those organizations from accessing their lockers.

## #Setup

### 1. Firebase Project Setup

1. Create a project on Firebase.
2. Get the Firebase Web API key and Realtime Database (RTDB) URL.
3. Add these to the `config.hpp` file.

```c++
// Cloud Configuration . . .
#define FIREBASE_WEB_API_KEY "Your Firebase API key"
#define FIREBASE_RTDB_REFERENCE_URL "Your Firebase RTDB Reference URL"
```

### 2. Firmware Configuration

To register the ESP32 with Firebase authentication, set `CREATE_NODEMCU_ACCOUNT` to `true` in [config.hpp](https://github.com/mediocre9/nodemcu-esp8266/blob/main/includes/config.hpp). Compile and run the ESP32 firmware to complete registration. Afterward, set `CREATE_NODEMCU_ACCOUNT` back to `false`.

### 3. Uploading HTML Files

To serve web pages from the ESP32:

1. Install the [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload) library.
2. Place HTML files in the data directory.
3. Upload the data directory to ESP32.

## #Usage

1. Connect to the ESP32 hotspot.
2. Open a web browser and go to `192.168.4.1`.
3. Login and enter your home Wifi SSID and password.
4. Save & Reboot.

-   #### Adding Authorized Users

1. Reconnect to the hotspot again after the restart.
2. Open a web browser and go to `192.168.4.1`.
3. Login and enter user emails.
4. Save and Reboot.

-   #### Smart Link
    Users have to download the Smart Link app and sign in with their Google account. Then Admins need to add the email that the user used to create their Smart Link account in Esp32 for authorization. Initially, users must connect to the Smart Lock network. Each time they want to open their locker, they must authenticate themselves with their phone's fingerprint. If successful, their respective locker will be unlocked.

### <ins>Note:</ins>

-   Admin (Organizations, homeowners or individuals) can manage smart lock access by authorizing users by adding their email. The Firebase feature allows developers to block entire organizations, preventing both users within those organizations from accessing their lockers. Admins can still manage local ESP32 settings, such as changing ESP32 hotspot SSIDs, login passwords etc.

-   If admin enters incorrect home wifi details, the authorised users won't be able to access the locker system.

### Libraries:

-   LittleFS
-   [Firebase_ESP_Client](https://github.com/mobizt/Firebase-ESP-Client)
-   [ESP8266WebServer](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer)
-   [UUID](https://github.com/RobTillaart/UUID)
-   [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload)
