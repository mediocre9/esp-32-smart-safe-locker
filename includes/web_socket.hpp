#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include "database.hpp"
#include "config.hpp"
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <map>

using ClientID = uint32_t;

struct ClientInfo
{
    uint8_t gpio;
    String userEmail;
    unsigned long timeout;
};

using ClientManager = std::map<ClientID, ClientInfo>;

class WebSocket
{
public:
    WebSocket() = delete;
    explicit WebSocket(String channel);
    void init();
    bool addClient(const int &gpio, const String &email);
    ClientManager &getClients();
    AsyncWebSocket &getWebSocketInstance();
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t length);

private:
    ClientManager _clients;
    AsyncWebSocket _webSocket;
    String _userEmail;
    int _gpio;
};

extern WebSocket webSocket;

#endif
