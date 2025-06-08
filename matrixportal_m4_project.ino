#include "config.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "matrix_display.h"
#include "widgets.h"

// Matrix configuration
uint8_t rgbPins[] = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20, 21};
uint8_t clockPin = 14;
uint8_t latchPin = 15;
uint8_t oePin = 16;

Adafruit_Protomatter matrix(
        WIDTH, 3, 1, rgbPins, 4, addrPins,
        clockPin, latchPin, oePin, true);

// NEW: Animation and display state variables
AnimationType currentAnimation = ANIMATION_TRUCK;

String displayText = "Hello Matrix!";
uint16_t currentColor = 0;
int scrollPosition = WIDTH;
uint32_t lastUpdate = 0;
int patternFrame = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("=== MatrixPortal M4 Project ===");

    // Initialize modules
    initializeMatrix();
    initializeWidgets();
    initializeWiFi();
    initializeWebServer();

    Serial.println("Setup complete!");
}

void loop() {

    if (!isWiFiConnected()) {
        // Handle WiFi reconnection (non-blocking, throttled internally)
        handleWiFiReconnection();
    } else {
        // If WiFi is connected, handle web clients
        handleWebClients();
    }
    // Update display - this handles its own timing internally
    updateMatrixDisplay();

}