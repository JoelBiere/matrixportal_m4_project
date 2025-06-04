// widgets.cpp - Add this as a new file

#include "config.h"
#include "widgets.h"

// Widget configuration variables
WidgetType topLeftWidget = WIDGET_CLOCK;
WidgetType topRightWidget = WIDGET_WEATHER;
bool widgetsEnabled = true;

// Widget data
WeatherData currentWeather = {"Memphis", 72, "Sunny", "☀", 0};
TeamsData currentTeams = {"Available", "", 0x07E0, 0}; // Green
StockData currentStock = {"AAPL", 150.25, 2.50, true, 0};

void initializeWidgets() {
  Serial.println("Initializing smart widgets...");
  
  // Initialize widget data
  currentWeather.lastUpdate = 0;
  currentTeams.lastUpdate = 0;
  currentStock.lastUpdate = 0;
  
  Serial.println("Widgets initialized");
}

void updateWidgets() {
  if (!widgetsEnabled) return;
  
  uint32_t now = millis();
  
  // Update widget data periodically
  if (now - currentWeather.lastUpdate > 600000) { // 10 minutes
    updateWeatherData();
  }
  
  if (now - currentTeams.lastUpdate > 30000) { // 30 seconds
    updateTeamsData();
  }
  
  if (now - currentStock.lastUpdate > 60000) { // 1 minute
    updateStockData();
  }
  
  // Draw widgets on display
  drawWidget(topLeftWidget, 0, 0, 32, 16);      // Top left quadrant
  drawWidget(topRightWidget, 32, 0, 32, 16);    // Top right quadrant
}

void drawWidget(WidgetType widget, int x, int y, int width, int height) {
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
      // Draw empty widget area
      matrix.fillRect(x, y, width, height, matrix.color565(20, 20, 20));
      break;
  }
  
  // Draw widget border
  matrix.drawRect(x, y, width, height, matrix.color565(100, 100, 100));
}

void drawClockWidget(int x, int y, int width, int height) {
  // Clear widget area
  matrix.fillRect(x + 1, y + 1, width - 2, height - 2, matrix.color565(0, 0, 50));
  
  // Get current time (simplified - you'd get real time from NTP)
  String timeStr = formatTime(false); // 12-hour format
  String dateStr = "12/25"; // You'd get real date
  
  // Draw time
  matrix.setTextColor(matrix.color565(255, 255, 255));
  matrix.setTextSize(1);
  matrix.setCursor(x + 2, y + 3);
  matrix.print(timeStr);
  
  // Draw date
  matrix.setTextColor(matrix.color565(200, 200, 200));
  matrix.setCursor(x + 2, y + 11);
  matrix.print(dateStr);
}

void drawWeatherWidget(int x, int y, int width, int height) {
  // Clear widget area
  matrix.fillRect(x + 1, y + 1, width - 2, height - 2, matrix.color565(0, 30, 50));
  
  // Draw temperature
  matrix.setTextColor(matrix.color565(255, 255, 100));
  matrix.setTextSize(1);
  matrix.setCursor(x + 2, y + 3);
  matrix.print(String(currentWeather.temperature) + "F");
  
  // Draw condition
  matrix.setTextColor(matrix.color565(200, 200, 255));
  matrix.setCursor(x + 2, y + 11);
  matrix.print(currentWeather.condition.substring(0, 6)); // Truncate to fit
}

void drawTeamsWidget(int x, int y, int width, int height) {
  // Clear widget area with status color background
  uint16_t bgColor = matrix.color565(
    ((currentTeams.statusColor >> 11) & 0x1F) << 1,  // Dim red
    ((currentTeams.statusColor >> 5) & 0x3F) << 0,   // Dim green  
    (currentTeams.statusColor & 0x1F) << 1            // Dim blue
  );
  matrix.fillRect(x + 1, y + 1, width - 2, height - 2, bgColor);
  
  // Draw status
  matrix.setTextColor(matrix.color565(255, 255, 255));
  matrix.setTextSize(1);
  matrix.setCursor(x + 2, y + 3);
  matrix.print("TEAMS");
  
  matrix.setCursor(x + 2, y + 11);
  if (currentTeams.status == "Available") {
    matrix.print("FREE");
  } else if (currentTeams.status == "Busy" || currentTeams.status == "In Meeting") {
    matrix.print("BUSY");
  } else {
    matrix.print("AWAY");
  }
}

