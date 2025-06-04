#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFiNINA.h>

// WiFi credentials - move these to a separate credentials file
extern char ssid[];
extern char pass[];
extern int wifiStatus;

// WiFi management functions
void initializeWiFi();
void connectToWiFi();
void printWiFiStatus();
bool isWiFiConnected();
void handleWiFiReconnection();
void scanNetworks();

#endif