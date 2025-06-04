#include "matrix_display.h"
#include "widgets.h"

// Color definitions
uint16_t colors[] = {
  0x0000,  // Black
  0xF800,  // Red
  0x07E0,  // Green
  0x001F,  // Blue
  0xFFE0,  // Yellow
  0xF81F,  // Magenta
  0x07FF,  // Cyan
  0xFFFF   // White
};

const char* colorNames[] = {
  "Black", "Red", "Green", "Blue", "Yellow", "Magenta", "Cyan", "White"
};

int truckPosition = WIDTH;
uint32_t lastTruckUpdate = 0;

void initializeMatrix() {
  Serial.println("Initializing LED matrix...");

  ProtomatterStatus status = matrix.begin();
  Serial.printf("Matrix status: %d ", status);

  switch (status) {
    case PROTOMATTER_OK:
      Serial.println("(SUCCESS)");
      break;
    case PROTOMATTER_ERR_PINS:
      Serial.println("(PIN ERROR - Check connections)");
      break;
    case PROTOMATTER_ERR_ARG:
      Serial.println("(ARGUMENT ERROR - Check configuration)");
      break;
    case PROTOMATTER_ERR_MALLOC:
      Serial.println("(MEMORY ERROR - Insufficient RAM)");
      break;
    default:
      Serial.println("(UNKNOWN ERROR)");
      break;
  }

  if (status != PROTOMATTER_OK) {
    Serial.println("Matrix failed to initialize!");
    while (1) delay(1000);
  }

  // Test matrix
  testMatrix();
}

void testMatrix() {
  Serial.println("Testing matrix...");

  // Test with dim colors for power savings
  matrix.fillScreen(matrix.color565(64, 64, 64));  // Dim white
  matrix.show();
  delay(500);

  matrix.fillScreen(matrix.color565(128, 0, 0));  // Dim red
  matrix.show();
  delay(300);

  matrix.fillScreen(matrix.color565(0, 128, 0));  // Dim green
  matrix.show();
  delay(300);

  matrix.fillScreen(matrix.color565(0, 0, 128));  // Dim blue
  matrix.show();
  delay(300);

  matrix.fillScreen(0);
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.color565(255, 255, 255));
  matrix.setCursor(0, HEIGHT / 2 - 4);
  matrix.print("READY");
  matrix.show();
  delay(1000);

  matrix.fillScreen(0);
  matrix.show();
  Serial.println("Matrix test complete");
}

void updateMatrixDisplay() {
  static uint32_t lastDebugOutput = 0;
  
  // Debug output every 2 seconds
  if (millis() - lastDebugOutput > 2000) {
    Serial.print("updateMatrixDisplay() - currentDisplayMode: ");
    Serial.println(currentDisplayMode);
    lastDebugOutput = millis();
  }
  
  switch (currentDisplayMode) {
    case PATTERN:
      animatePattern();
      break;
    case SCROLLING_TEXT:
      scrollText();
      break;
    case TRUCK_ANIMATION:
      animateTruck();
      break;
    case SMART_WIDGETS:
      updateSmartWidgets();
      break;
    case SOLID_COLOR:
    default:
      // Solid color mode - no animation needed
      break;
  }
}

// New function for smart widgets mode
void updateSmartWidgets() {
  // Clear the display
  matrix.fillScreen(0);
  
  // Update and draw widgets in top zones
  updateWidgets();
  
  // Draw current animation in bottom zone (y=16 to y=31)
  // You could have a mini truck animation or other content here
  drawBottomZoneAnimation();
  
  matrix.show();
}

void drawBottomZoneAnimation() {
  // Mini truck animation in bottom 64x16 area
  static int miniTruckPos = WIDTH;
  static uint32_t lastMiniUpdate = 0;
  
  if (millis() - lastMiniUpdate > 150) {
    // Clear bottom zone only
    matrix.fillRect(0, 16, WIDTH, 16, matrix.color565(0, 0, 0));
    
    // Draw mini truck (scaled down)
    int y = 20; // Center in bottom zone
    
    // Mini cab
    matrix.fillRect(miniTruckPos, y, 6, 4, matrix.color565(0, 100, 200));
    
    // Mini container
    matrix.fillRect(miniTruckPos + 6, y - 1, 12, 6, matrix.color565(220, 220, 220));
    
    // Mini logo
    matrix.fillRect(miniTruckPos + 9, y, 4, 3, matrix.color565(0, 180, 0));
    
    // Mini wheels
    matrix.drawPixel(miniTruckPos + 1, y + 4, matrix.color565(40, 40, 40));
    matrix.drawPixel(miniTruckPos + 14, y + 4, matrix.color565(40, 40, 40));
    
    miniTruckPos -= 2;
    if (miniTruckPos < -20) {
      miniTruckPos = WIDTH;
    }
    
    lastMiniUpdate = millis();
  }
}

