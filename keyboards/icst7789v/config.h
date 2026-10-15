#pragma once

// Use SPI0 on exposed RP2040-Zero pins.
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP2
#define SPI_MOSI_PIN GP3
#define SPI_MISO_PIN GP0

// Keep the display awake indefinitely for the bench test.
#define QUANTUM_PAINTER_DISPLAY_TIMEOUT 0
#define ST7789_NUM_DEVICES 1
