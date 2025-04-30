#ifndef FIREBASE_OPERATIONS_H
#define FIREBASE_OPERATIONS_H

#include <Firebase_ESP_Client.h>

class FirebaseOperations final {
public:
    FirebaseOperations() = default;

    [[nodiscard]] bool isOrganizationAuthorized() const {
        return _isAuthorized;
    }

    [[nodiscard]] bool isAuthenticated() const {
        return _isAuthenticated;
    }

    void authenticate();
    void listenForAuthorizationStatus();

private:
    bool _isAuthenticated{false};
    bool _isAuthorized{false};
    FirebaseData _data;
    FirebaseAuth _auth;
    FirebaseConfig _config;
};
#endif
