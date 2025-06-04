#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiNINA.h>
#include <Adafruit_Protomatter.h>

// WiFi credentials
extern char ssid[];
extern char pass[];

// Matrix configuration
#define HEIGHT 32
#define WIDTH 64

// Pin definitions for Adafruit Matrix Portal M4
extern uint8_t rgbPins[];
extern uint8_t addrPins[];
extern uint8_t clockPin;
extern uint8_t latchPin;
extern uint8_t oePin;

// Display modes
enum DisplayMode {
  SOLID_COLOR = 0,
  PATTERN = 1,  
  SCROLLING_TEXT = 2,
  TRUCK_ANIMATION = 3,
  SMART_WIDGETS = 4
};


enum DisplayZone {
  ZONE_TOP_LEFT = 0,
  ZONE_TOP_RIGHT = 1,
  ZONE_BOTTOM = 2  // Animation zone
};



// Global objects
extern WiFiServer server;
extern Adafruit_Protomatter matrix;
extern int wifiStatus;

extern DisplayMode currentDisplayMode;
extern String displayText;
extern uint16_t currentColor;
extern int scrollPosition;
extern uint32_t lastUpdate;
extern int patternFrame;

// truck animation 
extern int truckPosition;
extern uint32_t lastTruckUpdate;

#endif