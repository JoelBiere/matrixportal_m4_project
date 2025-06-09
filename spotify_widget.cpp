#include "widgets.h"
#include "credentials.h"
#include <WiFiNINA.h>
#include "wifi_manager.h"
#include <ArduinoJson.h>

// Spotify authentication state
static String spotifyAccessToken = "";
static String spotifyRefreshToken = "";
static uint32_t tokenExpiry = 0;
static bool authenticationComplete = false;

// Forward declarations
bool refreshSpotifyToken();
bool fetchCurrentlyPlaying();
void parseSpotifyResponse(String jsonString);

// Simple Base64 encoding for Basic Auth
String base64Encode(String input) {
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    String encoded = "";
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (int j = 0; j < input.length(); j++) {
        char_array_3[i++] = input[j];
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                encoded += chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; j < i + 1; j++)
            encoded += chars[char_array_4[j]];

        while (i++ < 3)
            encoded += '=';
    }

    return encoded;
}

void updateSpotifyData() {
    Serial.println("Updating Spotify data...");

    if (!isWiFiConnected()) {
        Serial.println("WiFi not connected - skipping Spotify update");
        return;
    }

    // Check if we need to refresh the access token
    if (millis() > tokenExpiry && spotifyRefreshToken.length() > 0) {
        if (!refreshSpotifyToken()) {
            Serial.println("Failed to refresh Spotify token");
            currentSpotifyTrack.dataValid = false;
            return;
        }
    }

    // If we don't have a valid token, we can't fetch data
    if (spotifyAccessToken.length() == 0) {
        Serial.println("No Spotify access token available");
        currentSpotifyTrack.dataValid = false;
        return;
    }

    bool success = fetchCurrentlyPlaying();

    if (!success) {
        Serial.println("Failed to fetch currently playing track");
        // Keep old data but mark as potentially stale
        if (millis() - currentSpotifyTrack.lastUpdate > 300000) { // 5 minutes
            currentSpotifyTrack.dataValid = false;
        }
    }

    lastSpotifyUpdate = millis();
}

bool refreshSpotifyToken() {
    if (spotifyRefreshToken.length() == 0) {
        Serial.println("No refresh token available");
        return false;
    }

    WiFiSSLClient client; // Use SSL client
    if (!client.connect("accounts.spotify.com", 443)) { // HTTPS port
        Serial.println("Failed to connect to Spotify accounts");
        return false;
    }

    // Prepare Basic Auth header
    String credentials = String(spotifyClientId) + ":" + String(spotifyClientSecret);
    String basicAuth = base64Encode(credentials);

    // Prepare the refresh token request
    String postData = "grant_type=refresh_token&refresh_token=" + spotifyRefreshToken;

    client.println("POST /api/token HTTP/1.1");
    client.println("Host: accounts.spotify.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Authorization: Basic " + basicAuth);
    client.println("Content-Length: " + String(postData.length()));
    client.println("Connection: close");
    client.println();
    client.println(postData);

    // Wait for response - NON-BLOCKING
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 15000) { // Longer timeout
            Serial.println("Token refresh timeout");
            client.stop();
            return false;
        }
        // REMOVED delay(100); - this was blocking!
    }

    // Skip headers
    while (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) break;
    }

    // Read response
    String response = "";
    while (client.available()) {
        char c = client.read();
        if (c >= 32 && c <= 126) {
            response += c;
        }
    }
    client.stop();

    // Parse the token response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (!error && doc["access_token"]) {
        spotifyAccessToken = doc["access_token"].as<String>();
        tokenExpiry = millis() + (doc["expires_in"].as<int>() * 1000);
        Serial.println("Spotify token refreshed successfully");
        return true;
    }

    Serial.println("Failed to refresh Spotify token");
    return false;
}

bool fetchCurrentlyPlaying() {
    WiFiSSLClient client; // Use SSL client

    if (!client.connect("api.spotify.com", 443)) { // HTTPS port
        Serial.println("Failed to connect to Spotify API");
        return false;
    }

    client.println("GET /v1/me/player/currently-playing HTTP/1.1");
    client.println("Host: api.spotify.com");
    client.println("Authorization: Bearer " + spotifyAccessToken);
    client.println("Connection: close");
    client.println();

    // Wait for response - NON-BLOCKING
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 15000) { // Longer timeout
            Serial.println("Spotify API timeout");
            client.stop();
            return false;
        }
        // REMOVED delay(100); - this was blocking!
    }

    // Read status line
    String statusLine = client.readStringUntil('\n');
    Serial.println("API Status: " + statusLine);

    // Skip remaining headers
    while (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) break;
    }

    // Read JSON response
    String response = "";
    while (client.available()) {
        char c = client.read();
        if (c >= 32 && c <= 126) {
            response += c;
        }
    }
    client.stop();

    if (response.length() == 0) {
        Serial.println("No currently playing track");
        currentSpotifyTrack.isPlaying = false;
        currentSpotifyTrack.trackName = "No Track";
        currentSpotifyTrack.artistName = "Paused";
        currentSpotifyTrack.dataValid = true;
        return true;
    }

    parseSpotifyResponse(response);
    return true;
}


