#pragma once

#define SPI_DRIVER SPID1
#define SPI_SCK_PIN GP14   // Serieller Takt (SCL/SCK)
#define SPI_MOSI_PIN GP15  // Datenausgang (SDA/MOSI)
#define SPI_MISO_PIN GP12  // Kann ungepinnt bleiben (Display sendet meist nichts)

// Display-Steuerpins (Frei wählbare GPIOs)
#define ST7789_CS_PIN GP13     // Chip Select (CS)
#define ST7789_DC_PIN GP11     // Data/Command (DC/RS)
#define ST7789_RESET_PIN GP10  // Reset (RST)

// Display-Eigenschaften (Beispiel für ein typisches 240x240 Panel)
#define LCD_WIDTH 240
#define LCD_HEIGHT 240

// Falls die Farben invertiert sind (häufig bei ST7789 der Fall), dies aktivieren:
#define LCD_INVERT_COLOR