#include "widgets.h"
#include "credentials.h"
#include <WiFiNINA.h>
#include "wifi_manager.h"
#include <ArduinoJson.h>

// Forward declarations for functions used within this file
bool parseWeatherJSON(String jsonString);
void drawWeatherIcon(int x, int y, String condition);


// Function to make HTTP request to WeatherAPI
bool fetchWeatherData() {
  WiFiClient client;

  Serial.println("Fetching weather data...");

  if (!client.connect("api.weatherapi.com", 80)) {
    Serial.println("Connection to WeatherAPI failed");
    return false;
  }

  // Build request URL for IP-based location detection
  String url = "/v1/current.json?key=" + String(weatherApiKey) + "&q=auto:ip&aqi=no";

  // Send HTTP/1.0 request to avoid chunked encoding
  client.print("GET " + url + " HTTP/1.0\r\n");
  client.print("Host: api.weatherapi.com\r\n");
  client.print("User-Agent: MatrixPortal-Weather/1.0\r\n");
  client.print("Connection: close\r\n\r\n");

  // Wait for response with timeout
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 10000) {
      Serial.println("Request timeout");
      client.stop();
      return false;
    }
  }

  // Skip HTTP headers
  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break; // Empty line = end of headers
  }

  // Read JSON response
  String response = "";
  while (client.available()) {
    char c = client.read();
    if (c >= 32 && c <= 126) { // Only printable ASCII characters
      response += c;
    }
  }

  client.stop();

  // Validate and clean JSON
  response.trim();
  if (!response.startsWith("{") || !response.endsWith("}")) {
    // Extract JSON if wrapped in extra characters
    int jsonStart = response.indexOf('{');
    int jsonEnd = response.lastIndexOf('}');
    if (jsonStart >= 0 && jsonEnd > jsonStart) {
      response = response.substring(jsonStart, jsonEnd + 1);
    } else {
      Serial.println("Invalid JSON response");
      return false;
    }
  }

  return parseWeatherJSON(response);
}

// example response
// {
//   "location": {
//     "name": "Leesburg",
//     "region": "Virginia",
//     "country": "United States of America",
//     "lat": 39.116,
//     "lon": -77.564,
//     "tz_id": "America/New_York",
//     "localtime_epoch": 1749098091,
//     "localtime": "2025-06-05 00:34"
//   },
//   "current": {
//     "last_updated_epoch": 1749097800,
//     "last_updated": "2025-06-05 00:30",
//     "temp_c": 23.2,
//     "temp_f": 73.8,
//     "is_day": 0,
//     "condition": {
//       "text": "Cloudy",
//       "icon": "//cdn.weatherapi.com/weather/64x64/night/119.png",
//       "code": 1006
//     },
//     "wind_mph": 6.7,
//     "wind_kph": 10.8,
//     "wind_degree": 207,
//     "wind_dir": "SSW",
//     "pressure_mb": 1021,
//     "pressure_in": 30.14,
//     "precip_mm": 0,
//     "precip_in": 0,
//     "humidity": 78,
//     "cloud": 0,
//     "feelslike_c": 25.2,
//     "feelslike_f": 77.3,
//     "windchill_c": 21.4,
//     "windchill_f": 70.5,
//     "heatindex_c": 22.7,
//     "heatindex_f": 72.8,
//     "dewpoint_c": 17.4,
//     "dewpoint_f": 63.3,
//     "vis_km": 16,
//     "vis_miles": 9,
//     "uv": 0,
//     "gust_mph": 14.1,
//     "gust_kph": 22.7
//   }
// }
bool parseWeatherJSON(String jsonString) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return false;
  }

  // Extract location data
  JsonObject location = doc["location"];
  if (location.isNull()) {
    Serial.println("Location data not found");
    return false;
  }

  currentWeather.location = location["name"].as<String>();
  currentWeather.region = location["region"].as<String>();
  currentWeather.country = location["country"].as<String>();

  // Extract current weather data
  JsonObject current = doc["current"];
  if (current.isNull()) {
    Serial.println("Current weather data not found");
    return false;
  }

  currentWeather.temperature = current["temp_f"].as<int>();
  currentWeather.isDay = static_cast<bool>(current["is_day"].as<int>());
  currentWeather.condition = current["condition"]["text"].as<String>();
  currentWeather.icon = current["condition"]["icon"].as<String>();
  currentWeather.humidity = current["humidity"].as<int>();
  currentWeather.windSpeed = current["wind_mph"].as<int>();
  currentWeather.windDirection = current["wind_dir"].as<String>();

  currentWeather.lastUpdate = millis();
  currentWeather.dataValid = true;

  Serial.println("Weather updated: " + currentWeather.location +
                 ", " + String(currentWeather.temperature) + "°F, " +
                 currentWeather.condition);

  return true;
}

