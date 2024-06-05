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

#if SIGIN_NODEMCU_ACCOUNT
  _auth.user.email = NODEMCU_EMAIL;
  _auth.user.password = NODEMUC_PWD;
#endif

#if CREATE_NODEMCU_ACCOUNT
  if (Firebase.signUp(&_config, &_auth, NODEMCU_EMAIL, NODEMUC_PWD))
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

#if SIGIN_NODEMCU_ACCOUNT
  while ((_auth.token.uid) == "")
  {
    Serial.print('.');
    delay(50);
  }
#endif
  Serial.println("Cloud: connected!");
}

void FirebaseOperations::listen()
{
  if (Firebase.RTDB.getBool(&_data, "/organizations/" + String(ORGANIZATION) + "/authorized"))
  {
    if (_data.boolData())
    {
      digitalWrite(BUILTIN_LED, LOW);
      _isAccessBlocked = true;
    }
    else
    {
      digitalWrite(BUILTIN_LED, HIGH);
      _isAccessBlocked = false;
    }
  }
}

bool FirebaseOperations::isAccessBlocked()
{
  return _isAccessBlocked;
}