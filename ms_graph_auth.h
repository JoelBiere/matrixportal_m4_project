#ifndef MS_GRAPH_AUTH_H
#define MS_GRAPH_AUTH_H

#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

// Microsoft Graph API configuration
extern String msGraphAccessToken;
extern String msGraphRefreshToken;
extern unsigned long msGraphTokenExpiry;

// Function declarations
String getMsGraphAuthURL();
bool exchangeMsGraphCodeForTokens(String authCode);
bool refreshMsGraphToken();
bool isMsGraphTokenValid();
void setMsGraphTokens(String accessToken, String refreshToken, unsigned long expiryTime);

#endif
