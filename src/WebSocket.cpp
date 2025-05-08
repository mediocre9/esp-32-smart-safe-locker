#include "../includes/WebSocket.hpp"

WebSocket::WebSocket(const String& channel) : _webSocket(channel) {}

// Please refer to the docs of [ESP32Async/ESPAsyncWebServer]:
// https://github.com/ESP32Async/ESPAsyncWebServer/wiki#async-websocket-event
void WebSocket::init() {
    _webSocket.onEvent([this](AsyncWebSocket* server,
                              AsyncWebSocketClient* client,
                              AwsEventType type,
                              void* arg,
                              uint8_t* data,
                              size_t length)

                       {
                           this->onEvent(server,
                                         client,
                                         type,
                                         arg,
                                         data,
                                         length);
                       });
}

bool WebSocket::addClient(uint16_t gpio, const String& email) {
    ClientManager::iterator it = std::find_if(
        _clients.begin(),
        _clients.end(),
        [&email](std::pair<ClientID, ClientInfo> client) -> bool {
            return client.second.email == email;
        });

    if (it != _clients.end()) {
        return false;
    }

    _gpio = gpio;
    _email = email;
    return true;
}

void WebSocket::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t length) {
    switch (type) {
        case WS_EVT_CONNECT: {
            digitalWrite(_gpio, LOW);
            uint16_t timeout = 10 * 1000;  // 10 seconds....
            auto clientInfo = ClientInfo{_gpio, timeout, _email.c_str()};
            _clients.emplace(client->id(), clientInfo);

            _gpio = 0;
            _email.clear();
        } break;

        case WS_EVT_DISCONNECT: {
            digitalWrite(_clients.at(client->id()).gpio, HIGH);
            client->close();
            _clients.erase(client->id());
        } break;

        case WS_EVT_ERROR: {
            client->close();
            _clients.erase(client->id());
        } break;

        default: {
            client->close();
            _clients.erase(client->id());
        } break;
    }
}