# Smart Safe Locker System Firmware

## Overview

This firmware lets users set up and control a Smart Safe Locker System using a NodeMCU device. Users connect to the NodeMCU hotspot, log in to a web page, and enter their home Wi-Fi details and authorized user emails. After saving these settings, the device restarts. Authorized users can then unlock their lockers using the Smart Link app, but they must first connect to the NodeMCU hotspot.

## Features

-   **Remote Setup and Control:** Configure the locker system from web pages.
-   **Wi-Fi Configuration:** Enter home Wi-Fi details via the web interface.
-   **User Authorization:** Add user emails to authorize them to unlock lockers.
-   **Internet Requirement:** The NodeMCU needs an internet connection to work fully.

## Usage

1. Connect to the NodeMCU hotspot after it starts.
2. Go to the web interface at the default IP address `192.168.4.1`.
3. Log in and set up the home Wi-Fi and authorized user emails.

    ```c++
    // config.hpp

    // NodeMCU SSID and Password
    #define NODEMCU_SSID "FAR-CRY"
    #define NODEMCU_PWD "1116equj5"

    // Login credentials
    #define NODEMCU_AUTH_USERNAME "admin"
    #define NODEMCU_AUTH_PWD "12345678"
    ```

4. Save the settings and let the device reboot.
5. After reboot, connect to the NodeMCU hotspot again. Users can then unlock their lockers with the Smart Link app.

### Libraries

-   LittleFS
-   [Firebase_ESP_Client](https://github.com/mobizt/Firebase-ESP-Client)
-   [ESP8266WebServer](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer)
-   [UUID](https://github.com/RobTillaart/UUID)
-   [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload) (for uploading data directory files to the NodeMCU)

### Note

-   Download and install the [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload) plugin to upload the data directory files to the NodeMCU.
-   Firebase is only used to allow or block the usage of the device.
