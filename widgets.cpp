#include "config.h"
#include "widgets.h"
#include "matrix_display.h"

// Widget state variables
WidgetType topLeftWidget = WIDGET_NONE;
WidgetType topRightWidget = WIDGET_NONE;

// Widget data
WeatherData currentWeather = {"Memphis", 72, "Sunny", "☀", 0};
TeamsData currentTeams = {"Available", "", 0x07E0, 0}; // Green
StockData currentStock = {"AAPL", 150.25, 2.50, true, 0};

void initializeWidgets() {
  Serial.println("Initializing widgets...");
  
  currentWeather.lastUpdate = 0;
  currentTeams.lastUpdate = 0;
  currentStock.lastUpdate = 0;
  
  Serial.println("Widgets initialized");
}

// Update widget drawing to use only the widget zone (y=0-14)
void drawWidget(WidgetType widget, int x, int y, int width, int height) {
  // Ensure widgets stay in widget zone
  if (y + height > WIDGET_ZONE_HEIGHT) {
    height = WIDGET_ZONE_HEIGHT - y;
  }

  // Serial.print("DEBUG: drawWidget() - type: ");
  // Serial.print(widget);
  // Serial.print(" in widget zone (");
  // Serial.print(x); Serial.print(","); Serial.print(y); Serial.println(")");

  switch (widget) {
  case WIDGET_CLOCK:
    drawClockWidget(x, y, width, height);
    break;
  case WIDGET_WEATHER:
    drawWeatherWidget(x, y, width, height);
    break;
  case WIDGET_TEAMS:
    drawTeamsWidget(x, y, width, height);
    break;
  case WIDGET_STOCKS:
    drawStocksWidget(x, y, width, height);
    break;
  case WIDGET_NONE:
  default:
    // Serial.println("DEBUG: Widget type is NONE - drawing nothing");
    resetWidgetZone(x, y, width, height);
    break;
  }
}

void updateWidgets() {

  uint32_t now = millis();

  // Update widget data periodically
  if (now - currentWeather.lastUpdate > 600000) {
    updateWeatherData();
  }

  if (now - currentTeams.lastUpdate > 30000) {
    updateTeamsData();
  }

  if (now - currentStock.lastUpdate > 60000) {
    updateStockData();
  }

  // Draw widgets only in top zone
  drawWidget(topLeftWidget, 0, 0, 32, WIDGET_ZONE_HEIGHT);      // Left half
  drawWidget(topRightWidget, 32, 0, 32, WIDGET_ZONE_HEIGHT);    // Right half
}

void drawClockWidget(int x, int y, int width, int height) {
  // Serial.println("DEBUG: drawClockWidget() entered");
  
  // Add a simple test rectangle to verify drawing works
  matrix.fillRect(x, y, width, height, matrix.color565(64, 0, 0)); // Dark red background
  
  matrix.setCursor(x, y + 8);
  matrix.setTextColor(0x001F); // Blue
  matrix.setTextSize(1);
  
  // Get current time (placeholder - you'll need to implement actual time)
  uint32_t now = millis();
  int hours = (now / 3600000) % 24;
  int minutes = (now / 60000) % 60;
  
  String timeStr = formatTime(false); // 12-hour format
  matrix.print(timeStr);
  
  // Serial.print("DEBUG: Clock widget drawn with time: ");
  // Serial.println(timeStr);
}

String formatTime(bool is24Hour) {
  // Placeholder implementation
  uint32_t now = millis();
  int hours = (now / 3600000) % 24;
  int minutes = (now / 60000) % 60;
  
  if (!is24Hour) {
    bool isPM = hours >= 12;
    if (hours > 12) hours -= 12;
    if (hours == 0) hours = 12;
    return String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes) + (isPM ? "P" : "A");
  } else {
    return String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);
  }
}

void drawWeatherWidget(int x, int y, int width, int height) {
  // Serial.println("DEBUG: drawWeatherWidget() entered");
  
  // Add a simple test rectangle to verify drawing works
  matrix.fillRect(x, y, width, height, matrix.color565(0, 64, 0)); // Dark green background
  
  matrix.setCursor(x, y + 8);
  matrix.setTextColor(0xFFE0); // Yellow
  matrix.setTextSize(1);
  
  // Show temperature
  String tempStr = String(currentWeather.temperature) + "F";
  matrix.print(tempStr);
  
  // Serial.print("DEBUG: Weather widget drawn with temp: ");
  // Serial.println(tempStr);
}

void drawTeamsWidget(int x, int y, int width, int height) {
  uint16_t dimColor = matrix.color565(
    ((currentTeams.statusColor >> 11) & 0x1F) << 1,  // Dim red
    ((currentTeams.statusColor >> 5) & 0x3F) << 1,   // Dim green  
    (currentTeams.statusColor & 0x1F) << 1            // Dim blue
  );
  
  matrix.setCursor(x, y + 8);
  matrix.setTextColor(dimColor);
  matrix.setTextSize(1);
  
  // Show first few characters of status
  String displayText = currentTeams.status;
  if (displayText.length() > 8) {
    displayText = displayText.substring(0, 8);
  }
  matrix.print(displayText);
}

void drawStocksWidget(int x, int y, int width, int height) {
  matrix.setCursor(x, y + 4);
  matrix.setTextColor(currentStock.isUp ? 0x07E0 : 0xF800); // Green if up, red if down
  matrix.setTextSize(1);
  
  // Show symbol on first line
  matrix.print(currentStock.symbol);
  
  // Show price on second line
  matrix.setCursor(x, y + 12);
  matrix.print("$" + String(currentStock.price, 1));
}

void resetWidgetZone(int x, int y, int width, int height){
  // Clear the entire widget zone
  matrix.fillRect(x, y, width, height, matrix.color565(0, 0, 0)); // Black background
  matrix.setTextWrap(true);
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.color565(255, 255, 255)); // White text
}

void updateWeatherData() {
  // Simulate weather data update
  currentWeather.temperature = random(65, 85);
  currentWeather.lastUpdate = millis();
  Serial.println("Weather data updated");
}

void updateTeamsData() {
  // Simulate teams status update
  String statuses[] = {"Available", "In Meeting", "Busy", "Away"};
  currentTeams.status = statuses[random(0, 4)];
  currentTeams.statusColor = getStatusColor(currentTeams.status);
  currentTeams.lastUpdate = millis();
  Serial.println("Teams data updated");
}

uint16_t getStatusColor(String status) {
  if (status == "Available") return 0x07E0;     // Green
  if (status == "Busy") return 0xF800;          // Red
  if (status == "Away") return 0xFFE0;          // Yellow
  return 0x001F;                                // Blue for "In Meeting"
}

void updateStockData() {
  // Simulate stock data update
  currentStock.change = random(-500, 500) / 100.0; // -5.00 to +5.00
  currentStock.isUp = currentStock.change >= 0;
  currentStock.price += currentStock.change;
  if (currentStock.price < 50) currentStock.price = 50; // Minimum price
  currentStock.lastUpdate = millis();
  Serial.println("Stock data updated");
}

void setTopLeftWidget(WidgetType widget) {
  topLeftWidget = widget;
  Serial.println("Top left widget set to: " + String(widget));
}

void setTopRightWidget(WidgetType widget) {
  topRightWidget = widget;
  Serial.println("Top right widget set to: " + String(widget));
}