// weather widget drawing function
void drawWeatherWidget(int x, int y, int width, int height) {
  if (!currentWeather.dataValid) {
    // Show loading or error state
    matrix.fillRect(x, y, width, height, matrix.color565(32, 16, 0)); // Dark orange
    matrix.setCursor(x, y + 4);
    matrix.setTextColor(matrix.color565(255, 128, 0));
    matrix.setTextSize(1);
    matrix.print("Loading...");
    return;
  }

  // Clear the widget area
  matrix.fillRect(x, y, width, height, matrix.color565(0, 16, 32)); // Dark blue background

  // Display temperature prominently
  matrix.setCursor(x, y + 1);
  matrix.setTextColor(matrix.color565(255, 255, 0)); // Yellow
  matrix.setTextSize(1);
  matrix.print(String(currentWeather.temperature) + "F");

  // Display location on second line (truncated if needed)
  matrix.setCursor(x, y + 9);
  matrix.setTextColor(matrix.color565(128, 255, 128)); // Light green
  String displayLocation = currentWeather.location;
  if (displayLocation.length() > 10) {
    displayLocation = displayLocation.substring(0, 10);
  }
  matrix.print(displayLocation);

  // Optional: Add a simple weather icon indicator
  drawWeatherIcon(x + 45, y + 1, currentWeather.condition);
}

// Simple weather icon function
void drawWeatherIcon(int x, int y, String condition) {
  uint16_t iconColor = matrix.color565(255, 255, 255); // Default white

  // Simple icon based on condition
  if (condition.indexOf("sunny") >= 0 || condition.indexOf("clear") >= 0) {
    // Draw sun
    iconColor = matrix.color565(255, 255, 0); // Yellow
    matrix.fillRect(x, y, 3, 3, iconColor);
  } else if (condition.indexOf("cloud") >= 0) {
    // Draw cloud
    iconColor = matrix.color565(192, 192, 192); // Light gray
    matrix.fillRect(x, y + 1, 4, 2, iconColor);
    matrix.drawPixel(x + 1, y, iconColor);
    matrix.drawPixel(x + 2, y, iconColor);
  } else if (condition.indexOf("rain") >= 0 || condition.indexOf("drizzle") >= 0) {
    // Draw rain drops
    iconColor = matrix.color565(0, 128, 255); // Blue
    matrix.drawPixel(x, y + 2, iconColor);
    matrix.drawPixel(x + 2, y + 1, iconColor);
    matrix.drawPixel(x + 1, y + 3, iconColor);
  } else if (condition.indexOf("snow") >= 0) {
    // Draw snowflake
    iconColor = matrix.color565(255, 255, 255); // White
    matrix.drawPixel(x + 1, y, iconColor);
    matrix.drawPixel(x + 1, y + 2, iconColor);
    matrix.drawPixel(x, y + 1, iconColor);
    matrix.drawPixel(x + 2, y + 1, iconColor);
  }
}

// Updated main weather update function
void updateWeatherData() {
  Serial.println("Updating weather data via WeatherAPI...");

  if (!isWiFiConnected()) {
    Serial.println("WiFi not connected - skipping weather update");
    return;
  }

  bool success = fetchWeatherData();

  if (!success) {
    Serial.println("Failed to fetch weather data");
    // Keep old data but mark as potentially stale
    if (millis() - currentWeather.lastUpdate > 1800000) { // 30 minutes
      currentWeather.dataValid = false;
    }
  }

  // Schedule next update (every 10 minutes for weather data)
  currentWeather.lastUpdate = millis();
}

// Alternative: Fallback to coordinates if IP detection fails
bool fetchWeatherByCoordinates(float lat, float lon) {
  WiFiClient client;
  const char* host = "api.weatherapi.com";

  if (!client.connect(host, 80)) {
    return false;
  }

  // Use coordinates instead of IP
  String url = "/v1/current.json?key=" + String(weatherApiKey) +
               "&q=" + String(lat, 6) + "," + String(lon, 6) + "&aqi=no";

  client.print("GET " + url + " HTTP/1.1\r\n");
  client.print("Host: " + String(host) + "\r\n");
  client.print("Connection: close\r\n\r\n");

  // ... rest of the function similar to fetchWeatherData()
  // (implementation details omitted for brevity)

  return true;
}