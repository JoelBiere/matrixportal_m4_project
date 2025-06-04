@echo off
arduino-cli compile --upload -p COM4 --fqbn adafruit:samd:adafruit_matrixportal_m4 .
if %errorlevel% equ 0 (
    echo Upload successful! Starting monitor...
    arduino-cli monitor -p COM4 -c baudrate=115200
) else (
    echo Upload failed!
)