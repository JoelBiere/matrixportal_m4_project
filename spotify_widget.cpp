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
bool fetchCurrentlyPlayingFast();
void parseSpotifyResponseFast(String jsonString);

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
    static uint32_t lastLightUpdate = 0; // For progress-only updates

    if (!isWiFiConnected()) {
        Serial.println("WiFi not connected - skipping Spotify update");
        return;
    }

    uint32_t now = millis();

    // Do light updates (just progress) every 10 seconds when playing
    if (currentSpotifyTrack.isPlaying && currentSpotifyTrack.dataValid) {
        if (now - lastLightUpdate > 10000) {
            // Just increment progress without network call
            currentSpotifyTrack.progressMs += (now - lastLightUpdate);
            lastLightUpdate = now;
            Serial.println("Light Spotify update (progress only)");
            return;
        }
    }

    // Full network update every 30 seconds or when no data
    if (now - lastSpotifyUpdate < 30000 && currentSpotifyTrack.dataValid) {
        return; // Skip this update
    }

    Serial.println("Full Spotify network update...");

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

    bool success = fetchCurrentlyPlayingFast();

    if (!success) {
        Serial.println("Failed to fetch currently playing track");
        // Keep old data but mark as potentially stale
        if (millis() - currentSpotifyTrack.lastUpdate > 300000) { // 5 minutes
            currentSpotifyTrack.dataValid = false;
        }
    }

    lastSpotifyUpdate = millis();
    lastLightUpdate = millis();
}

bool refreshSpotifyToken() {
    if (spotifyRefreshToken.length() == 0) {
        Serial.println("No refresh token available");
        return false;
    }

    WiFiSSLClient client;
    if (!client.connect("accounts.spotify.com", 443)) {
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

    // Wait for response with timeout
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println("Token refresh timeout");
            client.stop();
            return false;
        }
    }

    // Skip headers
    while (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) break;
    }

    // Read response
    String response = "";
    timeout = millis();
    while (client.available() && (millis() - timeout < 2000)) {
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

bool fetchCurrentlyPlayingFast() {
    WiFiSSLClient client;
    client.setTimeout(2000); // Set socket timeout to 2 seconds

    if (!client.connect("api.spotify.com", 443)) {
        Serial.println("Failed to connect to Spotify API");
        return false;
    }

    // Send request quickly
    client.print("GET /v1/me/player/currently-playing HTTP/1.1\r\n");
    client.print("Host: api.spotify.com\r\n");
    client.print("Authorization: Bearer ");
    client.print(spotifyAccessToken);
    client.print("\r\nConnection: close\r\n\r\n");

    // Very short wait for response start
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 1500) { // Only 1.5 seconds
            Serial.println("Spotify API timeout");
            client.stop();
            return false;
        }
    }

    // Skip headers ultra-fast
    bool headersComplete = false;
    while (client.available() && !headersComplete) {
        String line = client.readStringUntil('\n');
        if (line.length() <= 1) headersComplete = true; // Empty line = end of headers
    }

    // Read response with strict timeout
    String response = "";
    timeout = millis();
    while (client.available() && (millis() - timeout < 1000)) { // Only 1 second for JSON
        response += (char)client.read();
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

    // Quick JSON validation
    response.trim();
    if (response.startsWith("{") && response.endsWith("}")) {
        parseSpotifyResponseFast(response);
        return true;
    }

    Serial.println("Invalid JSON received");
    return false;
}

void parseSpotifyResponseFast(String jsonString) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }

    JsonObject item = doc["item"];
    if (!item.isNull()) {
        currentSpotifyTrack.trackName = item["name"].as<String>();
        currentSpotifyTrack.durationMs = item["duration_ms"].as<int>();
        currentSpotifyTrack.progressMs = doc["progress_ms"].as<int>();
        currentSpotifyTrack.isPlaying = doc["is_playing"].as<bool>();

        JsonArray artists = item["artists"];
        if (artists.size() > 0) {
            currentSpotifyTrack.artistName = artists[0]["name"].as<String>();
        }

        currentSpotifyTrack.lastUpdate = millis();
        currentSpotifyTrack.dataValid = true;

        Serial.println("♪ " + currentSpotifyTrack.trackName + " - " + currentSpotifyTrack.artistName);
    }
}

