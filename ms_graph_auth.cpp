#include "ms_graph_auth.h"
#include "credentials.h"
#include "web_server.h"

// Microsoft Graph OAuth tokens
String msGraphAccessToken = "";
String msGraphRefreshToken = "";
unsigned long msGraphTokenExpiry = 0;

// Microsoft Graph authentication URL
String getMsGraphAuthURL() {
    // Create the authorization URL for Microsoft Graph
    String url = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize";
    url += "?client_id=" + String(msGraphClientId);
    url += "&response_type=code";

    // Use the registered redirect URI from your Azure app registration
    // This should match exactly what's configured in your Azure portal
    url += "&redirect_uri=https://login.microsoftonline.com/common/oauth2/nativeclient";
    url += "&response_mode=query";
    url += "&scope=Presence.Read";
    url += "&state=12345";

    return url;
}

// Exchange authorization code for access and refresh tokens
bool exchangeMsGraphCodeForTokens(String authCode) {
    WiFiSSLClient client;

    Serial.println("Exchanging authorization code for tokens...");
    Serial.print("Code length: ");
    Serial.println(authCode.length());
    Serial.print("Code starts with: ");
    Serial.println(authCode.substring(0, 10) + "...");

    if (!client.connect("login.microsoftonline.com", 443)) {
        Serial.println("Connection to Microsoft login server failed");
        return false;
    }

    // Prepare the POST request
    String postData = "client_id=" + String(msGraphClientId);
    postData += "&scope=Presence.Read";
    postData += "&code=" + authCode;

    // Use the registered redirect URI from your Azure app registration
    // This should match exactly what's configured in your Azure portal
    postData += "&redirect_uri=https://login.microsoftonline.com/common/oauth2/nativeclient";
    postData += "&grant_type=authorization_code";
    postData += "&client_secret=" + String(msGraphClientSecret);

    // Send the HTTP request
    client.println("POST /common/oauth2/v2.0/token HTTP/1.1");
    client.println("Host: login.microsoftonline.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println("Connection: close");
    client.println();
    client.println(postData);

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
        Serial.println("Invalid response format");
        return false;
    }

    String jsonResponse = response.substring(jsonStart);

    // Parse the JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Check if the response contains an error
    if (doc["error"].isNull() == false) {
        Serial.print("Auth error: ");
        Serial.println(doc["error_description"].as<String>());
        return false;
    }

    // Extract tokens
    if (!doc["access_token"].isNull() && !doc["refresh_token"].isNull() && !doc["expires_in"].isNull()) {
        msGraphAccessToken = doc["access_token"].as<String>();
        msGraphRefreshToken = doc["refresh_token"].as<String>();

        // Calculate token expiry time (current time + expires_in - 300s buffer)
        unsigned long expiresIn = doc["expires_in"].as<unsigned long>();
        msGraphTokenExpiry = millis() + (expiresIn * 1000) - 300000; // 5 min buffer

        Serial.println("Microsoft Graph tokens obtained successfully");
        Serial.print("Access token: ");
        Serial.println(msGraphAccessToken.substring(0, 20) + "...");
        return true;
    }

    Serial.println("Failed to extract tokens from response");
    return false;
}

// Refresh the access token using the refresh token
bool refreshMsGraphToken() {
    WiFiSSLClient client;

    Serial.println("Refreshing Microsoft Graph token...");

    if (msGraphRefreshToken.length() == 0) {
        Serial.println("No refresh token available");
        return false;
    }

    if (!client.connect("login.microsoftonline.com", 443)) {
        Serial.println("Connection to Microsoft login server failed");
        return false;
    }

    // Prepare the POST request
    String postData = "client_id=" + String(msGraphClientId);
    postData += "&scope=Presence.Read";
    postData += "&refresh_token=" + msGraphRefreshToken;
    postData += "&grant_type=refresh_token";
    postData += "&client_secret=" + String(msGraphClientSecret);

    // Send the HTTP request
    client.println("POST /common/oauth2/v2.0/token HTTP/1.1");
    client.println("Host: login.microsoftonline.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println("Connection: close");
    client.println();
    client.println(postData);

    // Wait for the response
    while (client.connected() && !client.available()) delay(10);

    // Read and parse the response
    String response = "";
    while (client.available()) {
        char c = client.read();
        response += c;
    }
    client.stop();

    // Log HTTP status line
    int statusLineEnd = response.indexOf("\r\n");
    if (statusLineEnd > 0) {
        Serial.print("HTTP Status: ");
        Serial.println(response.substring(0, statusLineEnd));
    }

    // Extract the JSON part from the response
    int jsonStart = response.indexOf("{");
    if (jsonStart == -1) {
        Serial.println("Invalid response format - no JSON found");
        Serial.println("Response first 100 chars: " + response.substring(0, min(100, (int)response.length())));
        return false;
    }

    String jsonResponse = response.substring(jsonStart);

    // Parse the JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Check if the response contains an error
    if (doc["error"].isNull() == false) {
        Serial.print("Refresh error: ");
        Serial.println(doc["error_description"].as<String>());
        return false;
    }

    // Extract tokens
    if (!doc["access_token"].isNull() && !doc["expires_in"].isNull()) {
        msGraphAccessToken = doc["access_token"].as<String>();

        // Update refresh token if provided
        if (!doc["refresh_token"].isNull()) {
            msGraphRefreshToken = doc["refresh_token"].as<String>();
        }

        // Calculate token expiry time
        unsigned long expiresIn = doc["expires_in"].as<unsigned long>();
        msGraphTokenExpiry = millis() + (expiresIn * 1000) - 300000; // 5 min buffer

        Serial.println("Microsoft Graph token refreshed successfully");
        return true;
    }

    Serial.println("Failed to refresh token");
    return false;
}

// Check if the current token is valid
bool isMsGraphTokenValid() {
    // Check if we have a token and it's not expired
    return (msGraphAccessToken.length() > 0 && millis() < msGraphTokenExpiry);
}

// Set MS Graph tokens manually
void setMsGraphTokens(String accessToken, String refreshToken, unsigned long expiryTime) {
    msGraphAccessToken = accessToken;
    msGraphRefreshToken = refreshToken;
    msGraphTokenExpiry = expiryTime;
    Serial.println("Microsoft Graph tokens set manually");
}
