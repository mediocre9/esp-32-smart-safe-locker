
#ifndef FIREBASE_OPERATIONS_H
#define FIREBASE_OPERATIONS_H

#include <Firebase_ESP_Client.h>

class FirebaseOperations
{
public:
    int isOrganizationAuthorized();
    bool isAuthenticated();
    void startAuthentication();
    void listenForAuthorizationStatus();

private:
    bool _isAuthenticated;
    int _isAuthorized;
    FirebaseData _data;
    FirebaseAuth _auth;
    FirebaseConfig _config;
};

#endif