// Replace the drawSpotifyWidget function with the corrected version
void drawSpotifyWidget(int x, int y, int width, int height) {
    // Update scroll animation every 120ms (same as your global scrollText)
    if (millis() - lastSpotifyScroll > 120) {
        // Simple scrolling logic for track name (exactly like your global scrollText)
        if (currentSpotifyTrack.trackName.length() > 8) { // 8 chars fit in available space
            spotifyTitleScroll--;
            if (spotifyTitleScroll < -(int)(currentSpotifyTrack.trackName.length() * 6)) {
                spotifyTitleScroll = WIDTH;
            }
        }

        // Simple scrolling logic for artist name
        if (currentSpotifyTrack.artistName.length() > 8) { // 8 chars fit in available space
            spotifyArtistScroll--;
            if (spotifyArtistScroll < -(int)(currentSpotifyTrack.artistName.length() * 6)) {
                spotifyArtistScroll = WIDTH;
            }
        }

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

    // Progress bar at top (y=0)
    if (currentSpotifyTrack.durationMs > 0) {
        drawSpotifyProgressBar(x, 0, width, currentSpotifyTrack.progressMs, currentSpotifyTrack.durationMs);
    }

    // Play/pause indicator
    uint16_t statusColor = currentSpotifyTrack.isPlaying ?
                           matrix.color565(30, 215, 96) :   // Spotify green
                           matrix.color565(255, 100, 100);  // Light red for paused

    if (currentSpotifyTrack.isPlaying) {
        drawPlayingBars(x + 1, y + 3, statusColor);
    } else {
        // Pause symbol - two vertical bars
        matrix.fillRect(x + 1, y + 4, 2, 6, statusColor);
        matrix.fillRect(x + 4, y + 4, 2, 6, statusColor);
    }

    // BOUNDARY PROTECTION: Clear text area first to prevent overflow into play/pause area
    matrix.fillRect(x + 8, y + 1, width - 8, 8, bgColor);   // Clear track name area
    matrix.fillRect(x + 8, y + 9, width - 8, 6, bgColor);   // Clear artist name area

    // Track name - WHITE and SIZE 1 (exactly like your global scrollText)
    matrix.setTextWrap(false);

    // Track name - let it scroll normally, matrix will clip at boundaries
    matrix.setCursor(x + 8 + spotifyTitleScroll, y + 1);
    matrix.setTextColor(matrix.color565(255, 255, 255)); // Pure white
    matrix.setTextSize(1);
    matrix.print(currentSpotifyTrack.trackName);

    // Artist name - let it scroll normally, matrix will clip at boundaries
    matrix.setTextWrap(false);
    matrix.setCursor(x + 8 + spotifyArtistScroll, y + 9);
    matrix.setTextColor(matrix.color565(102, 95, 95)); // Darker gray
    matrix.setTextSize(1);
    matrix.print(currentSpotifyTrack.artistName);
}

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

void setSpotifyTokens(String accessToken, String refreshToken) {
    spotifyAccessToken = accessToken;
    spotifyRefreshToken = refreshToken;
    tokenExpiry = millis() + 3600000; // 1 hour from now
    authenticationComplete = true;
    Serial.println("Spotify tokens set manually");
}

String getSpotifyAuthURL() {
    String authURL = "https://accounts.spotify.com/authorize?";
    authURL += "client_id=" + String(spotifyClientId);
    authURL += "&response_type=code";
    authURL += "&redirect_uri=https://spotify.com";
    authURL += "&scope=user-read-currently-playing,user-read-playback-state";
    authURL += "&show_dialog=true";

    return authURL;
}

bool exchangeCodeForTokens(String authCode) {
    WiFiSSLClient client;

    Serial.println("Connecting to accounts.spotify.com...");

    if (!client.connect("accounts.spotify.com", 443)) {
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

    // Wait for response with shorter timeout
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 10000) {
            Serial.println("Token exchange timeout");
            client.stop();
            return false;
        }
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

    // Read JSON response with timeout
    String response = "";
    timeout = millis();
    while (client.available() && (millis() - timeout < 3000)) {
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