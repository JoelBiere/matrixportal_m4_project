#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFiNINA.h>

// WiFi status and server
extern int wifiStatus;
extern WiFiServer server;

extern uint32_t lastWiFiCheck;
extern uint32_t lastReconnectAttempt;
extern bool reconnectionInProgress;

// WiFi management functions
void initializeWiFi();
void connectToWiFi(char *ssid, char *pass);
void printWiFiStatus();
bool isWiFiConnected();
void handleWiFiReconnection();
void attemptReconnection();
void showReconnectionSuccess();
void scanNetworks();
String getWiFiStatusString(int status);

#endif