void parseSpotifyResponse(String jsonString) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        Serial.print("Spotify JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Extract track information
    JsonObject item = doc["item"];
    if (item.isNull()) {
        Serial.println("No track item in response");
        return;
    }

    currentSpotifyTrack.trackName = item["name"].as<String>();
    currentSpotifyTrack.albumName = item["album"]["name"].as<String>();
    currentSpotifyTrack.durationMs = item["duration_ms"].as<int>();
    currentSpotifyTrack.progressMs = doc["progress_ms"].as<int>();
    currentSpotifyTrack.isPlaying = doc["is_playing"].as<bool>();

    // Extract artist name (first artist)
    JsonArray artists = item["artists"];
    if (artists.size() > 0) {
        currentSpotifyTrack.artistName = artists[0]["name"].as<String>();
    }

    // Extract device info
    JsonObject device = doc["device"];
    if (!device.isNull()) {
        currentSpotifyTrack.deviceName = device["name"].as<String>();
    }

    currentSpotifyTrack.lastUpdate = millis();
    currentSpotifyTrack.dataValid = true;

    Serial.println("Spotify updated: " + currentSpotifyTrack.trackName + " by " + currentSpotifyTrack.artistName);
}

void drawSpotifyWidget(int x, int y, int width, int height) {
    // Update scroll animation every 200ms (slightly slower)
    if (millis() - lastSpotifyScroll > 200) {
        scrollSpotifyText(currentSpotifyTrack.trackName, spotifyTitleScroll, WIDTH); // Leave room for logo
        scrollSpotifyText(currentSpotifyTrack.artistName, spotifyArtistScroll, WIDTH);
        lastSpotifyScroll = millis();
    }

    // Update progress every second
    if (currentSpotifyTrack.isPlaying && millis() - lastSpotifyProgress > 1000) {
        currentSpotifyTrack.progressMs += 1000;
        lastSpotifyProgress = millis();
    }

    if (!currentSpotifyTrack.dataValid) {
        // Show loading state with Spotify green
        matrix.fillRect(x, y, width, height, matrix.color565(0, 20, 10)); // Dark green
        matrix.setCursor(x + 2, y + 4);
        matrix.setTextColor(matrix.color565(30, 215, 96)); // Spotify green
        matrix.setTextSize(1);
        matrix.print("Spotify...");
        return;
    }

    // Clean background - dark but not black
    uint16_t bgColor = currentSpotifyTrack.isPlaying ?
                       matrix.color565(5, 15, 5) :      // Very dark green if playing
                       matrix.color565(15, 10, 5);      // Dark warm color if paused

    matrix.fillRect(x, y, width, height, bgColor);

    // Draw Spotify logo on the right side
//    drawSpotifyLogo(x + width - 14, y + 1);

    // Play/pause indicator - make it more prominent
    uint16_t statusColor = currentSpotifyTrack.isPlaying ?
                           matrix.color565(30, 215, 96) :   // Spotify green
                           matrix.color565(255, 100, 100);  // Light red for paused

    if (currentSpotifyTrack.isPlaying) {
        // Animated playing bars instead of just a triangle
        drawPlayingBars(x + 1, y + 3, statusColor);
    } else {
        // Pause symbol - two vertical bars
        matrix.fillRect(x + 1, y + 4, 2, 6, statusColor);
        matrix.fillRect(x + 4, y + 4, 2, 6, statusColor);
    }

    // Track name - better positioned and colored
    matrix.setCursor(x + 8, y + 1);
    matrix.setTextColor(matrix.color565(255, 255, 255)); // Pure white
    matrix.setTextSize(1);

    String displayTitle = currentSpotifyTrack.trackName;
    if (displayTitle.length() > 8) {
        int start = spotifyTitleScroll;
        int end = min(start + 8, (int)displayTitle.length());
        displayTitle = displayTitle.substring(start, end);
    }
    matrix.print(displayTitle);

    // Artist name - smaller and dimmer
    matrix.setCursor(x + 8, y + 9);
    matrix.setTextColor(matrix.color565(180, 180, 180)); // Light gray

    String displayArtist = currentSpotifyTrack.artistName;
    if (displayArtist.length() > 8) {
        int start = spotifyArtistScroll;
        int end = min(start + 8, (int)displayArtist.length());
        displayArtist = displayArtist.substring(start, end);
    }
    matrix.print(displayArtist);

    // Progress bar at bottom - make it more prominent
    if (currentSpotifyTrack.durationMs > 0) {
        drawSpotifyProgressBar(x, 0, width, currentSpotifyTrack.progressMs, currentSpotifyTrack.durationMs);
    }
}