// Web interface Setters
void setSmartWidgetsMode() {
  currentDisplayMode = SMART_WIDGETS;
  widgetsEnabled = true;
  Serial.println("Smart widgets mode activated");
}
void setMatrixColor(int colorIndex) {
  if (colorIndex >= 0 && colorIndex < 8) {
    currentDisplayMode = SOLID_COLOR;
    currentColor = colors[colorIndex];
    matrix.fillScreen(currentColor);
    matrix.show();
    Serial.printf("Color: %s (0x%04X)\n", colorNames[colorIndex], currentColor);
  }
}

void setMatrixPattern() {
  currentDisplayMode = PATTERN;
  patternFrame = 0;
  Serial.println("Pattern mode activated");
}

void setMatrixText(String text) {
  displayText = text;
  currentDisplayMode = SCROLLING_TEXT;
  scrollPosition = WIDTH;
  Serial.println("Text: " + displayText);
}

void clearMatrixDisplay() {
  currentDisplayMode = SOLID_COLOR;
  currentColor = 0;
  matrix.fillScreen(0);
  matrix.show();
  Serial.println("Display cleared");
}

void showMatrixIPAddress() {
  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
  matrix.setTextColor(matrix.color565(0, 255, 0));
  matrix.setTextSize(1);
  matrix.print("IP:");

  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);

  matrix.setCursor(0, 8);
  matrix.print(ipStr.substring(0, 11));
  if (ipStr.length() > 11) {
    matrix.setCursor(0, 16);
    matrix.print(ipStr.substring(11));
  }

  matrix.show();
  Serial.println("Matrix ready at: http://" + ipStr);
  delay(5000);

  matrix.fillScreen(0);
  matrix.setCursor(0, HEIGHT / 2 - 4);
  matrix.setTextColor(matrix.color565(255, 255, 255));
  matrix.print("READY");
  matrix.show();
}

void animatePattern() {
  if (millis() - lastUpdate > 150) {
    matrix.fillScreen(0);

    for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        int colorIndex = ((x + y + patternFrame) / 8) % 6 + 1;  // Skip black
        // Use dimmer colors for power savings
        uint16_t dimColor = colors[colorIndex];
        int r = ((dimColor >> 11) & 0x1F) >> 1;  // Half brightness
        int g = ((dimColor >> 5) & 0x3F) >> 1;
        int b = (dimColor & 0x1F) >> 1;
        matrix.drawPixel(x, y, matrix.color565(r << 3, g << 2, b << 3));
      }
    }

    matrix.show();
    patternFrame++;
    lastUpdate = millis();
  }
}

void scrollText() {
  if (millis() - lastUpdate > 120) {
    matrix.fillScreen(0);
    matrix.setTextWrap(false);
    matrix.setCursor(scrollPosition, HEIGHT / 2 - 4);
    matrix.setTextSize(1);
    matrix.setTextColor(matrix.color565(255, 255, 255));
    matrix.print(displayText);
    matrix.show();

    scrollPosition--;
    if (scrollPosition < -(int)(displayText.length() * 6)) {
      scrollPosition = WIDTH;
    }

    lastUpdate = millis();
  }
}

/* =======================
  Truck Logo and Animation
========================= */

void drawCompanyLogo(int x, int y) {
  // Simple green shield - no border, moved left for text to fit

  // Draw green shield (clean and simple)
  matrix.fillRect(x, y, 8, 5, matrix.color565(0, 180, 0));          // Green shield body
  matrix.fillRect(x + 1, y + 5, 6, 1, matrix.color565(0, 180, 0));  // Shield taper
  matrix.fillRect(x + 3, y + 6, 2, 1, matrix.color565(0, 180, 0));  // Shield point

  // IMC text in black
  matrix.setTextColor(matrix.color565(0, 0, 0));  // Black text
  matrix.setTextSize(1);
  matrix.setTextWrap(false);
  matrix.setCursor(x + 8, y);  // Text next to shield
  matrix.print("IMC");
}


