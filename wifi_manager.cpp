#include "wifi_manager.h"
#include "matrix_display.h"
#include "credentials.h"

// WiFi status tracking
uint32_t lastWiFiCheck = 0;
uint32_t lastReconnectAttempt = 0;
bool reconnectionInProgress = false;

void initializeWiFi() {
    Serial.println("Initializing WiFi...");

    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("WiFi module not found!");
        return;
    }

    Serial.print("Firmware: ");
    Serial.println(WiFi.firmwareVersion());

    scanNetworks();

    connectToWiFi(ssid, wifiPass);
    if (wifiStatus == WL_CONNECTED) {
        printWiFiStatus();
        showMatrixIPAddress();
        Serial.println("Web server started successfully");
    }
    // Check if a second SSID is defined
    else if (ssid2[0] != '\0') {
        Serial.println("Primary WiFi connection failed, trying backup SSID...");
        connectToWiFi(ssid2, wifiPass);
        if (wifiStatus == WL_CONNECTED) {
            printWiFiStatus();
            showMatrixIPAddress();
            Serial.println("Web server started successfully");
        }
    } else {
        Serial.println("WiFi connection failed - no web server");
    }

    lastWiFiCheck = millis();
}

void connectToWiFi(char *network, char *wifiPass) {
    Serial.print("Connecting to: ");
    Serial.println(network);
    wifiStatus = WL_IDLE_STATUS;
    int attempts = 0;

    while (wifiStatus != WL_CONNECTED && attempts < 3) {
        attempts++;
        Serial.print("Attempt ");
        Serial.println(attempts);

        wifiStatus = WiFi.begin(network, wifiPass);

        for (int i = 0; i < 20 && wifiStatus != WL_CONNECTED; i++) {
            delay(500);
            wifiStatus = WiFi.status();
            Serial.print(".");
        }

        if (wifiStatus == WL_CONNECTED) {
            Serial.println("\nConnected!");
            reconnectionInProgress = false;
            break;
        } else {
            Serial.println("\nConnection failed: " + getWiFiStatusString(wifiStatus));
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
    Serial.println("Handling WiFi reconnection...");
    uint32_t now = millis();

    // Check WiFi status every 10 seconds
    if (now - lastWiFiCheck > 10000) {
        int currentStatus = WiFi.status();

        // Update our tracked status
        if (currentStatus != wifiStatus) {
            Serial.println("WiFi status changed from " + getWiFiStatusString(wifiStatus) +
                           " to " + getWiFiStatusString(currentStatus));
            wifiStatus = currentStatus;
        }

        lastWiFiCheck = now;

        // If we're not connected and not already trying to reconnect
        if (wifiStatus != WL_CONNECTED && !reconnectionInProgress) {
            // Don't attempt reconnection too frequently (wait at least 30 seconds between attempts)
            if (now - lastReconnectAttempt > 30000) {
                Serial.println("WiFi disconnected, attempting reconnection...");
                attemptReconnection();
                lastReconnectAttempt = now;
            }
        }
    }
}

void attemptReconnection() {
    reconnectionInProgress = true;
    Serial.println("Starting WiFi reconnection process...");

    // Try to disconnect cleanly first
    WiFi.disconnect();
    delay(1000);

    // Try primary SSID first
    Serial.println("Attempting to reconnect to primary network...");
    connectToWiFi(ssid, wifiPass);

    // If primary fails and we have a backup SSID, try that
    if (wifiStatus != WL_CONNECTED && ssid2[0] != '\0') {
        Serial.println("Primary failed, trying backup network...");
        connectToWiFi(ssid2, wifiPass);
    }

    if (wifiStatus == WL_CONNECTED) {
        Serial.println("WiFi reconnection successful!");
        printWiFiStatus();

        // Restart the web server
//        server.begin();
        Serial.println("Web server restarted");

        // Optionally show IP on matrix briefly
        showReconnectionSuccess();
    } else {
        Serial.println("WiFi reconnection failed - will retry later");
    }

    reconnectionInProgress = false;
}

void showReconnectionSuccess() {
    // Briefly flash a reconnection indicator on the matrix
    matrix.fillRect(0, 0, 64, 8, matrix.color565(0, 64, 0)); // Green bar
    matrix.setCursor(2, 1);
    matrix.setTextColor(matrix.color565(255, 255, 255));
    matrix.setTextSize(1);
    matrix.print("WiFi OK");
    matrix.show();
    delay(2000);

    // Clear the indicator
    matrix.fillRect(0, 0, 64, 8, 0);
    matrix.show();
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