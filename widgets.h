#ifndef WIDGETS_H
#define WIDGETS_H

#include "hardware_config.h"

// Widget types for upper quadrants
enum WidgetType
{
  WIDGET_NONE = 0,
  WIDGET_CLOCK = 1,
  WIDGET_WEATHER = 2,
  WIDGET_TEAMS = 3,
  WIDGET_STOCKS = 4,
  WIDGET_STATUS = 5,
  WIDGET_COUNTER = 6,
  WIDGET_TEMPERATURE = 7,
};

extern WidgetType currentWidget;

// Widget data structures
struct WeatherData
{
  String location;
  String region;
  String country;
  int temperature;
  bool isDay;
  String condition;
  String icon;
  int humidity;
  int windSpeed;
  String windDirection;
  uint32_t lastUpdate;
  bool dataValid;
};

struct TeamsData
{
  String status;
  String details;
  uint16_t statusColor;
  uint32_t lastUpdate;
};

struct StockData
{
  String symbol;
  float price;
  float change;
  bool isUp;
  uint32_t lastUpdate;
};

extern WeatherData currentWeather;
extern TeamsData currentTeams;
extern StockData currentStock;

// Widget functions
void initializeWidgets();
void updateWidgets();
void drawWidget(WidgetType widget, int x, int y, int width, int height);
void drawClockWidget(int x, int y, int width, int height);
void drawWeatherWidget(int x, int y, int width, int height);
void drawTeamsWidget(int x, int y, int width, int height);
void drawStocksWidget(int x, int y, int width, int height);
void resetWidgetZone(int x, int y, int width, int height);

// Widget setter
void setWidget(WidgetType widget);

// Data update functions
void updateWeatherData();
void updateTeamsData();
void updateStockData();

// Helper functions
String formatTime(bool is24Hour);
uint16_t getStatusColor(String status);

#endif
