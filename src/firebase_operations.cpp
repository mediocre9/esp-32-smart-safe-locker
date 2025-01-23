
#include <Arduino.h>
#include "../includes/config.hpp"
#include "../includes/firebase_operations.hpp"

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

void FirebaseOperations::startAuthentication()
{
  LOGLN("Firebase is being started!");
  _config.api_key = FIREBASE_WEB_API_KEY;
  _config.database_url = FIREBASE_RTDB_REFERENCE_URL;

#if LOGIN_ESP_ON_FIREBASE
  _auth.user.email = ESP_FIREBASE_AUTH_EMAIL;
  _auth.user.password = ESP_FIREBASE_AUTH_PWD;
#endif

#if REGISTER_ESP_ON_FIREBASE
  if (Firebase.signUp(&_config, &_auth, ESP_FIREBASE_AUTH_EMAIL, ESP_FIREBASE_AUTH_PWD))
  {
    LOG("Device registered successfully!");
  }
  else
  {
    LOG(_config.signer.signupError.message.c_str());
  }
  _config.token_status_callback = tokenStatusCallback;
#endif

  Firebase.begin(&_config, &_auth);
  Firebase.reconnectWiFi(true);

#if LOGIN_ESP_ON_FIREBASE
  while ((_auth.token.uid) == "")
  {
    LOG('.');
  }
#endif
  LOGLN("Logged In!");
  _isAuthenticated = true;
}

bool FirebaseOperations::isAuthenticated()
{
  return _isAuthenticated;
}

void FirebaseOperations::listenForAuthorizationStatus()
{
  Firebase.RTDB.getBool(&_data, "/organizations/" + String(ORGANIZATION) + "/authorized");
  _isAuthorized = _data.boolData();
}

int FirebaseOperations::isOrganizationAuthorized()
{
  return _isAuthorized;
}