#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include "hardware_config.h"
#include "display_modes.h"
#include <WiFiNINA.h>

// Global variables
extern Adafruit_Protomatter matrix;
extern AnimationType currentAnimation;  // Changed from DisplayMode
extern String displayText;
extern uint16_t currentColor;
extern int scrollPosition;
extern uint32_t lastUpdate;
extern int patternFrame;

// Matrix management
void initializeMatrix();
void testMatrix();
void updateMatrixDisplay();

// Animation zone functions (y=15-31)
void updateAnimationZone();
void animatePattern();
void scrollText();
void animateTruck();
void drawSolidColor();

// Web interface setters
void setAnimation(AnimationType animation);
void setAnimationColor(int colorIndex);
void setAnimationPattern();
void setAnimationText(String text);
void setTruckAnimation();
void clearAnimationZone();

// Utility functions
void showMatrixIPAddress();
void drawCompanyLogo(int x, int y);

#endif