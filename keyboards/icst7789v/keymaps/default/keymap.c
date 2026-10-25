#include QMK_KEYBOARD_H
#include "qp.h"

// Variable zur Speicherung des Display-Geräte-Handles
painter_device_t display;
#define TFT_PANEL_WIDTH 135
#define TFT_PANEL_HEIGHT 240

#define TFT_CS_PIN GP4
#define TFT_DC_PIN GP5
#define TFT_RST_PIN GP6

#define TFT_SPI_DIVISOR 4
#define TFT_SPI_MODE 0

static painter_device_t display;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(KC_NO)
};

void keyboard_post_init_user(void) {
    // Initialisiert das Display mit den in der config.h definierten Werten
    display = qp_st7789_make_spi_device(LCD_WIDTH, LCD_HEIGHT, ST7789_CS_PIN, ST7789_DC_PIN, ST7789_RESET_PIN, 0, 0);
    
    if (display) {
        qp_init(display, QP_ROTATION_0); // Rotation anpassen falls nötig (QP_ROTATION_90 etc.)
        qp_clear(display);               // Bildschirm leeren
        qp_flush(display);
    }
}
    // Diagnostic mode: no rotation, no manual viewport offsets.
    if (!qp_init(display, QP_ROTATION_0)) {
        return;
    }

    if (!qp_power(display, true)) {
        return;
    }

    // Fill the complete 135x240 canvas solid red.
    // If this appears, SPI + reset + display initialization are working.
    qp_rect(display, 0, 0, TFT_PANEL_WIDTH - 1, TFT_PANEL_HEIGHT - 1, 0, 255, 255, true);
    qp_flush(display);
}
