/*
  ST7789V 135x240 on Waveshare RP2040-Zero
  Independent color test based on the successful ESP32/Adafruit test.
  Board package: "Raspberry Pi Pico/RP2040" by Earle F. Philhower
  Board: "Waveshare RP2040 Zero"
  Libraries: "Adafruit ST7735 and ST7789 Library", "Adafruit GFX Library",
             "Adafruit BusIO" (installed as dependency)

  TFT GND -> RP2040 GND
  TFT VCC -> RP2040 3V3
  TFT SCL -> RP2040 GP2  (SPI0 SCK)
  TFT SDA -> RP2040 GP3  (SPI0 MOSI/TX)
  TFT CS  -> RP2040 GP4
  TFT DC  -> RP2040 GP5
  TFT RES -> RP2040 GP6
  TFT BLK -> RP2040 3V3

  Note: SCL/SDA on this TFT mean SPI, not I2C.
  CS and RES must be connected to GP4 / GP6 (not tied to GND / 3V3).
*/

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

constexpr int8_t TFT_SCLK = 2;
constexpr int8_t TFT_MOSI = 3;
constexpr int8_t TFT_CS   = 4;
constexpr int8_t TFT_DC   = 5;
constexpr int8_t TFT_RST  = 6;

// Explicit SPI0 instance; chip select handled by the Adafruit library.
Adafruit_ST7789 tft(&SPI, TFT_CS, TFT_DC, TFT_RST);

struct ColorStep {
  uint16_t color;
  const char *name;
};

const ColorStep steps[] = {
  { ST77XX_RED,   "ROT" },
  { ST77XX_GREEN, "GRUEN" },
  { ST77XX_BLUE,  "BLAU" },
  { ST77XX_WHITE, "WEISS" },
  { ST77XX_BLACK, "SCHWARZ" }
};

void setup() {
  Serial.begin(115200);
  delay(500);  // No Serial wait: display test runs without a serial monitor.

  Serial.println("RP2040-Zero ST7789V Displaytest startet...");

  // Earle Philhower's Arduino-Pico core supports assigning SPI0 GPIOs.
  // Set these BEFORE SPI.begin() / tft.init().
  SPI.setSCK(TFT_SCLK);
  SPI.setTX(TFT_MOSI);
  // RP2040-Zero defaults MISO to GP4, which this display uses for CS.
  // Move unused MISO to GP0 so SPI.begin() cannot take over the CS pin.
  // Leave GP0 unconnected; the display does not need MISO.
  SPI.setRX(0);
  SPI.begin();

  tft.init(135, 240);
  tft.setRotation(1);  // 240 x 135 landscape, same as the successful ESP32 test.

  Serial.println("ST7789V initialisiert!");
}

void loop() {
  for (const ColorStep &step : steps) {
    Serial.println(step.name);
    tft.fillScreen(step.color);
    delay(2000);
  }
}
