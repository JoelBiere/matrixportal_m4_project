#include "widgets.h"
#include "credentials.h"
#include <WiFiNINA.h>
#include "wifi_manager.h"
#include <ArduinoJson.h>
#include "hardware_config.h"

// Animation state variables
static uint32_t lastWeatherAnimation = 0;
static int cloudOffset = 0;
static int rainOffset = 0;
static int sunRayFrame = 0;


/*
 * For DEBUG mode - to view on the display the various weather conditions
 * and their effects on the display
 */

// Debug mode variables
static bool weatherDebugMode = false;
static int debugConditionIndex = 0;
static bool debugIsDay = true;
static uint32_t lastDebugSwitch = 0;

// Debug weather conditions to cycle through
struct DebugWeatherCondition {
    String condition;
    String location;
    int temperature;
    bool isDay;
    String description;
};

DebugWeatherCondition debugConditions[] = {
        {"Clear",         "Debug", 75, true,  "Sunny Day"},
        {"Clear",         "Debug", 65, false, "Clear Night"},
        {"Sunny",         "Debug", 82, true,  "Bright Sun"},
        {"Partly Cloudy", "Debug", 70, true,  "Partly Cloudy Day"},
        {"Partly Cloudy", "Debug", 58, false, "Partly Cloudy Night"},
        {"Cloudy",        "Debug", 68, true,  "Cloudy Day"},
        {"Cloudy",        "Debug", 55, false, "Cloudy Night"},
        {"Overcast",      "Debug", 65, true,  "Overcast"},
        {"Light Rain",    "Debug", 60, true,  "Light Rain"},
        {"Rain",          "Debug", 58, true,  "Heavy Rain"},
        {"Drizzle",       "Debug", 62, false, "Night Drizzle"},
        {"Snow",          "Debug", 32, true,  "Snowing"},
        {"Snow",          "Debug", 28, false, "Night Snow"},
        {"Thunderstorm",  "Debug", 72, true,  "Thunder & Lightning"},
        {"Thunderstorm",  "Debug", 66, false, "Night Storm"}
};

const int numDebugConditions = sizeof(debugConditions) / sizeof(debugConditions[0]);

// Function to enable/disable debug mode
void setWeatherDebugMode(bool enabled) {
    weatherDebugMode = enabled;
    debugConditionIndex = 0;
    lastDebugSwitch = millis();

    if (enabled) {
        Serial.println("=== Weather Debug Mode ENABLED ===");
        Serial.println("Will cycle through all weather conditions every 5 seconds");
        Serial.println("Use /weather_debug_off to disable");
    } else {
        Serial.println("=== Weather Debug Mode DISABLED ===");
        Serial.println("Returning to real weather data");
    }
}

// Function to manually advance to next debug condition
void advanceDebugWeather() {
    if (!weatherDebugMode) return;

    debugConditionIndex = (debugConditionIndex + 1) % numDebugConditions;
    lastDebugSwitch = millis();

    Serial.print("Debug weather: ");
    Serial.print(debugConditions[debugConditionIndex].description);
    Serial.print(" (");
    Serial.print(debugConditionIndex + 1);
    Serial.print("/");
    Serial.print(numDebugConditions);
    Serial.println(")");
}

// Core weather widget drawing (separated for debug use)
void drawWeatherWidgetCore(int x, int y, int width, int height) {
    // Update animation frame every 500ms
    if (millis() - lastWeatherAnimation > 500) {
        cloudOffset = (cloudOffset + 1) % width;
        rainOffset = (rainOffset + 1) % 4;
        sunRayFrame = (sunRayFrame + 1) % 8;
        lastWeatherAnimation = millis();
    }

    // Draw background based on day/night
    drawWeatherBackground(x, y, width, height);

    // Draw weather elements based on condition
    drawWeatherElements(x, y, width, height);

    // Draw text overlay (temperature and location)
    drawWeatherText(x, y, width, height);
}

// Function to get current debug status info
String getDebugWeatherInfo() {
    if (!weatherDebugMode) {
        return "Debug mode: OFF";
    }

    DebugWeatherCondition &current = debugConditions[debugConditionIndex];
    return "Debug: " + current.description +
           " (" + String(debugConditionIndex + 1) + "/" + String(numDebugConditions) + ")";
}

