#include "../includes/NetworkManager.hpp"
#include "../includes/Config.hpp"

void WiFiNetworkManager::setupNetworks() {
    LOGLN("\nEstablishing networks . . . .");
    WiFi.mode(WIFI_AP_STA);

    auto deviceConfig = _getNetworkConfigs("device_ssid", "device_pwd", CfgFilePath::DEVICE);

    //  @todo Refactor this piece of section....
    if (!deviceConfig.has_value()) {
        WiFi.softAP(ESP_DEFAULT_SSID, ESP_DEFAULT_PWD, 1, 0, 10);
    } else {
        auto& [deviceSSID, devicePassword] = *deviceConfig;

        String AP_SSID = deviceSSID.isEmpty() ? ESP_DEFAULT_SSID : deviceSSID;
        String AP_PASSWORD = devicePassword.isEmpty() ? ESP_DEFAULT_PWD : devicePassword;

        WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str(), 1, 0, 10);
    }

    auto homeConfig = _getNetworkConfigs("home_ssid", "home_pwd", CfgFilePath::HOME);

    if (!homeConfig.has_value()) {
        LOGLN("\nInternet connection failed! Please provide correct ssid and password.");
        return;
    }

    auto& [homeSSID, homePassword] = *homeConfig;

    // if (!homeConfig.has_value()) {
    //     LOGLN("[home_wifi.cfg] file is either corrupted or has invalid keys!");
    //     return;
    // }

    // Try only once to connect to the internet, if it fails then end-user
    // will be informed via rest-api in [HttpServer::lockController_POST]
    // to provide correct wifi credentials from the web interface admin panel...
    int timeout = 50;
    LOGLN("Establishing connection to the internet!");
    WiFi.begin(homeSSID.c_str(), homePassword.c_str());
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        LOG(".");
        delay(50);
        timeout--;
    }

    if (WiFi.status() != WL_CONNECTED) {
        LOGLN("\nInternet connection failed! Please provide correct ssid and password.");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP);
    }
}

[[nodiscard]] auto WiFiNetworkManager::_getNetworkConfigs(const char* first, const char* second, const char* filename) -> std::optional<NetworkConfig> {
    Cfg config = _database.read(filename);

    Cfg::iterator firstIterator = config.find(first);
    Cfg::iterator secondIterator = config.find(second);

    if (firstIterator == config.end() || secondIterator == config.end()) {
        return std::nullopt;
    }

    auto& [_, ssid] = *firstIterator;
    auto& [__, password] = *secondIterator;

    if (ssid.isEmpty() || password.isEmpty()) {
        return std::nullopt;
    }

    return std::make_pair(ssid, password);
}

bool WiFiNetworkManager::isConnectedToInternet() {
    return WiFi.status() == WL_CONNECTED;
}