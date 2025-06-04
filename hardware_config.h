#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <Adafruit_Protomatter.h>

// Matrix hardware configuration
#define HEIGHT 32
#define WIDTH 64

// Pin definitions for Adafruit Matrix Portal M4
extern uint8_t rgbPins[];
extern uint8_t addrPins[];
extern uint8_t clockPin;
extern uint8_t latchPin;
extern uint8_t oePin;

// Global matrix object
extern Adafruit_Protomatter matrix;

#endif