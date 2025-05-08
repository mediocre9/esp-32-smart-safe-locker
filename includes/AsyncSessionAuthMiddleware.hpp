#ifndef ASYNC_COOKIE_AUTH_MIDDLEWARE_H
#define ASYNC_COOKIE_AUTH_MIDDLEWARE_H

#include <ESPAsyncWebServer.h>
#include <UUID.h>
#include <optional>

#include "Config.hpp"

class SessionIdGenerator final {
public:
    SessionIdGenerator() = default;

    const String& getId() const {
        return _id;
    }

    void generate() {
        UUID uuid;
        long seed = 9999999;
        uuid.seed(random(seed), random(seed));
        uuid.generate();
        _id = String(uuid.toCharArray());
    }

private:
    String _id;
};

class AsyncSessionAuthMiddleware final : public AsyncMiddleware {
public:
    AsyncSessionAuthMiddleware() = delete;

    explicit AsyncSessionAuthMiddleware(const SessionIdGenerator& auth)
        : _auth(auth) {}

    void run(AsyncWebServerRequest* request, ArMiddlewareNext next) {
        if (!_isAuthenticated(request)) {
            LOGLN("Not authorized!");
            request->redirect("/");
            return;
        }
        next();
    }

private:
    [[nodiscard]] bool _isAuthenticated(AsyncWebServerRequest* request) {
        if (!request->hasHeader("Cookie")) {
            LOGLN("Cookie Header not found!");
            return false;
        }

        String cookie = request->header("Cookie");
        String id = _auth.getId();
        if (cookie.indexOf("COOKIE_ID=" + id) == -1) {
            LOGLN("Authentication Failed");
            return false;
        }

        LOGLN("Authentication Successful");
        return true;
    }

private:
    SessionIdGenerator _auth;
};

#endif