#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <map>
#include <optional>

#include "Config.hpp"
#include "Database.hpp"

using NetworkConfig = std::pair<String, String>;

class WiFiNetworkManager final {
public:
    WiFiNetworkManager() = default;
    void setupNetworks();
    [[nodiscard]] bool isConnectedToInternet();

private:
    [[nodiscard]] auto _getNetworkConfigs(const char* first, const char* second, const char* filename) -> std::optional<NetworkConfig>;

private:
    CfgDatabase _database;
};

#endif