#include "widgets.h"
#include "teams_widget.h"
#include "matrix_display.h"
#include "ms_graph_auth.h"
#include <ArduinoJson.h>

// Teams presence status icons
void drawPresenceIcon(int x, int y, uint16_t color) {
    // Draw a small colored circle to represent presence status
    matrix.fillCircle(x + 4, y + 4, 3, color);
}

// Function to fetch Teams presence data from Microsoft Graph API
void updateTeamsData() {
    // Check if we have a valid token or refresh if needed
    if (!isMsGraphTokenValid()) {
        if (!refreshMsGraphToken()) {
            Serial.println("Failed to refresh MS Graph token, can't update Teams data");
            return;
        }
    }

    WiFiSSLClient client;

    Serial.println("Fetching Teams presence data...");

    if (!client.connect("graph.microsoft.com", 443)) {
        Serial.println("Connection to Microsoft Graph failed");
        return;
    }

    // Send the HTTP request to fetch presence data
    client.println("GET /v1.0/me/presence HTTP/1.1");
    client.println("Host: graph.microsoft.com");
    client.println("Authorization: Bearer " + msGraphAccessToken);
    client.println("Connection: close");
    client.println();

    // Wait for the response
    while (client.connected() && !client.available()) delay(10);

    // Read and parse the response
    String response = "";
    while (client.available()) {
        char c = client.read();
        response += c;
    }
    client.stop();

    // Extract the JSON part from the response
    int jsonStart = response.indexOf("{");
    if (jsonStart == -1) {
        Serial.println("Invalid response format from Graph API");
        return;
    }

    String jsonResponse = response.substring(jsonStart);

    // Parse the JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Check if the response contains an error
    if (doc["error"].isNull() == false) {
        Serial.print("Graph API error: ");
        Serial.println(doc["error"]["message"].as<String>());
        return;
    }

    // Extract presence information
    if (!doc["availability"].isNull() && !doc["activity"].isNull()) {
        String availability = doc["availability"].as<String>();
        String activity = doc["activity"].as<String>();

        // Update the Teams data structure
        currentTeams.status = availability;
        currentTeams.details = activity;
        currentTeams.statusColor = getTeamsStatusColor(availability);
        currentTeams.lastUpdate = millis();

        Serial.print("Teams presence updated: ");
        Serial.print(availability);
        Serial.print(" - ");
        Serial.println(activity);
    } else {
        Serial.println("No presence data found in response");
    }
}

// Enhanced status color mapping
uint16_t getTeamsStatusColor(String status) {
    if (status.equalsIgnoreCase("Available")) return matrix.color565(0, 128, 0);       // Green
    if (status.equalsIgnoreCase("AvailableIdle")) return matrix.color565(0, 128, 0);   // Green
    if (status.equalsIgnoreCase("Busy")) return matrix.color565(255, 0, 0);           // Red
    if (status.equalsIgnoreCase("BusyIdle")) return matrix.color565(255, 0, 0);       // Red
    if (status.equalsIgnoreCase("DoNotDisturb")) return matrix.color565(255, 0, 0);    // Red
    if (status.equalsIgnoreCase("BeRightBack")) return matrix.color565(255, 165, 0);   // Orange
    if (status.equalsIgnoreCase("Away")) return matrix.color565(255, 255, 0);         // Yellow
    if (status.equalsIgnoreCase("OffWork")) return matrix.color565(128, 0, 128);       // Purple
    if (status.equalsIgnoreCase("Offline")) return matrix.color565(128, 128, 128);     // Gray
    if (status.equalsIgnoreCase("PresenceUnknown")) return matrix.color565(0, 0, 255);  // Blue

    // Default case - light blue for unknown status
    return matrix.color565(0, 175, 240);
}

// Enhanced Teams widget drawing
void drawTeamsWidget(int x, int y, int width, int height) {
    // Background based on status color but dimmed
    uint16_t bgColor = matrix.color565(
        ((currentTeams.statusColor >> 11) & 0x1F) >> 2, // Dimmed red
        ((currentTeams.statusColor >> 5) & 0x3F) >> 2, // Dimmed green
        (currentTeams.statusColor & 0x1F) >> 2         // Dimmed blue
    );

    // Clear widget area
    matrix.fillRect(x, y, width, height, matrix.color565(0, 0, 0));

    // Draw presence icon
    drawPresenceIcon(x + 2, y + 3, currentTeams.statusColor);

    // Show status text
    matrix.setCursor(x + 10, y + 8);
    matrix.setTextColor(currentTeams.statusColor);
    matrix.setTextSize(1);

    // Format status text for display
    String displayText = currentTeams.status;
    if (displayText.length() > 12) {
        displayText = displayText.substring(0, 12);
    }
    matrix.print(displayText);
}
