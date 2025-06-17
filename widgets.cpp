#include "config.h"
#include "widgets.h"
#include "matrix_display.h"

// Widget state variables
WidgetType currentWidget = WIDGET_WEATHER;

// Widget data
WeatherData currentWeather = {"Memphis", "TN", "US", 70, true, "Sunny", "Sun", 100, 20, "NW", 0, false};
TeamsData currentTeams = {"Available", "", 0x07E0, 0}; // Green
StockData currentStock = {"AAPL", 150.25, 2.50, true, 0};
SpotifyTrackData currentSpotifyTrack = {"No Track", "No Artist", "", 0, 0, false, "", false, 0};

// Spotify widget state variables
uint32_t lastSpotifyUpdate = 0;
uint32_t lastSpotifyScroll = 0;
int spotifyTitleScroll = 0;
int spotifyArtistScroll = 0;
bool spotifyScrollDirection = true;
uint32_t lastSpotifyProgress = 0;

void initializeWidgets()
{
    Serial.println("Initializing widgets...");

    currentWeather.lastUpdate = 0;
    currentTeams.lastUpdate = 0;
    currentStock.lastUpdate = 0;
    lastSpotifyUpdate = 0;

    Serial.println("Widgets initialized");
}

// Update widget drawing to use only the widget zone (y=0-14)
void drawWidget(WidgetType widget, int x, int y, int width, int height)
{
    // Ensure widgets stay in widget zone
    if (y + height > WIDGET_ZONE_HEIGHT)
    {
        height = WIDGET_ZONE_HEIGHT - y;
    }

    switch (widget)
    {
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
        case WIDGET_SPOTIFY:
            drawSpotifyWidget(x, y, width, height);
            break;
        case WIDGET_NONE:
        default:
            resetWidgetZone(x, y, width, height);
            break;
    }
}

void updateWidgets()
{
    uint32_t now = millis();

    // Only update weather data if weather widget is selected
    if (currentWidget == WIDGET_WEATHER)
    {
        if (now - currentWeather.lastUpdate > 600000 || currentWeather.lastUpdate == 0)
        {
            updateWeatherData();
        }
    }

    // Only update teams data if teams widget is selected
    if (currentWidget == WIDGET_TEAMS)
    {
        if (now - currentTeams.lastUpdate > 30000)
        {
            updateTeamsData();
        }
    }

    // Only update stock data if stock widget is selected
    if (currentWidget == WIDGET_STOCKS)
    {
        if (now - currentStock.lastUpdate > 60000)
        {
            updateStockData();
        }
    }

    // Only update Spotify data if Spotify widget is selected
    if (currentWidget == WIDGET_SPOTIFY)
    {
        if (now - lastSpotifyUpdate > 10000 || lastSpotifyUpdate == 0)
        {
            updateSpotifyData();
        }
    }
}

void drawClockWidget(int x, int y, int width, int height)
{
    // Add a simple test rectangle to verify drawing works
    matrix.fillRect(x, y, width, height, matrix.color565(64, 0, 0)); // Dark red background

    matrix.setCursor(x, y + 8);
    matrix.setTextColor(0x001F); // Blue
    matrix.setTextSize(1);

    String timeStr = formatTime(false); // 12-hour format
    matrix.print(timeStr);
}

String formatTime(bool is24Hour)
{
    // Placeholder implementation
    uint32_t now = millis();
    int hours = (now / 3600000) % 24;
    int minutes = (now / 60000) % 60;

    if (!is24Hour)
    {
        bool isPM = hours >= 12;
        if (hours > 12) hours -= 12;
        if (hours == 0) hours = 12;
        return String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes) + (isPM ? "P" : "A");
    }
    else
    {
        return String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);
    }
}

void drawTeamsWidget(int x, int y, int width, int height)
{
    uint16_t dimColor = matrix.color565(
            ((currentTeams.statusColor >> 11) & 0x1F) << 1, // Dim red
            ((currentTeams.statusColor >> 5) & 0x3F) << 1, // Dim green
            (currentTeams.statusColor & 0x1F) << 1 // Dim blue
    );

    matrix.setCursor(x, y + 8);
    matrix.setTextColor(dimColor);
    matrix.setTextSize(1);

    // Show first few characters of status
    String displayText = currentTeams.status;
    if (displayText.length() > 8)
    {
        displayText = displayText.substring(0, 8);
    }
    matrix.print(displayText);
}

void drawStocksWidget(int x, int y, int width, int height)
{
    matrix.setCursor(x, y + 4);
    matrix.setTextColor(currentStock.isUp ? 0x07E0 : 0xF800); // Green if up, red if down
    matrix.setTextSize(1);

    // Show symbol on first line
    matrix.print(currentStock.symbol);

    // Show price on second line
    matrix.setCursor(x, y + 12);
    matrix.print("$" + String(currentStock.price, 1));
}

void resetWidgetZone(int x, int y, int width, int height)
{
    // Clear the entire widget zone
    matrix.fillRect(x, y, width, height, matrix.color565(0, 0, 0)); // Black background
    matrix.setTextWrap(true);
    matrix.setTextSize(1);
    matrix.setTextColor(matrix.color565(255, 255, 255)); // White text
}

void updateTeamsData()
{
    // Simulate teams status update
    String statuses[] = {"Available", "In Meeting", "Busy", "Away"};
    currentTeams.status = statuses[random(0, 4)];
    currentTeams.statusColor = getStatusColor(currentTeams.status);
    currentTeams.lastUpdate = millis();
    Serial.println("Teams data updated");
}

uint16_t getStatusColor(String status)
{
    if (status == "Available") return 0x07E0; // Green
    if (status == "Busy") return 0xF800; // Red
    if (status == "Away") return 0xFFE0; // Yellow
    return 0x001F; // Blue for "In Meeting"
}

void updateStockData()
{
    // Simulate stock data update
    currentStock.change = random(-500, 500) / 100.0; // -5.00 to +5.00
    currentStock.isUp = currentStock.change >= 0;
    currentStock.price += currentStock.change;
    if (currentStock.price < 50) currentStock.price = 50; // Minimum price
    currentStock.lastUpdate = millis();
    Serial.println("Stock data updated");
}

void setWidget(WidgetType widget)
{
    currentWidget = widget;
    Serial.println("Widget set to: " + String(widget));
}