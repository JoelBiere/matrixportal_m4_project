#ifndef DISPLAY_MODES_H
#define DISPLAY_MODES_H

// Animation types for the main animation zone (y=15-31)
enum AnimationType {
  ANIMATION_NONE = 0,
  ANIMATION_SOLID_COLOR = 1,
  ANIMATION_PATTERN = 2,
  ANIMATION_SCROLLING_TEXT = 3,
  ANIMATION_TRUCK = 4
};

// Legacy enum for compatibility (can be removed later)
enum DisplayMode {
  SOLID_COLOR = 0,
  PATTERN = 1,
  SCROLLING_TEXT = 2,
  TRUCK_ANIMATION = 3,
  SMART_WIDGETS = 4
};

// Widget zone is always available (y=0-14)
// Widgets are controlled independently via widgetsEnabled

// Display zones
#define WIDGET_ZONE_HEIGHT 15      // y = 0 to 14
#define ANIMATION_ZONE_Y 15        // y = 15 to 31
#define ANIMATION_ZONE_HEIGHT 17   // 17 pixels tall

#endif