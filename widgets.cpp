#include "config.h"
#include "widgets.h"
#include "matrix_display.h"

// Widget state variables
WidgetType topLeftWidget = WIDGET_CLOCK;
WidgetType topRightWidget = WIDGET_WEATHER;
bool widgetsEnabled = false;

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

void updateWidgets() {
  Serial.print("DEBUG: updateWidgets() - widgetsEnabled: ");
  Serial.print(widgetsEnabled ? "true" : "false");
  Serial.print(", currentDisplayMode: ");
  Serial.println(currentDisplayMode);
  
  if (!widgetsEnabled) {
    Serial.println("DEBUG: Widgets disabled - returning early");
    return;
  }
  
  Serial.println("DEBUG: Widgets enabled, continuing...");
  
  uint32_t now = millis();
  
  // Update weather every 10 minutes
  if (now - currentWeather.lastUpdate > 600000) { // 10 minutes
    updateWeatherData();
  }
  
  // Update teams every 30 seconds
  if (now - currentTeams.lastUpdate > 30000) { // 30 seconds
    updateTeamsData();
  }
  
  // Update stocks every 1 minute
  if (now - currentStock.lastUpdate > 60000) { // 1 minute
    updateStockData();
  }
  
  // Draw the widgets
  Serial.println("DEBUG: About to draw widgets");
  drawWidget(topLeftWidget, 0, 0, 32, 16);      // Top left quadrant
  Serial.println("DEBUG: Left widget drawn");
  drawWidget(topRightWidget, 32, 0, 32, 16);    // Top right quadrant
  Serial.println("DEBUG: Right widget drawn");
}

void drawWidget(WidgetType widget, int x, int y, int width, int height) {
  Serial.print("DEBUG: drawWidget() called - type: ");
  Serial.print(widget);
  Serial.print(" at position (");
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.println(")");
  
  switch (widget) {
    case WIDGET_CLOCK:
      Serial.println("DEBUG: Drawing clock widget");
      drawClockWidget(x, y, width, height);
      break;
    case WIDGET_WEATHER:
      Serial.println("DEBUG: Drawing weather widget");
      drawWeatherWidget(x, y, width, height);
      break;
    case WIDGET_TEAMS:
      Serial.println("DEBUG: Drawing teams widget");
      drawTeamsWidget(x, y, width, height);
      break;
    case WIDGET_STOCKS:
      Serial.println("DEBUG: Drawing stocks widget");
      drawStocksWidget(x, y, width, height);
      break;
    case WIDGET_NONE:
    default:
      Serial.println("DEBUG: Widget type is NONE - drawing nothing");
      break;
  }
}

void drawClockWidget(int x, int y, int width, int height) {
  Serial.println("DEBUG: drawClockWidget() entered");
  
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
  
  Serial.print("DEBUG: Clock widget drawn with time: ");
  Serial.println(timeStr);
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
  Serial.println("DEBUG: drawWeatherWidget() entered");
  
  // Add a simple test rectangle to verify drawing works
  matrix.fillRect(x, y, width, height, matrix.color565(0, 64, 0)); // Dark green background
  
  matrix.setCursor(x, y + 8);
  matrix.setTextColor(0xFFE0); // Yellow
  matrix.setTextSize(1);
  
  // Show temperature
  String tempStr = String(currentWeather.temperature) + "F";
  matrix.print(tempStr);
  
  Serial.print("DEBUG: Weather widget drawn with temp: ");
  Serial.println(tempStr);
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

void toggleWidgets() {
  widgetsEnabled = !widgetsEnabled;
  Serial.println("Widgets " + String(widgetsEnabled ? "enabled" : "disabled"));
}