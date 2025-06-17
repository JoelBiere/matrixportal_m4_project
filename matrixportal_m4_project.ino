#include "config.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "matrix_display.h"
#include "widgets.h"
#include "Arduino.h"
#include <FreeRTOS_SAMD51.h>

WiFiServer server(80);
int wifiStatus = WL_IDLE_STATUS;

// Animation and display state variables
AnimationType currentAnimation = ANIMATION_TRUCK;
String displayText = "Hello Matrix!";
uint16_t currentColor = 0;
int scrollPosition = WIDTH;
uint32_t lastUpdate = 0;
int patternFrame = 0;

// Forward declarations for FreeRTOS tasks
void displayTask(void *pvParameters);
void networkTask(void *pvParameters);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("=== MatrixPortal M4 FreeRTOS Project ===");

    // Initialize hardware first
    initializeMatrix();
    initializeWidgets();
    initializeWiFi();
    initializeWebServer();

    Serial.println("Hardware initialization complete!");

    // Create high-priority display task (never blocks)
    BaseType_t displayResult = xTaskCreate(
            displayTask,           // Function
            "Display",            // Name
            2048,                 // Stack size (words)
            NULL,                 // Parameters
            3,                    // Priority (higher = more important)
            NULL                  // Handle (we don't need it)
    );

    if (displayResult != pdPASS) {
        Serial.println("Failed to create display task!");
        while(1) delay(1000);
    }

    // Create lower-priority network task (can block for API calls)
    BaseType_t networkResult = xTaskCreate(
            networkTask,          // Function
            "Network",           // Name
            4096,                // Larger stack for network operations
            NULL,                // Parameters
            1,                   // Lower priority than display
            NULL                 // Handle (we don't need it)
    );

    if (networkResult != pdPASS) {
        Serial.println("Failed to create network task!");
        while(1) delay(1000);
    }

    Serial.println("FreeRTOS tasks created successfully!");
    Serial.print("Free heap before scheduler: ");
    Serial.println(xPortGetFreeHeapSize());

    // Start the FreeRTOS scheduler
    Serial.println("Starting FreeRTOS scheduler...");
    vTaskStartScheduler();

    // Should never reach here if scheduler starts successfully
    Serial.println("ERROR: FreeRTOS scheduler failed to start!");
    while(1) {
        Serial.println("Scheduler failure - system halted");
        delay(5000);
    }
}

void loop() {
    // Empty - FreeRTOS handles everything now
    // This should never execute once the scheduler starts
}

// ============================================================================
// HIGH-PRIORITY DISPLAY TASK - Maintains smooth 60 FPS animations
// ============================================================================
void displayTask(void *pvParameters) {
    Serial.println("Display task started!");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(16); // ~60 FPS (16ms)

    uint32_t frameCount = 0;
    uint32_t lastStatsReport = 0;

    while(1) {
        frameCount++;

        // Update display - this is always fast and never blocks
        updateMatrixDisplay();

        // Report performance stats every 5 seconds
        uint32_t now = millis();
        if (now - lastStatsReport > 5000) {
            float fps = frameCount / ((now - lastStatsReport) / 1000.0);
            Serial.printf("Display: %.1f FPS, Free heap: %d bytes\n",
                          fps, xPortGetFreeHeapSize());
            frameCount = 0;
            lastStatsReport = now;
        }

        // Wait for next frame (maintains consistent 60 FPS)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ============================================================================
// LOWER-PRIORITY NETWORK TASK - Handles all blocking network operations
// ============================================================================
void networkTask(void *pvParameters) {
    Serial.println("Network task started!");

    TickType_t lastWiFiCheck = 0;
    const TickType_t wifiCheckInterval = pdMS_TO_TICKS(10000); // 10 seconds

    while(1) {
        TickType_t now = xTaskGetTickCount();

        // WiFi connection maintenance (every 10 seconds)
        if ((now - lastWiFiCheck) > wifiCheckInterval) {
            if (!isWiFiConnected()) {
                Serial.println("Network: WiFi disconnected, attempting reconnection...");
                handleWiFiReconnection();
            }
            lastWiFiCheck = now;
        }

        // Only do network operations if WiFi is connected
        if (isWiFiConnected()) {
            // Use existing widget update logic - this handles all timing internally
            // The updateWidgets() function already:
            // - Checks if it's time to update each widget type
            // - Only updates the currently active widget
            // - Handles all the timing intervals (weather every 10 minutes, etc.)
            // - Updates the global data structures (currentWeather, currentSpotifyTrack, etc.)
            updateWidgets();

            // Handle web server requests (can block briefly)
            handleWebClients();
        }

        // Sleep for 500ms - updateWidgets() has its own timing logic
        // so we don't need to check as frequently
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ============================================================================
// FREERTOS ERROR HANDLING
// ============================================================================

//// This function is called if a FreeRTOS API call fails
//extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
//    Serial.print("STACK OVERFLOW in task: ");
//    Serial.println(pcTaskName);
//    while(1) {
//        digitalWrite(LED_BUILTIN, HIGH);
//        delay(100);
//        digitalWrite(LED_BUILTIN, LOW);
//        delay(100);
//    }
//}
//
//// This function is called if malloc fails
//extern "C" void vApplicationMallocFailedHook(void) {
//    Serial.println("MALLOC FAILED - Out of heap memory!");
//    Serial.print("Free heap: ");
//    Serial.println(xPortGetFreeHeapSize());
//    while(1) {
//        digitalWrite(LED_BUILTIN, HIGH);
//        delay(500);
//        digitalWrite(LED_BUILTIN, LOW);
//        delay(500);
//    }
//}
//
//// Idle task hook - called when no other tasks are running
//extern "C" void vApplicationIdleHook(void) {
//    // Optional: Put CPU in low power mode here
//    // For now, just yield to let other tasks run
//    __WFI(); // Wait for interrupt (low power)
//}