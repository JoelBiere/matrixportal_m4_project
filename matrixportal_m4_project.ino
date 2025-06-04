
#include "config.h"
#include "wifi_manager.h"
#include "web_server.h"  
#include "matrix_display.h"
#include "widgets.h"

// DEFINE GLOBAL VARIABLES HERE (not just declare them)
WiFiServer server(80);
int wifiStatus = WL_IDLE_STATUS;

// Matrix configuration
uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20, 21};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

Adafruit_Protomatter matrix(
  WIDTH, 3, 1, rgbPins, 4, addrPins,
  clockPin, latchPin, oePin, true);

// Display state variables
DisplayMode currentDisplayMode = SOLID_COLOR;
String displayText = "Hello Matrix!";
uint16_t currentColor = 0;
int scrollPosition = WIDTH;
uint32_t lastUpdate = 0;
int patternFrame = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  
  Serial.println("=== MatrixPortal M4 Multi-File Project ===");
  
  // Initialize modules
  initializeMatrix();
  initializeWidgets();
  initializeWiFi();
  initializeWebServer();
  
  Serial.println("Setup complete!");
}

void loop() {
  handleWebClients();
  updateMatrixDisplay();
}
