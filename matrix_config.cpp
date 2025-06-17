#include "hardware_config.h"

// Pin definitions for Adafruit Matrix Portal M4
uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20, 21};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

// Global matrix object definition
Adafruit_Protomatter matrix(
        WIDTH, 3, 1, rgbPins, 4, addrPins,
        clockPin, latchPin, oePin, true
);