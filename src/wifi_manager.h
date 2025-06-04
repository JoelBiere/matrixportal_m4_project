#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "config.h"

// Function declarations
void initializeWiFi();
void connectToWiFi();
void scanNetworks();
void printWifiStatus();
String getWiFiStatusString(int status);

#endif