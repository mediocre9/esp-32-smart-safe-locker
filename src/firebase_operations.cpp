#include <Arduino.h>
#include "../includes/config.hpp"
#include "../includes/firebase_operations.hpp"

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

void FirebaseOperations::configure()
{
  Serial.println("Firebase is being started!");
  _config.api_key = FIREBASE_WEB_API_KEY;
  _config.database_url = FIREBASE_RTDB_REFERENCE_URL;

#if LOGIN_ESP_ON_FIREBASE
  _auth.user.email = ESP_FIREBASE_AUTH_EMAIL;
  _auth.user.password = ESP_FIREBASE_AUTH_PWD;
#endif

#if REGISTER_ESP_ON_FIREBASE
  if (Firebase.signUp(&_config, &_auth, ESP_FIREBASE_AUTH_EMAIL, ESP_FIREBASE_AUTH_PWD))
  {
    Serial.print("Account registered successfully!");
  }
  else
  {
    Serial.print(_config.signer.signupError.message.c_str());
  }
  _config.token_status_callback = tokenStatusCallback;
#endif

  Firebase.begin(&_config, &_auth);
  Firebase.reconnectWiFi(true);

#if LOGIN_ESP_ON_FIREBASE
  while ((_auth.token.uid) == "")
  {
    Serial.print('.');
  }
#endif
  Serial.println("Cloud: connected!");
}

void FirebaseOperations::listen()
{
  Firebase.RTDB.getBool(&_data, "/organizations/" + String(ORGANIZATION) + "/authorized");
  _isAuthorized = _data.boolData();
}

int FirebaseOperations::isAuthorized()
{
  return _isAuthorized;
}