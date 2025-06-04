use arduino-cli to compile and upload

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