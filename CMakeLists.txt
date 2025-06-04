cmake_minimum_required(VERSION 3.16)
project(matrixportal_m4_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

# Arduino paths - adjust these to match your system
set(ARDUINO_PATH "C:/Users/joelb/AppData/Local/Arduino15")
set(ARDUINO_CORE_PATH "${ARDUINO_PATH}/packages/adafruit/hardware/samd/1.7.11")
set(ARDUINO_LIBRARIES_PATH "C:/Users/joelb/Documents/Arduino/libraries")

# Add project source directory for local headers
include_directories(${CMAKE_SOURCE_DIR})

# Arduino core include directories
include_directories(
    ${ARDUINO_CORE_PATH}/cores/arduino
    ${ARDUINO_CORE_PATH}/cores/arduino/api/deprecated
    ${ARDUINO_CORE_PATH}/cores/arduino/api/deprecated-avr-comp
    ${ARDUINO_CORE_PATH}/variants/matrixportal_m4
    ${ARDUINO_CORE_PATH}/libraries/SPI/src
    ${ARDUINO_CORE_PATH}/libraries/Wire/src
)

# Arduino library include directories
include_directories(
    # WiFiNINA library
    ${ARDUINO_LIBRARIES_PATH}/WiFiNINA/src
    
    # Adafruit libraries - add the specific header path
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_Protomatter
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_Protomatter/src
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_BusIO
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_GFX_Library
    
    # Other common libraries that might be needed
    ${ARDUINO_LIBRARIES_PATH}/SPI/src
    ${ARDUINO_LIBRARIES_PATH}/Wire/src
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
};

// Mock Serial class
class HardwareSerial {
public:
    void begin(int baud) {}
    void println(const char* s) {}
    void println(const String& s) {}
    void print(const char* s) {}
    void print(const String& s) {}
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
    void drawPixel(int16_t x, int16_t y, uint16_t color) {}
    void setCursor(int16_t x, int16_t y) {}
    void setTextColor(uint16_t color) {}
    void setTextSize(uint8_t size) {}
    void print(const char* text) {}
    void print(const String& text) {}
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return 0; }
    int16_t width() { return 64; }
    int16_t height() { return 32; }
};

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
    ${ARDUINO_CORE_PATH}/cores/arduino
    ${ARDUINO_CORE_PATH}/cores/arduino/api/deprecated
    ${ARDUINO_CORE_PATH}/variants/matrixportal_m4
    ${ARDUINO_CORE_PATH}/libraries/SPI/src
    ${ARDUINO_CORE_PATH}/libraries/Wire/src
    ${ARDUINO_LIBRARIES_PATH}/WiFiNINA/src
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_Protomatter
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_Protomatter/src
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_BusIO
    ${ARDUINO_LIBRARIES_PATH}/Adafruit_GFX_Library
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

# Optional: Set up custom target for arduino-cli compilation
add_custom_target(arduino_compile
    COMMAND arduino-cli compile --fqbn adafruit:samd:adafruit_matrixportal_m4 ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Compiling with arduino-cli"
)

add_custom_target(arduino_upload
    COMMAND arduino-cli compile --upload -p COM4 --fqbn adafruit:samd:adafruit_matrixportal_m4 ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Uploading with arduino-cli"
    DEPENDS arduino_compile
)

add_custom_target(deploy
    COMMAND ${CMAKE_SOURCE_DIR}/deploy.bat
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running deploy.bat script"
)