// New function to draw a recognizable Spotify logo
void drawSpotifyLogo(int x, int y) {
    uint16_t spotifyGreen = matrix.color565(30, 215, 96); // Official Spotify green
    uint16_t darkGreen = matrix.color565(15, 107, 48);    // Darker shade

    // Draw a circular background (simplified)
    matrix.fillRect(x, y + 1, 12, 10, matrix.color565(0, 0, 0)); // Black background
    matrix.drawRect(x, y + 1, 12, 10, darkGreen); // Border

    // Draw simplified sound waves (Spotify-ish curves)
    // Top curve
    matrix.drawLine(x + 2, y + 3, x + 9, y + 3, spotifyGreen);
    matrix.drawPixel(x + 1, y + 4, spotifyGreen);
    matrix.drawPixel(x + 10, y + 4, spotifyGreen);

    // Middle curve
    matrix.drawLine(x + 3, y + 6, x + 8, y + 6, spotifyGreen);
    matrix.drawPixel(x + 2, y + 7, spotifyGreen);
    matrix.drawPixel(x + 9, y + 7, spotifyGreen);

    // Bottom curve
    matrix.drawLine(x + 4, y + 9, x + 7, y + 9, spotifyGreen);
}

// New function for animated playing bars
void drawPlayingBars(int x, int y, uint16_t color) {
    static uint8_t barFrame = 0;
    static uint32_t lastBarUpdate = 0;

    // Only update bars every 100ms for smooth animation
    if (millis() - lastBarUpdate > 100) {
        barFrame = (barFrame + 1) % 8; // Cycle through 8 frames
        lastBarUpdate = millis();
    }

    // Draw 3 animated bars of different heights
    int bar1Height = 3 + (barFrame % 3);           // Height 3-5
    int bar2Height = 4 + ((barFrame + 2) % 4);     // Height 4-7
    int bar3Height = 2 + ((barFrame + 4) % 3);     // Height 2-4

    // Clear the area first
    matrix.fillRect(x, y, 6, 8, matrix.color565(0, 0, 0));

    // Draw bars from bottom up
    matrix.fillRect(x, y + 8 - bar1Height, 1, bar1Height, color);
    matrix.fillRect(x + 2, y + 8 - bar2Height, 1, bar2Height, color);
    matrix.fillRect(x + 4, y + 8 - bar3Height, 1, bar3Height, color);
}

// Improved progress bar with Spotify styling
void drawSpotifyProgressBar(int x, int y, int width, int progress, int total) {
    if (total <= 0) return;

    int progressWidth = (progress * width) / total;
    progressWidth = constrain(progressWidth, 0, width);

    // Background track - dark gray
    matrix.drawLine(x, y, x + width - 1, y, matrix.color565(40, 40, 40));

    // Progress - Spotify green
    if (progressWidth > 0) {
        matrix.drawLine(x, y, x + progressWidth - 1, y, matrix.color565(30, 215, 96));
    }

    // Add a small progress indicator dot if there's progress
    if (progressWidth > 2 && progressWidth < width - 1) {
        matrix.drawPixel(x + progressWidth, y, matrix.color565(255, 255, 255));
    }
}

