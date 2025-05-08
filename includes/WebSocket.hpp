#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>

#include "Config.hpp"
#include "Database.hpp"

using ClientID = uint32_t;

struct ClientInfo {
    uint16_t gpio;
    uint16_t timeout;
    String email;
};

using ClientManager = std::map<ClientID, ClientInfo>;

class WebSocket final {
public:
    WebSocket() = delete;
    explicit WebSocket(const String& channel);
    void init();
    [[nodiscard]] bool addClient(uint16_t gpio, const String& email);
    void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t length);

    [[nodiscard]] AsyncWebSocket& getWebSocketInstance() {
        return _webSocket;
    }

    [[nodiscard]] ClientManager& getClients() {
        return _clients;
    }

private:
    String _email;
    uint16_t _gpio;
    ClientManager _clients;
    AsyncWebSocket _webSocket;
};

extern WebSocket webSocket;

#endif