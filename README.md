# Multi Smart Safe Locker System

A multi smart locker management system powered by ESP32, enabling secure and convenient access control for individuals, organizations, and homeowners. Users can configure and manage their lockers via a web interface, authorize access using email, and unlock lockers through the Smart Link mobile app.

## Table of Contents

-   [Features](#features)

-   [Technical Features](#technical-features)

-   [Setup](#setup)

-   [Firebase Project Setup](#firebase-project-setup)

-   [Firmware Configuration](#firmware-configuration)

-   [Uploading HTML Files](#uploading-html-files)

-   [Usage](#usage)

-   [Adding Authorized Users](#adding-authorized-users)

-   [Smart Link Integration](#smart-link-integration)

-   [Web Interface Previews](#web-interface-previews)

-   [Response When Interacting with the System via Smart Link Mobile App](#response-when-interacting-with-the-system-via-smart-link-mobile-app)

-   [Libraries](#libraries)

## Features

-   **Remote Setup and Control**: Manage locker settings through an ESP32-hosted web interface.

-   **Wi-Fi Configuration**: Admins must provide home Wi-Fi SSID and password via the web interface for internet connectivity.

-   **User Authorization**: Grant locker access by adding authorized user emails locally.

-   **Individual Auto-Lock Timers**: 20-second auto-lock intervals for each user.

-   **WebSocket Notifications**: Real-time notifications alert users when their locker is about to lock.

-   **Secure Locking System**: Email-based access for enhanced security.

-   **Firebase Integration for Firmware Blocking**: Firebase is used solely to allow developers to restrict firmware functionality if necessary.

## Technical Features

### Core System

-   **Microcontroller**: ESP32 with Wi-Fi and Bluetooth capabilities.

-   **Cloud Integration**: Firebase Realtime Database is only used for firmware blocking and developer controls.

### Authentication and Security

-   **Email-Based Authorization**: Users are granted access by the admin through the web interface by adding their Gmail ID, which they use to sign in to the Smart Link app.

-   **Fingerprint Verification**: The fingerprint is used as a guard wall to send the unlock request but is not processed or stored.

### Lock Management

-   **Individual Timers**: Each user's locker has a separate auto-lock timeout of 20 seconds.

-   **WebSocket Updates**: Users are notified in real-time when their locker is about to lock.

### Web Interface

-   **Hosted by ESP32**: Provides a web UI for system setup and management.

-   **Customizable Settings**: Modify Wi-Fi credentials, user authorizations, and system parameters.

### System Messages

-   If the system is not connected to the internet, end users will receive:

`Unable to connect. Please contact the admin to configure the system's network settings.`

-   If the end user has not been granted access, they will receive:

`Access Denied. Please contact the admin to gain access.`

-   If developers have blocked the firmware (which even admins cannot override), users interacting with the system will see:

`Locker access is restricted. Contact Developers for further details.`

## Setup

### Firebase Project Setup

1. Create a Firebase project and obtain **Firebase Web API Key**

and **Realtime Database (RTDB) URL**.

2. Add these to the [config.hpp](https://github.com/mediocre9/esp-32-smart-safe-locker/blob/main/includes/config.hpp) file:

```cpp

#define  FIREBASE_WEB_API_KEY  "Your Firebase API Key"

#define  FIREBASE_RTDB_REFERENCE_URL  "Your Firebase RTDB URL"
```

### Firmware Configuration

Set `REGISTER_ESP_ON_FIREBASE` to true in [config.hpp](https://github.com/mediocre9/esp-32-smart-safe-locker/blob/main/includes/config.hpp) for initial device registration.

Compile and upload the firmware to the ESP32.

After registration, set `REGISTER_ESP_ON_FIREBASE` back to `false` to enable login mode.

#### Uploading HTML Files

1. Install the `arduino-littlefs-upload plugin`.

2. Place the web interface HTML files in the **data** directory.

3. Use the plugin to upload these files to the ESP32's LittleFS.

### Usage

1. Connect to the ESP32 Hotspot

2. Access the web interface at http://192.168.4.1.

3. Log in and configure the system.

4. Enter Wi-Fi credentials.

5. Add authorized users' emails.

Admins grant locker access by adding users' Gmail IDs through the web interface. The Gmail ID must match the one users use to sign in to the [Smart Link](https://github.com/mediocre9/smart-link) app.

#### Smart Link Integration

1. Download the [Smart Link](https://github.com/mediocre9/smart-link) mobile app.

2. Sign in with a Google account.

3. Admins must add the same Google account email to the ESP32 for authorization.

Users must authenticate with a fingerprint before sending an unlock request. Note that the fingerprint verification serves only as a guard wall and is not processed or stored.

### Web Interface Previews

<img  src="previews/1.png"  width="80%">
<img  src="previews/2.png"  width="80%">
<img  src="previews/3.png"  width="80%">
<img  src="previews/4.png"  width="80%">
<img  src="previews/5.png"  width="80%">

### <ins> Note </ins>:

-   The firmware requires internet connectivity to function. Admins must provide home wifi `SSID` and `password` via the web interface to connect the system to the internet.

-   If incorrect wifi credentials are provided, users will not be able to access the lockers, and the system will display the message:
    `Unable to connect. Please contact the admin to configure the system's network settings.`

-   Firebase is solely used to give developers control over the firmware, such as disabling functionality if necessary.

#### Response When Interacting with the System via Smart Link Mobile App

When users attempt to interact with the Smart Safe Locker System through the [Smart Link](https://github.com/mediocre9/smart-link) mobile app, the following responses may occur based on the system’s status:

#### 1. Access Denied:

If a user has not been authorized by the admin, they will receive:
`Access Denied. Please contact the admin to gain access.`

#### 2. Firmware Restrictions:

If the firmware has been blocked by developers, users will see the message:
`Locker access is restricted. Contact Developers for further details.`

#### 3. Network Connection Issues:

If the system is not connected to the internet, users will receive:
`Unable to connect. Please contact the admin to configure the system's network settings.`

### Libraries:

-   LittleFS
-   [Firebase_ESP_Client](https://github.com/mobizt/Firebase-ESP-Client)
-   [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
-   [UUID](https://github.com/RobTillaart/UUID)
-   [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload)
-   [CustomJWT](https://github.com/Ant2000/CustomJWT)