void animateTruck() {
  if (millis() - lastTruckUpdate > 80) {  // Slightly faster animation
    matrix.fillScreen(0);

    // More detailed truck design

    // Truck cab (more realistic shape)
    matrix.fillRect(truckPosition, 20, 10, 8, matrix.color565(0, 100, 200));     // Blue cab
    matrix.fillRect(truckPosition + 1, 19, 8, 6, matrix.color565(0, 150, 255));  // Lighter blue windows

    // Cab details
    matrix.drawPixel(truckPosition + 2, 20, matrix.color565(200, 200, 255));  // Windshield
    matrix.drawPixel(truckPosition + 3, 20, matrix.color565(200, 200, 255));
    matrix.drawPixel(truckPosition + 4, 20, matrix.color565(200, 200, 255));
    matrix.drawLine(truckPosition + 5, 22, truckPosition + 5, 27, matrix.color565(0, 50, 150));  // Door line

    // Container/trailer (larger and more detailed)
    matrix.fillRect(truckPosition + 10, 16, 28, 12, matrix.color565(220, 220, 220));  // Light gray container
    matrix.drawRect(truckPosition + 10, 16, 28, 12, matrix.color565(180, 180, 180));  // Container border

    // Container details (corrugated sides)
    matrix.drawLine(truckPosition + 13, 16, truckPosition + 13, 27, matrix.color565(180, 180, 180));
    matrix.drawLine(truckPosition + 17, 16, truckPosition + 17, 27, matrix.color565(180, 180, 180));
    matrix.drawLine(truckPosition + 21, 16, truckPosition + 21, 27, matrix.color565(180, 180, 180));
    matrix.drawLine(truckPosition + 25, 16, truckPosition + 25, 27, matrix.color565(180, 180, 180));
    matrix.drawLine(truckPosition + 29, 16, truckPosition + 29, 27, matrix.color565(180, 180, 180));

    // Company logo on container (moved further left)
    if (truckPosition + 12 >= -30) {
      drawCompanyLogo(truckPosition + 12, 18);
    }

    // More realistic wheels - repositioned for longer trailer
    // Front wheel (cab)
    matrix.fillRect(truckPosition + 2, 28, 3, 3, matrix.color565(40, 40, 40));  // Tire
    matrix.drawPixel(truckPosition + 3, 29, matrix.color565(120, 120, 120));    // Rim

    // Middle wheel (trailer front)
    matrix.fillRect(truckPosition + 14, 28, 3, 3, matrix.color565(40, 40, 40));  // Tire
    matrix.drawPixel(truckPosition + 15, 29, matrix.color565(120, 120, 120));    // Rim

    // Back wheel (trailer rear) - moved to end of longer container
    matrix.fillRect(truckPosition + 28, 28, 3, 3, matrix.color565(40, 40, 40));  // Tire
    matrix.drawPixel(truckPosition + 29, 29, matrix.color565(120, 120, 120));    // Rim

    // Connection between cab and trailer
    matrix.drawLine(truckPosition + 9, 22, truckPosition + 11, 22, matrix.color565(100, 100, 100));

    // Headlights
    matrix.drawPixel(truckPosition - 1, 25, matrix.color565(255, 255, 200));  // Headlight
    matrix.drawPixel(truckPosition - 1, 27, matrix.color565(255, 255, 200));  // Headlight

    // Tail lights - moved to end of longer container
    matrix.drawPixel(truckPosition + 38, 25, matrix.color565(255, 0, 0));  // Red tail light (top)
    matrix.drawPixel(truckPosition + 38, 27, matrix.color565(255, 0, 0));  // Red tail light (bottom)

    // Exhaust stack behind cab
    matrix.fillRect(truckPosition + 8, 17, 1, 4, matrix.color565(60, 60, 60));  // Stack body
    matrix.drawPixel(truckPosition + 8, 16, matrix.color565(80, 80, 80));       // Stack top
    matrix.drawPixel(truckPosition + 8, 15, matrix.color565(100, 100, 100));    // Stack cap

    // exhaust smoke
    if (millis() % 500 < 250) {  // Flashing smoke effect
      matrix.drawPixel(truckPosition + 10, 13, matrix.color565(80, 80, 80));
      matrix.drawPixel(truckPosition + 9, 14, matrix.color565(60, 60, 60));
    }

    matrix.show();

    truckPosition -= 1;         // Slower movement for better visibility
    if (truckPosition < -36) {  // Truck fully off screen (adjusted for longer truck)
      truckPosition = WIDTH;
    }

    lastTruckUpdate = millis();
  }
}

void setTruckAnimation() {
  currentDisplayMode = TRUCK_ANIMATION;
  truckPosition = WIDTH;  // Reset truck position
  Serial.println("Truck animation activated");
}
