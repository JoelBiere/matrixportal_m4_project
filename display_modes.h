#ifndef DISPLAY_MODES_H
#define DISPLAY_MODES_H

#include <Arduino.h>
#include "hardware_config.h"

// Display modes for the bottom animation zone
enum DisplayMode {
  SOLID_COLOR = 0,
  PATTERN = 1,  
  SCROLLING_TEXT = 2,
  TRUCK_ANIMATION = 3,
  SMART_WIDGETS = 4
};

// Display zones
enum DisplayZone {
  ZONE_TOP_LEFT = 0,
  ZONE_TOP_RIGHT = 1,
  ZONE_BOTTOM = 2  // Animation zone
};

// Display state variables
extern DisplayMode currentDisplayMode;
extern String displayText;
extern uint16_t currentColor;
extern int scrollPosition;
extern uint32_t lastUpdate;
extern int patternFrame;

// Truck animation state
extern int truckPosition;
extern uint32_t lastTruckUpdate;

// Display mode functions
void initializeDisplayModes();
void updateDisplayMode();
void setDisplayMode(DisplayMode mode);
void drawSolidColor(uint16_t color);
void drawPattern();
void drawScrollingText();
void drawTruckAnimation();

#endif