void scrollSpotifyText(String& text, int& scrollPos, int maxChars) {
    if (text.length() <= maxChars) {
        scrollPos = 0;
        return;
    }

    static uint32_t lastPauseTime = 0;
    static bool isPausing = false;
    static uint8_t pauseDirection = 0; // 0 = not pausing, 1 = pausing at start, 2 = pausing at end

    // Check if we should be pausing
    if (isPausing) {
        if (millis() - lastPauseTime > 2000) { // 2 second pause
            isPausing = false;
            pauseDirection = 0;
        } else {
            return; // Still pausing, don't change scroll position
        }
    }

    if (spotifyScrollDirection) {
        scrollPos++;
        if (scrollPos + maxChars >= text.length()) {
            spotifyScrollDirection = false;
            isPausing = true;
            pauseDirection = 2; // Pausing at end
            lastPauseTime = millis();
        }
    } else {
        scrollPos--;
        if (scrollPos <= 0) {
            scrollPos = 0;
            spotifyScrollDirection = true;
            isPausing = true;
            pauseDirection = 1; // Pausing at start
            lastPauseTime = millis();
        }
    }
}

// Manual token setting function (for initial setup)
void setSpotifyTokens(String accessToken, String refreshToken) {
    spotifyAccessToken = accessToken;
    spotifyRefreshToken = refreshToken;
    tokenExpiry = millis() + 3600000; // 1 hour from now
    authenticationComplete = true;
    Serial.println("Spotify tokens set manually");
}

// Replace the getSpotifyAuthURL function in spotify_widget.cpp with this:

// Replace the getSpotifyAuthURL function in spotify_widget.cpp:

String getSpotifyAuthURL() {
    String authURL = "https://accounts.spotify.com/authorize?";
    authURL += "client_id=" + String(spotifyClientId);
    authURL += "&response_type=code";
    authURL += "&redirect_uri=https://spotify.com";  // Fake redirect - matches app settings
    authURL += "&scope=user-read-currently-playing,user-read-playback-state";
    authURL += "&show_dialog=true";  // Force consent screen every time

    return authURL;
}


bool exchangeCodeForTokens(String authCode) {
    WiFiSSLClient client; // Use WiFiSSLClient for HTTPS

    Serial.println("Connecting to accounts.spotify.com...");

    if (!client.connect("accounts.spotify.com", 443)) { // HTTPS port 443
        Serial.println("Failed to connect to Spotify accounts");
        return false;
    }

    Serial.println("Connected! Preparing token exchange...");

    // Prepare Basic Auth header
    String credentials = String(spotifyClientId) + ":" + String(spotifyClientSecret);
    String basicAuth = base64Encode(credentials);

    // Prepare POST data
    String postData = "grant_type=authorization_code";
    postData += "&code=" + authCode;
    postData += "&redirect_uri=https://spotify.com";

    Serial.println("Sending token request...");

    // Send request
    client.println("POST /api/token HTTP/1.1");
    client.println("Host: accounts.spotify.com");
    client.println("Authorization: Basic " + basicAuth);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println("Connection: close");
    client.println();
    client.println(postData);

    Serial.println("Waiting for response...");

    // Wait for response with longer timeout - NON-BLOCKING
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 15000) { // Increased timeout to 15 seconds
            Serial.println("Token exchange timeout");
            client.stop();
            return false;
        }
        // REMOVED delay(100); - this was blocking the animations!
    }

    Serial.println("Response received, reading headers...");

    // Read status line
    String statusLine = client.readStringUntil('\n');
    Serial.println("Status: " + statusLine);

    // Skip remaining headers
    while (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) break;
    }

    Serial.println("Reading JSON response...");

    // Read JSON response
    String response = "";
    while (client.available()) {
        char c = client.read();
        if (c >= 32 && c <= 126) {
            response += c;
        }
    }
    client.stop();

    Serial.println("Raw response length: " + String(response.length()));
    if (response.length() > 0) {
        Serial.println("Response preview: " + response.substring(0, min(200, (int)response.length())));
    }

    if (response.length() == 0) {
        Serial.println("Empty response from Spotify");
        return false;
    }

    // Parse response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }

    if (doc["access_token"]) {
        spotifyAccessToken = doc["access_token"].as<String>();
        if (doc["refresh_token"]) {
            spotifyRefreshToken = doc["refresh_token"].as<String>();
        }
        tokenExpiry = millis() + (doc["expires_in"].as<int>() * 1000);

        Serial.println("Spotify tokens obtained successfully!");
        Serial.println("Access token length: " + String(spotifyAccessToken.length()));

        // Immediately try to fetch current track
        currentSpotifyTrack.dataValid = false;
        lastSpotifyUpdate = 0;
        updateSpotifyData();

        return true;
    } else {
        Serial.println("No access_token in response");
        if (doc["error"]) {
            Serial.println("Error: " + doc["error"].as<String>());
            if (doc["error_description"]) {
                Serial.println("Description: " + doc["error_description"].as<String>());
            }
        }
        return false;
    }
}

