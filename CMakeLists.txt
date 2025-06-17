cmake_minimum_required(VERSION 3.16)
project(matrixportal_m4_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

# Arduino paths - adjust these to match your system
set(SYSTEM_USER "jbiere")
set(ARDUINO_PATH "C:/Users/${SYSTEM_USER}/Documents/Arduino")
set(ARDUINO_LIBRARIES_PATH "${ARDUINO_PATH}/libraries")

# Add project source directory for local headers
include_directories(${CMAKE_SOURCE_DIR})

# Only include user libraries for IDE support
# Core libraries are handled by arduino-cli during compilation
include_directories(
        # User-installed libraries from Documents/Arduino/libraries
        ${ARDUINO_LIBRARIES_PATH}/WiFiNINA/src
        ${ARDUINO_LIBRARIES_PATH}/Adafruit_Protomatter/src
        ${ARDUINO_LIBRARIES_PATH}/Adafruit_BusIO
        ${ARDUINO_LIBRARIES_PATH}/Adafruit_GFX_Library
        ${ARDUINO_LIBRARIES_PATH}/ArduinoJson/src
        ${ARDUINO_LIBRARIES_PATH}/FreeRTOS_SAMD51/src    # NEW: FreeRTOS support
)

# Add all your source files
set(SOURCES
        # Main Arduino sketch
        matrixportal_m4_project.ino

        # Your custom modules
        wifi_manager.cpp
        web_server.cpp
        matrix_display.cpp
        widgets.cpp
        credentials.cpp
        weather_widget.cpp
        spotify_widget.cpp    # NEW: Spotify widget file
        matrix_config.cpp

)

# Add header files explicitly for better IDE support
set(HEADERS
        config.h
        credentials.h
        display_modes.h
        hardware_config.h
        matrix_display.h
        web_server.h
        widgets.h
        wifi_manager.h
)

# Create a mock Arduino.h for IDE support
file(WRITE ${CMAKE_BINARY_DIR}/Arduino.h
        "#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <string>

// Mock Arduino types and classes for IDE support
class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    String(int n) : std::string(std::to_string(n)) {}
    String(float f, int decimals = 2) : std::string(std::to_string(f)) {}

    int length() const { return size(); }
    String substring(int start) const { return substr(start); }
    String substring(int start, int end) const { return substr(start, end - start); }
    int indexOf(const String& s) const { return find(s); }
    int indexOf(const String& s, int start) const { return find(s, start); }
    int indexOf(const char* s) const { return find(s); }
    String replace(const String& from, const String& to) const {
        String result = *this;
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
        return result;
    }
    int toInt() const { return std::stoi(*this); }
    float toFloat() const { return std::stof(*this); }
};

// Mock Serial class
class HardwareSerial {
public:
    void begin(int baud) {}
    void println(const char* s) {}
    void println(const String& s) {}
    void println(int n) {}
    void print(const char* s) {}
    void print(const String& s) {}
    void print(int n) {}
    void printf(const char* format, ...) {}
    operator bool() { return true; }
};

extern HardwareSerial Serial;

// Mock Arduino functions
unsigned long millis();
void delay(unsigned long ms);
int random(int max);
int random(int min, int max);

// Mock WiFi constants
#define WL_IDLE_STATUS 0
#define WL_NO_SSID_AVAIL 1
#define WL_SCAN_COMPLETED 2
#define WL_CONNECTED 3
#define WL_CONNECT_FAILED 4
#define WL_CONNECTION_LOST 5
#define WL_DISCONNECTED 6
#define WL_NO_MODULE 255

// Basic Arduino types
typedef uint8_t byte;

#endif
")

# Create mock Adafruit_Protomatter.h for IDE support
file(WRITE ${CMAKE_BINARY_DIR}/Adafruit_Protomatter.h
        "#ifndef ADAFRUIT_PROTOMATTER_H
#define ADAFRUIT_PROTOMATTER_H

#include <stdint.h>

// Mock Adafruit_Protomatter class for IDE support
class Adafruit_Protomatter {
public:
    Adafruit_Protomatter(int width, int bit_planes, int rgbcount,
                        uint8_t* rgbpins, int addr_count, uint8_t* addrpins,
                        uint8_t clockpin, uint8_t latchpin, uint8_t oepin, bool dbuf) {}

    void begin() {}
    void show() {}
    void fillScreen(uint16_t color) {}
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {}
    void drawPixel(int16_t x, int16_t y, uint16_t color) {}
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {}
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {}
    void setCursor(int16_t x, int16_t y) {}
    void setTextColor(uint16_t color) {}
    void setTextSize(uint8_t size) {}
    void setTextWrap(bool wrap) {}
    void print(const char* text) {}
    void print(const String& text) {}
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return 0; }
    int16_t width() { return 64; }
    int16_t height() { return 32; }
};

enum ProtomatterStatus {
    PROTOMATTER_OK = 0,
    PROTOMATTER_ERR_PINS = 1,
    PROTOMATTER_ERR_ARG = 2,
    PROTOMATTER_ERR_MALLOC = 3
};

#endif
")

# Create mock WiFiNINA.h for IDE support
file(WRITE ${CMAKE_BINARY_DIR}/WiFiNINA.h
        "#ifndef WIFININA_H
#define WIFININA_H

#include <stdint.h>

class IPAddress {
public:
    IPAddress() {}
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {}
    uint8_t operator[](int index) const { return 192; } // Mock IP
};

class WiFiClient {
public:
    bool connect(const char* host, int port) { return true; }
    void stop() {}
    bool connected() { return true; }
    int available() { return 0; }
    char read() { return 0; }
    void print(const char* s) {}
    void println(const char* s) {}
    String readStringUntil(char terminator) { return \"\"; }
};

class WiFiSSLClient : public WiFiClient {
public:
    WiFiSSLClient() : WiFiClient() {}
    void setTimeout(int timeout) {}
};

class WiFiServer {
public:
    WiFiServer(int port) {}
    void begin() {}
    WiFiClient available() { return WiFiClient(); }
};

class WiFiClass {
public:
    int begin(const char* ssid, const char* pass) { return WL_CONNECTED; }
    int status() { return WL_CONNECTED; }
    IPAddress localIP() { return IPAddress(192, 168, 1, 100); }
    const char* SSID() { return \"MockSSID\"; }
    long RSSI() { return -50; }
    const char* firmwareVersion() { return \"1.0.0\"; }
    int scanNetworks() { return 1; }
    const char* SSID(int index) { return \"MockNetwork\"; }
    long RSSI(int index) { return -60; }
    void disconnect() {}
};

extern WiFiClass WiFi;

#endif
")

# Create mock ArduinoJson.h for IDE support
file(WRITE ${CMAKE_BINARY_DIR}/ArduinoJson.h
        "#ifndef ARDUINOJSON_H
#define ARDUINOJSON_H

#include <string>
#include <map>

class JsonObject {
public:
    template<typename T>
    T as() const { return T(); }

    JsonObject operator[](const char* key) const { return JsonObject(); }
    bool isNull() const { return false; }
};

class JsonArray {
public:
    int size() const { return 1; }
    JsonObject operator[](int index) const { return JsonObject(); }
};

enum DeserializationError {
    Ok = 0
};

template<int SIZE>
class StaticJsonDocument {
public:
    JsonObject as<JsonObject>() { return JsonObject(); }
    JsonObject operator[](const char* key) { return JsonObject(); }
};

typedef StaticJsonDocument<2048> DynamicJsonDocument;
typedef StaticJsonDocument<2048> JsonDocument;

DeserializationError deserializeJson(DynamicJsonDocument& doc, const std::string& input) {
    return Ok;
}

DeserializationError deserializeJson(JsonDocument& doc, const std::string& input) {
    return Ok;
}

#endif
")

# Create mock FreeRTOS_SAMD51.h for IDE support
file(WRITE ${CMAKE_BINARY_DIR}/FreeRTOS_SAMD51.h
        "#ifndef FREERTOS_SAMD51_H
#define FREERTOS_SAMD51_H

#include <stdint.h>

// Mock FreeRTOS types and constants for IDE support
typedef void* TaskHandle_t;
typedef void* SemaphoreHandle_t;
typedef uint32_t TickType_t;
typedef int BaseType_t;

// FreeRTOS constants
#define pdPASS 1
#define pdFAIL 0
#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY 0xFFFFFFFF

// Task priorities
#define tskIDLE_PRIORITY 0
#define configMAX_PRIORITIES 5

// Time conversion macros
#define pdMS_TO_TICKS(ms) (ms)
#define portTICK_PERIOD_MS 1

// Mock FreeRTOS functions
BaseType_t xTaskCreate(void (*taskFunction)(void*), const char* name,
                      uint16_t stackSize, void* parameters,
                      uint8_t priority, TaskHandle_t* handle) { return pdPASS; }

void vTaskStartScheduler() {}
void vTaskDelay(TickType_t ticks) {}
void vTaskDelayUntil(TickType_t* previousWakeTime, TickType_t increment) {}
TickType_t xTaskGetTickCount() { return 0; }

// Semaphore functions
SemaphoreHandle_t xSemaphoreCreateMutex() { return (SemaphoreHandle_t)1; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t timeout) { return pdPASS; }
BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore) { return pdPASS; }

// Memory functions
size_t xPortGetFreeHeapSize() { return 50000; }

#endif
")

# Add the mock directory to includes
include_directories(${CMAKE_BINARY_DIR})

# Create executable (this won't actually build for the target, but helps with IDE support)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set target-specific include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${ARDUINO_LIBRARIES_PATH}/WiFiNINA/src
        ${ARDUINO_LIBRARIES_PATH}/Adafruit_Protomatter/src
        ${ARDUINO_LIBRARIES_PATH}/Adafruit_BusIO
        ${ARDUINO_LIBRARIES_PATH}/Adafruit_GFX_Library
        ${ARDUINO_LIBRARIES_PATH}/ArduinoJson/src
        ${ARDUINO_LIBRARIES_PATH}/FreeRTOS_SAMD51/src
)

# Arduino-specific definitions that are safe for IDE analysis
target_compile_definitions(${PROJECT_NAME} PRIVATE
        ARDUINO=10819
        ARDUINO_SAMD_MATRIXPORTAL_M4
        ARDUINO_ARCH_SAMD
        __SAMD51__
        F_CPU=120000000L
        ADAFRUIT_MATRIXPORTAL_M4_EXPRESS
        USB_VID=0x239A
        USB_PID=0x80F1
        USB_PRODUCT="MatrixPortal M4"
        USB_MANUFACTURER="Adafruit"
        BOARD_NAME="MatrixPortal M4"
        VARIANT_QSPI_BAUD_DEFAULT=50000000
        ENABLE_CACHE
        # FreeRTOS definitions for IDE
        FREERTOS_SAMD51
        configUSE_16_BIT_TICKS=0
        configMAX_PRIORITIES=5
        configMINIMAL_STACK_SIZE=128
)

# Set C++ standard for the target
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
)

# Set file associations for CLion - treat .ino files as C++
set_source_files_properties(
        matrixportal_m4_project.ino
        PROPERTIES
        LANGUAGE CXX
)

# Enable compile commands export for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Custom target for arduino-cli compilation
add_custom_target(arduino_compile
        COMMAND arduino-cli compile --fqbn adafruit:samd:adafruit_matrixportal_m4 ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling with arduino-cli"
)

add_custom_target(arduino_upload
        COMMAND arduino-cli compile --upload -p COM8 --fqbn adafruit:samd:adafruit_matrixportal_m4 ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Uploading with arduino-cli"
        DEPENDS arduino_compile
)

add_custom_target(deploy
        COMMAND ${CMAKE_SOURCE_DIR}/deploy.bat
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running deploy.bat script"
)

# FreeRTOS compatibility test target
add_custom_target(freertos_test
        COMMAND echo "Testing FreeRTOS compatibility..."
        COMMAND arduino-cli compile --fqbn adafruit:samd:adafruit_matrixportal_m4 --library FreeRTOS_SAMD51 ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Testing FreeRTOS compilation compatibility"
)