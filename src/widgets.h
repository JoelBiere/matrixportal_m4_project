#ifndef WIDGETS_H
#define WIDGETS_H

#include "config.h"

enum WidgetType {
  WIDGET_NONE = 0,
  WIDGET_CLOCK = 1,
  WIDGET_WEATHER = 2,
  WIDGET_TEAMS = 3,
  WIDGET_STOCKS = 4
};

// Widget data structures
struct WeatherData {
  String city;
  int temperature;
  String condition;
  String icon;
  uint32_t lastUpdate;
};

struct TeamsData {
  String status;        // "Available", "Busy", "Away", "In Meeting"
  String statusMessage;
  uint16_t statusColor;
  uint32_t lastUpdate;
};

struct StockData {
  String symbol;
  float price;
  float change;
  bool isUp;
  uint32_t lastUpdate;
};


// Widget configuration
extern WidgetType topLeftWidget;
extern WidgetType topRightWidget;
extern bool widgetsEnabled;

extern WeatherData currentWeather;
extern TeamsData currentTeams;
extern StockData currentStock;

// Widget functions
void initializeWidgets();
void updateWidgets();
void drawWidget(WidgetType widget, int x, int y, int width, int height);

// Individual widget drawing functions
void drawClockWidget(int x, int y, int width, int height);
void drawWeatherWidget(int x, int y, int width, int height);
void drawTeamsWidget(int x, int y, int width, int height);
void drawStocksWidget(int x, int y, int width, int height);

// Widget data update functions
void updateWeatherData();
void updateTeamsData();
void updateStockData();

// Web interface functions
void setTopLeftWidget(WidgetType widget);
void setTopRightWidget(WidgetType widget);
void toggleWidgets();

// Utility functions
String formatTime(bool show24Hour = false);
uint16_t getStatusColor(String status);
void drawSmallText(int x, int y, String text, uint16_t color);

#endif // WIDGETS_H