# ESP32 WIshioT LoRa 915 MHz

This target is for the WIshioT ESP32 LoRa development board listed as Amazon
ASIN B07916Q3T6, using the 915 MHz SX1276 variant. It uses ESP-IDF and keeps
the LoRa radio and OLED board wiring local to `boards/wishio-lora-915`.

The pin map follows the common ESP32 LoRa/OLED V2 layout: OLED I2C uses SDA
GPIO 4 and SCL GPIO 15; the SX1276 uses SPI MOSI 27, MISO 19, SCK 5, CS 18,
RESET 14, and DIO0 26; the onboard LED is GPIO 25. Verify the silkscreen
against the purchased board before connecting external hardware because
WIshioT listings have had board revisions.

Configure and build from a clean directory with ESP-IDF:

```powershell
idf.py -C platform/esp32 -B build\esp32-wishio-lora-915 set-target esp32
idf.py -C platform/esp32 -B build\esp32-wishio-lora-915 build
```

The generated artifact is an ESP-IDF `.bin` image. The current implementation
provides the CTI serial transport and platform I/O contract. The LoRa radio and
OLED are reserved board peripherals; they are not exposed as CTI VISA buses.
The ESP32 ADC calibration and PWM behavior should be verified on hardware
before claiming parity with the Pico targets.
