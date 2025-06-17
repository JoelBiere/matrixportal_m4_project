#ifndef TEAMS_WIDGET_H
#define TEAMS_WIDGET_H

#include "widgets.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>

// Function to get Teams status color
uint16_t getTeamsStatusColor(String status);

// Function to draw Teams presence icon
void drawPresenceIcon(int x, int y, uint16_t color);

// Function to update Teams data from Microsoft Graph API
void updateTeamsData();

// Function to draw the Teams widget
void drawTeamsWidget(int x, int y, int width, int height);

#endif // TEAMS_WIDGET_H
