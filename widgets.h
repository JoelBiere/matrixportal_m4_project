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
    WIDGET_SPOTIFY = 5,     // NEW: Spotify now playing
    WIDGET_STATUS = 6,
    WIDGET_COUNTER = 7,
    WIDGET_TEMPERATURE = 8,
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

// Spotify data structure
struct SpotifyTrackData {
    String trackName;
    String artistName;
    String albumName;
    int durationMs;
    int progressMs;
    bool isPlaying;
    String deviceName;
    bool dataValid;
    uint32_t lastUpdate;
};

extern WeatherData currentWeather;
extern TeamsData currentTeams;
extern StockData currentStock;
extern SpotifyTrackData currentSpotifyTrack;

// Spotify widget timing variables
extern uint32_t lastSpotifyUpdate;
extern uint32_t lastSpotifyScroll;
extern int spotifyTitleScroll;
extern int spotifyArtistScroll;
extern bool spotifyScrollDirection;
extern uint32_t lastSpotifyProgress;

// Widget functions
void initializeWidgets();
void updateWidgets();
void drawWidget(WidgetType widget, int x, int y, int width, int height);
void drawClockWidget(int x, int y, int width, int height);
void drawWeatherWidget(int x, int y, int width, int height);
void drawTeamsWidget(int x, int y, int width, int height);
void drawStocksWidget(int x, int y, int width, int height);
void drawSpotifyWidget(int x, int y, int width, int height);
void resetWidgetZone(int x, int y, int width, int height);
bool exchangeCodeForTokens(String authCode);

// Widget setter
void setWidget(WidgetType widget);

// Data update functions
void updateWeatherData();
void updateTeamsData();
void updateStockData();
void updateSpotifyData();

// Spotify specific functions
void setSpotifyTokens(String accessToken, String refreshToken);
String getSpotifyAuthURL();
bool exchangeCodeForTokens(String authCode);
bool refreshSpotifyToken();


void drawSpotifyLogo(int x, int y);
void drawPlayingBars(int x, int y, uint16_t color);
void drawSpotifyProgressBar(int x, int y, int width, int progress, int total);
String base64Encode(String input);



// weather animations
void drawWeatherBackground(int x, int y, int width, int height);
void drawWeatherElements(int x, int y, int width, int height);
void drawWeatherText(int x, int y, int width, int height);
void drawStars(int x, int y, int width, int height);
void drawAnimatedSun(int x, int y);
void drawMoon(int x, int y);
void drawAnimatedClouds(int x, int y, int width, int height, bool heavy);
void drawCloudShape(int x, int y, uint16_t color);
void drawAnimatedRain(int x, int y, int width, int height);
void drawAnimatedSnow(int x, int y, int width, int height);
void drawLightning(int x, int y, int width, int height);
void drawWeatherWidgetCore(int x, int y, int width, int height);

// weather animation DEBUG mode
void setWeatherDebugMode(bool enabled);
void advanceDebugWeather();
String getDebugWeatherInfo();

// Helper functions
String formatTime(bool is24Hour);
uint16_t getStatusColor(String status);

#endif