#include "../includes/web_socket.hpp"

WebSocket::WebSocket(String channel) : _webSocket(channel) {}

void WebSocket::init()
{
    _webSocket.onEvent([this](AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                              AwsEventType type,
                              void *arg,
                              uint8_t *data,
                              size_t length)

                       { this->onEvent(server,
                                       client,
                                       type,
                                       arg,
                                       data,
                                       length); });
}

bool WebSocket::addClient(const int &gpio, const String &email)
{
    for (auto &client : _clients)
    {
        if (email == client.second.userEmail)
        {
            return false;
        }
    }

    this->_gpio = gpio;
    this->_userEmail = email;
    return true;
}

AsyncWebSocket &WebSocket::getWebSocketInstance()
{
    return _webSocket;
}

void WebSocket::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t length)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
    {
        digitalWrite(this->_gpio, LOW);
        auto clientInfo = std::make_pair(client->id(), ClientInfo{this->_gpio, this->_userEmail.c_str(), 20 * 1000});
        _clients.insert(clientInfo);

        this->_gpio = 0;
        this->_userEmail.clear();
    }
    break;

    case WS_EVT_DISCONNECT:
    {
        digitalWrite(_clients.at(client->id()).gpio, HIGH);
        client->close();
        _clients.erase(client->id());
    }
    break;

    case WS_EVT_ERROR:
        _clients.clear();
        break;

    default:
        break;
    }
}

ClientManager &WebSocket::getClients()
{
    return _clients;
}
