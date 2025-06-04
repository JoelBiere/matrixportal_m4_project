#ifndef CONFIG_H
#define CONFIG_H

// Main configuration header - includes all subsystem headers
#include "hardware_config.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "display_modes.h"
#include "widgets.h"

// Global configuration constants
#define SERIAL_BAUD_RATE 115200
#define WIFI_TIMEOUT_MS 10000
#define DISPLAY_UPDATE_INTERVAL 50  // ms
#define WIDGET_UPDATE_INTERVAL 1000 // ms

#endif