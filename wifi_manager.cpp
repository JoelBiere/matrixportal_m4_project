#include "wifi_manager.h"
#include "matrix_display.h"
#include "credentials.h"

void initializeWiFi() {
  Serial.println("Initializing WiFi...");
  
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("WiFi module not found!");
    return;
  }

  Serial.print("Firmware: ");
  Serial.println(WiFi.firmwareVersion());
  
  scanNetworks();
  connectToWiFi();
  
  if (wifiStatus == WL_CONNECTED) {
    // server.begin(); // should now be handled in web_server.cpp
    printWiFiStatus();
    showMatrixIPAddress();
    Serial.println("Web server started successfully");
  } else {
    Serial.println("WiFi connection failed - no web server");
  }
}

void connectToWiFi() {
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  wifiStatus = WL_IDLE_STATUS;
  int attempts = 0;
  
  while (wifiStatus != WL_CONNECTED && attempts < 3) {
    attempts++;
    Serial.print("Attempt ");
    Serial.println(attempts);
    
    wifiStatus = WiFi.begin(ssid, pass);
    
    for (int i = 0; i < 20 && wifiStatus != WL_CONNECTED; i++) {
      delay(500);
      wifiStatus = WiFi.status();
      Serial.print(".");
    }
    Serial.println();
    
    if (wifiStatus == WL_CONNECTED) {
      Serial.println("Connected!");
      break;
    }
  }
}

void scanNetworks() {
  Serial.println("Scanning networks...");
  int numNetworks = WiFi.scanNetworks();
  
  for (int i = 0; i < numNetworks; i++) {
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(" dBm)");
  }
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

bool isWiFiConnected() {
  return wifiStatus == WL_CONNECTED;
}

void handleWiFiReconnection() {
  if (wifiStatus != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting reconnection...");
    connectToWiFi();
  }
}

String getWiFiStatusString(int status) {
  switch(status) {
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL (Network not found)";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED (Wrong password?)";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    case WL_NO_MODULE: return "WL_NO_MODULE (Hardware issue)";
    default: return "UNKNOWN_STATUS (" + String(status) + ")";
  }
}