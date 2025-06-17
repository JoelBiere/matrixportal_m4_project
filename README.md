use arduino-cli to compile and upload
# Matrix Portal M4 Project

## Microsoft Teams Presence Setup

To use the Microsoft Teams presence widget, you'll need to:

1. Register an app in the Microsoft Azure Portal (https://portal.azure.com)
   - Add the "Presence.Read" API permission
   - Set up a client secret
   - Configure a redirect URI matching your device's IP address

2. Add your Microsoft Graph credentials in your secrets file:
   ```c
   // In credentials.h
   char msGraphClientId[] = "your-client-id";
   char msGraphClientSecret[] = "your-client-secret";
   ```

3. Connect to your device's web interface
4. Click "Authorize Teams" and follow the authentication process
5. Select the Teams widget to display your presence status

## Usage

After authorization, the Teams widget will display your current Microsoft Teams presence status, including:
- Available (green)
- Busy (red) 
- In a meeting (blue)
- Away (yellow)
- Do not disturb (red)

The widget updates automatically every 30 seconds when active.
- List recognized boards currently connected (find out which COM port)
    - `arduino-cli board list`
- compile project
  - `arduino-cli compile --fqbn adafruit:samd:adafruit_matrixportal_m4 .`
- upload to board
  - `arduino-cli upload -p COM4 --fqbn adafruit:samd:adafruit_matrixportal_m4 .`

- monitor output
  - `arduino-cli monitor -p COM4 -c baudrate=115200`
- one-liner compile + upload
  - `arduino-cli compile --upload -p COM4 --fqbn adafruit:samd:adafruit_matrixportal_m4 .`