// END OF DEBUG WEATHER CODE

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
        if (c >= 32 && c <= 126) {
            // Only printable ASCII characters
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
// Modified drawWeatherWidget function with debug support
void drawWeatherWidget(int x, int y, int width, int height) {
    // Handle debug mode
    if (weatherDebugMode) {
        // Auto-advance every 5 seconds in debug mode
        if (millis() - lastDebugSwitch > 5000) {
            advanceDebugWeather();
        }

        // Override weather data with debug data
        DebugWeatherCondition &debugWeather = debugConditions[debugConditionIndex];

        // Temporarily override the current weather data for display
        WeatherData originalWeather = currentWeather;
        currentWeather.condition = debugWeather.condition;
        currentWeather.location = debugWeather.location;
        currentWeather.temperature = debugWeather.temperature;
        currentWeather.isDay = debugWeather.isDay;
        currentWeather.dataValid = true;

        // Draw the weather widget with debug data
        drawWeatherWidgetCore(x, y, width, height);

        // Show debug info on display
        // matrix.setCursor(x + 1, y + height - 16);
        // matrix.setTextColor(matrix.color565(255, 100, 100)); // Light red for debug
        // matrix.setTextSize(1);
        // matrix.print("DBG:" + String(debugConditionIndex + 1) + "/" + String(numDebugConditions));

        // Restore original weather data
        currentWeather = originalWeather;
        return;
    }

    // Normal mode - check if we have valid weather data
    if (!currentWeather.dataValid) {
        // Show loading or error state
        matrix.fillRect(x, y, width, height, matrix.color565(32, 16, 0)); // Dark orange
        matrix.setCursor(x, y + 4);
        matrix.setTextColor(matrix.color565(255, 128, 0));
        matrix.setTextSize(1);
        matrix.print("Loading...");
        return;
    }

    drawWeatherWidgetCore(x, y, width, height);
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
        if (millis() - currentWeather.lastUpdate > 1800000) {
            // 30 minutes
            currentWeather.dataValid = false;
        }
    }

    // Schedule next update (every 10 minutes for weather data)
    currentWeather.lastUpdate = millis();
}

