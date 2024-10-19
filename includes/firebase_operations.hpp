#ifndef FIREBASE_OPERATIONS_H
#define FIREBASE_OPERATIONS_H

#include <Firebase_ESP_Client.h>

class FirebaseOperations
{
public:
    FirebaseOperations()
        : _isAuthorized(1) {}

    int isAuthorized();
    void configure();
    void listen();

private:
    int _isAuthorized;
    FirebaseData _data;
    FirebaseAuth _auth;
    FirebaseConfig _config;
};

#endif
