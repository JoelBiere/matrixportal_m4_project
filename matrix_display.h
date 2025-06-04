#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include "config.h"
#include "widgets.h"  // Add this include

// Function declarations
void initializeMatrix();
void updateMatrixDisplay();
void setMatrixColor(int colorIndex);
void setMatrixPattern();
void setMatrixText(String text);
void clearMatrixDisplay();
void showMatrixIPAddress();
void testMatrix();
void drawCompanyLogo(int x, int y);
void setTruckAnimation();            

// Widget stuff
void updateSmartWidgets();
void drawBottomZoneAnimation();
void setSmartWidgetsMode();

// Internal animation functions
void animatePattern();
void scrollText();
void animateTruck();

#endif