void drawStocksWidget(int x, int y, int width, int height) {
  // Clear widget area
  matrix.fillRect(x + 1, y + 1, width - 2, height - 2, matrix.color565(30, 30, 30));
  
  // Draw stock symbol
  matrix.setTextColor(matrix.color565(255, 255, 255));
  matrix.setTextSize(1);
  matrix.setCursor(x + 2, y + 3);
  matrix.print(currentStock.symbol);
  
  // Draw price with color based on change
  uint16_t priceColor = currentStock.isUp ? 
    matrix.color565(0, 255, 0) : matrix.color565(255, 0, 0);
  matrix.setTextColor(priceColor);
  matrix.setCursor(x + 2, y + 11);
  matrix.print("$" + String(currentStock.price, 0));
}

// Data update functions (simplified - you'd call real APIs)
void updateWeatherData() {
  Serial.println("Updating weather data...");
  // Here you'd make an HTTP request to weather API
  // For now, just simulate data
  currentWeather.temperature = random(65, 85);
  currentWeather.lastUpdate = millis();
}

void updateTeamsData() {
  Serial.println("Updating Teams data...");
  // Here you'd call Microsoft Graph API
  // For now, simulate status changes
  String statuses[] = {"Available", "Busy", "Away", "In Meeting"};
  currentTeams.status = statuses[random(0, 4)];
  currentTeams.statusColor = getStatusColor(currentTeams.status);
  currentTeams.lastUpdate = millis();
}

void updateStockData() {
  Serial.println("Updating stock data...");
  // Here you'd call stock API
  // For now, simulate price changes
  currentStock.change = random(-500, 500) / 100.0; // -5.00 to +5.00
  currentStock.price += currentStock.change;
  currentStock.isUp = currentStock.change >= 0;
  currentStock.lastUpdate = millis();
}

// Utility functions
String formatTime(bool show24Hour) {
  // Simplified - you'd use real time library
  static int hour = 9;
  static int minute = 30;
  static uint32_t lastUpdate = 0;
  
  if (millis() - lastUpdate > 60000) { // Update every minute
    minute++;
    if (minute >= 60) {
      minute = 0;
      hour++;
      if (hour >= (show24Hour ? 24 : 13)) {
        hour = show24Hour ? 0 : 1;
      }
    }
    lastUpdate = millis();
  }
  
  String timeStr = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);
  if (!show24Hour) {
    timeStr += (hour >= 12) ? "P" : "A";
  }
  return timeStr;
}

uint16_t getStatusColor(String status) {
  if (status == "Available") return matrix.color565(0, 255, 0);     // Green
  if (status == "Busy") return matrix.color565(255, 0, 0);          // Red
  if (status == "Away") return matrix.color565(255, 255, 0);        // Yellow
  if (status == "In Meeting") return matrix.color565(255, 0, 0);    // Red
  return matrix.color565(128, 128, 128);                            // Gray
}

// Web interface functions
void setTopLeftWidget(WidgetType widget) {
  topLeftWidget = widget;
  Serial.println("Top left widget changed to: " + String(widget));
}

void setTopRightWidget(WidgetType widget) {
  topRightWidget = widget;
  Serial.println("Top right widget changed to: " + String(widget));
}

void toggleWidgets() {
  widgetsEnabled = !widgetsEnabled;
  Serial.println("Widgets " + String(widgetsEnabled ? "enabled" : "disabled"));
}