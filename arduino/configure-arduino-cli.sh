set -e

arduino-cli config init --overwrite

export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json \
https://dl.espressif.com/dl/package_esp32_index.json \
http://arduino.esp8266.com/stable/package_esp8266com_index.json \
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json \
https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json \
http://dl.sipeed.com/MAIX/Maixduino/package_Maixduino_k210_index.json"

arduino-cli core update-index

arduino-cli core search .

arduino-cli core install arduino:avr
arduino-cli core install arduino:mbed_edge
arduino-cli core install arduino:mbed_rp2040
arduino-cli core install arduino:sam
arduino-cli core install arduino:samd
arduino-cli core install esp8266:esp8266
arduino-cli core install arduino:megaavr
arduino-cli core install arduino:nrf52

arduino-cli core install esp32:esp32
arduino-cli core install SparkFun:avr
arduino-cli core install SparkFun:esp32
arduino-cli core install STMicroelectronics:stm32

arduino-cli core list