#pragma once

// RP2040-Zero, ST7789 240x320 native / 320x240 landscape.
// These pins match the working Arduino Layer Map wiring.
// SPI0: GP2 SCK, GP3 MOSI. This is a write-only panel.
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP2
#define SPI_MOSI_PIN GP3
#define SPI_MISO_PIN NO_PIN

// 1 = hardware SPI0, 0 = proven software-GPIO implementation.
#define ICST_USE_HW_SPI 1
// Conservative initial SPI clock divider (125 MHz / 32 ~ 3.9 MHz).
#define ICST_SPI_DIVISOR 32

#define TFT_CS_PIN GP6
#define TFT_DC_PIN GP5
#define TFT_RST_PIN GP4
#define TFT_BL_PIN GP7

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

// Both button contacts switch to GND.
#define ENCODER_PUSH_PIN GP10
#define TFT_K0_PIN GP11

// Keep the QP display awake while diagnosing the hardware.
#define QUANTUM_PAINTER_DISPLAY_TIMEOUT 0

// Optional, external WS2812/SK6812 chain; not the RP2040-Zero's on-board LED.
// Only enabled when RGBLIGHT_ENABLE=yes in rules.mk.
#define WS2812_DI_PIN GP29
#define RGBLIGHT_LED_COUNT 1
#define RGBLIGHT_LIMIT_VAL 96