// Alternative: Fallback to coordinates if IP detection fails
bool fetchWeatherByCoordinates(float lat, float lon) {
    WiFiClient client;
    const char *host = "api.weatherapi.com";

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


/*
 * Weather animation functions
 */

void drawStars(int x, int y, int width, int height) {
    // Draw a few small stars for night sky
    uint16_t starColor = matrix.color565(255, 255, 255);

    // Fixed star positions based on widget location
    int starPositions[][2] = {
            {x + 8,  y + 2},
            {x + 25, y + 1},
            {x + 45, y + 3},
            {x + 58, y + 2},
            {x + 15, y + 4},
            {x + 38, y + 5}
    };

    for (int i = 0; i < 6; i++) {
        // Twinkling effect - show star based on animation frame
        if ((sunRayFrame + i) % 4 != 0) {
            matrix.drawPixel(starPositions[i][0], starPositions[i][1], starColor);
        }
    }
}

void drawWeatherElements(int x, int y, int width, int height) {
    String condition = currentWeather.condition;
    condition.toLowerCase(); // Make case-insensitive

    // Determine main weather elements to draw
    if (condition.indexOf("clear") >= 0 || condition.indexOf("sunny") >= 0) {
        if (currentWeather.isDay) {
            drawAnimatedSun(x + width - 20, y + 2);
        } else {
            drawMoon(x + width - 16, y + 2);
        }
    } else if (condition.indexOf("partly cloudy") >= 0 || condition.indexOf("partly") >= 0) {
        // Draw sun/moon with clouds
        if (currentWeather.isDay) {
            drawAnimatedSun(x + width - 25, y + 1);
        } else {
            drawMoon(x + width - 20, y + 1);
        }
        drawAnimatedClouds(x, y, width, height, false); // Light clouds
    } else if (condition.indexOf("cloudy") >= 0 || condition.indexOf("overcast") >= 0) {
        drawAnimatedClouds(x, y, width, height, true); // Heavy clouds
    } else if (condition.indexOf("rain") >= 0 || condition.indexOf("drizzle") >= 0) {
        drawAnimatedClouds(x, y, width, height, true); // Rain clouds
        drawAnimatedRain(x, y, width, height);
        if (condition.indexOf("thunder") >= 0) {
            drawLightning(x, y, width, height); // Add lightning if thunderstorm
        }
    } else if (condition.indexOf("snow") >= 0 || condition.indexOf("ice") >= 0) {
        drawAnimatedClouds(x, y, width, height, true); // Snow clouds
        drawAnimatedSnow(x, y, width, height);
    } else if (condition.indexOf("thunder") >= 0 || condition.indexOf("storm") >= 0) {
        drawAnimatedClouds(x, y, width, height, true); // Storm clouds
        drawAnimatedRain(x, y, width, height);
        drawLightning(x, y, width, height);
    } else {
        // Default: just sun or moon
        if (currentWeather.isDay) {
            drawAnimatedSun(x + width - 20, y + 2);
        } else {
            drawMoon(x + width - 16, y + 2);
        }
    }
}

void drawAnimatedSun(int x, int y) {
    uint16_t sunColor = matrix.color565(255, 255, 0); // Bright yellow
    uint16_t rayColor = matrix.color565(255, 215, 0); // Gold

    // Draw sun center (3x3)
    matrix.fillRect(x, y, 3, 3, sunColor);

    // Draw animated sun rays
    if (sunRayFrame % 2 == 0) {
        // Horizontal and vertical rays
        matrix.drawPixel(x - 1, y + 1, rayColor); // Left
        matrix.drawPixel(x + 3, y + 1, rayColor); // Right
        matrix.drawPixel(x + 1, y - 1, rayColor); // Top
        matrix.drawPixel(x + 1, y + 3, rayColor); // Bottom
    } else {
        // Diagonal rays
        matrix.drawPixel(x - 1, y, rayColor); // Top-left
        matrix.drawPixel(x + 3, y, rayColor); // Top-right
        matrix.drawPixel(x - 1, y + 2, rayColor); // Bottom-left
        matrix.drawPixel(x + 3, y + 2, rayColor); // Bottom-right
    }
}

void drawMoon(int x, int y) {
    uint16_t moonColor = matrix.color565(255, 255, 224); // Light yellow
    uint16_t craterColor = matrix.color565(200, 200, 180); // Slightly darker

    // Draw moon (4x4 circle-ish)
    matrix.fillRect(x, y + 1, 4, 2, moonColor);
    matrix.fillRect(x + 1, y, 2, 4, moonColor);

    // Add a few crater pixels
    matrix.drawPixel(x + 1, y + 1, craterColor);
    matrix.drawPixel(x + 2, y + 2, craterColor);
}

void drawAnimatedClouds(int x, int y, int width, int height, bool heavy) {
    uint16_t cloudColor = heavy
                          ? matrix.color565(128, 128, 128)
                          : // Dark gray for heavy clouds
                          matrix.color565(220, 220, 220); // Light gray for light clouds

    // Draw multiple cloud layers that move at different speeds
    int cloud1Pos = (cloudOffset * 2) % (width + 10) - 5;
    int cloud2Pos = (cloudOffset * 3) % (width + 15) - 8;

    // Cloud 1 (upper)
    drawCloudShape(x + cloud1Pos, y + 2, cloudColor);

    // Cloud 2 (lower, different speed)
    drawCloudShape(x + cloud2Pos, y + 5, cloudColor);

    if (heavy) {
        // Add more cloud layers for heavy clouds
        int cloud3Pos = (cloudOffset) % (width + 8) - 3;
        drawCloudShape(x + cloud3Pos, y + 4, cloudColor);
    }
}

void drawCloudShape(int x, int y, uint16_t color) {
    // Draw a simple cloud shape (skip if off-screen)
    if (x < -8 || x > 64) return;

    // Cloud body
    matrix.fillRect(x + 1, y, 6, 2, color);
    matrix.fillRect(x, y + 1, 8, 1, color);
    matrix.drawPixel(x + 2, y - 1, color); // Cloud puff
    matrix.drawPixel(x + 5, y - 1, color); // Cloud puff
}

void drawAnimatedRain(int x, int y, int width, int height) {
    uint16_t rainColor = matrix.color565(0, 150, 255); // Blue

    // Draw rain drops at different positions based on animation frame
    for (int i = 0; i < width; i += 4) {
        int dropY = y + 8 + ((rainOffset + i) % 3);
        if (dropY < y + height - 1) {
            matrix.drawPixel(x + i + 1, dropY, rainColor);
            matrix.drawPixel(x + i + 2, dropY + 1, rainColor);
        }
    }
}

void drawAnimatedSnow(int x, int y, int width, int height) {
    uint16_t snowColor = matrix.color565(255, 255, 255); // White

    // Draw snowflakes falling at different rates
    for (int i = 0; i < width; i += 6) {
        int flakeY = y + 6 + ((rainOffset + i / 2) % 4);
        if (flakeY < y + height - 1) {
            matrix.drawPixel(x + i + 2, flakeY, snowColor);
            // Add some sparkle effect
            if (sunRayFrame % 3 == 0) {
                matrix.drawPixel(x + i + 1, flakeY - 1, snowColor);
                matrix.drawPixel(x + i + 3, flakeY - 1, snowColor);
            }
        }
    }
}

void drawLightning(int x, int y, int width, int height) {
    // Lightning flashes occasionally
    if (sunRayFrame == 0) {
        uint16_t lightningColor = matrix.color565(255, 255, 255);
        // Draw simple lightning bolt
        matrix.drawPixel(x + width / 2, y + 6, lightningColor);
        matrix.drawPixel(x + width / 2 + 1, y + 7, lightningColor);
        matrix.drawPixel(x + width / 2, y + 8, lightningColor);
        matrix.drawPixel(x + width / 2 - 1, y + 9, lightningColor);
    }
}

void drawWeatherBackground(int x, int y, int width, int height) {
    uint16_t bgColor;
    String condition = currentWeather.condition;
    condition.toLowerCase(); // Make case-insensitive

    if (currentWeather.isDay) {
        // Day: Light blue sky gradient
        bgColor = matrix.color565(3, 44, 98);
        // Add some darker blue at the top for sky gradient effect
        uint16_t lightSky = matrix.color565(36, 145, 186);
        if (
                condition.indexOf("rain") >= 0 || condition.indexOf("drizzle") >= 0 ||
                condition.indexOf("storm") >= 0 || condition.indexOf("thunder") >= 0 ||
                condition.indexOf("snow") >= 0) {
            // Rainy day - use dark blue for sky
            bgColor = matrix.color565(55, 55, 56);
            lightSky = matrix.color565(86, 86, 87);
        }
        matrix.fillRect(x, y, width, height, bgColor);
        matrix.fillRect(x, y, width, 3, lightSky);
    } else {
        // Night: Dark blue to purple gradient
        bgColor = matrix.color565(20, 1, 54); // Midnight blue
        matrix.fillRect(x, y, width, height, bgColor);

        // Add purple tint at top
        uint16_t darkPurple = matrix.color565(25, 25, 112);
        matrix.fillRect(x, y, width, 4, darkPurple);

        // Draw some stars
        drawStars(x, y, width, height);
    }
}

void drawWeatherText(int x, int y, int width, int height) {
    // Determine text colors based on background and weather conditions
    uint16_t tempColor, locationColor;

    String condition = currentWeather.condition;
    condition.toLowerCase();

    if (currentWeather.isDay) {
        tempColor = matrix.color565(237, 5, 16); // Red
        locationColor = matrix.color565(247, 153, 2); // Burnt Orange

    } else {
        // Night scenes - generally dark backgrounds
        if (condition.indexOf("rain") >= 0 || condition.indexOf("storm") >= 0 ||
            condition.indexOf("thunder") >= 0) {
            // Stormy night - use very bright colors
            tempColor = matrix.color565(247, 247, 0); // Loud yellow
            locationColor = matrix.color565(207, 255, 4); // Neon Yellow
        } else if (condition.indexOf("snow") >= 0) {
            // Snowy night - colorful on dark
            tempColor = matrix.color565(150, 150, 255); // Light blue
            locationColor = matrix.color565(255, 150, 150); // Light red
        } else {
            // Clear/cloudy night - dark background, use bright text
            tempColor = matrix.color565(247, 247, 0); // Loud yellow
            locationColor = matrix.color565(207, 255, 4); // Neon Yellow
        }
    }

    // Temperature in upper left with adaptive color
    matrix.setCursor(x + 1, y);
    matrix.setTextColor(tempColor);
    matrix.setTextSize(1);
    matrix.print(String(currentWeather.temperature) + "F");

    // Location on bottom line with adaptive color
    matrix.setCursor(x + 1, y + height - 7);
    matrix.setTextColor(locationColor);
    String displayLocation = currentWeather.location;
    // word length + 1 is spaces - each char is 5 pixels wide
    // I have 64 pixels wide, so 10 characters max
    if (displayLocation.length() > 10) {
        displayLocation = displayLocation.substring(0, 10) + "...";
    }
    matrix.print(displayLocation);
}
