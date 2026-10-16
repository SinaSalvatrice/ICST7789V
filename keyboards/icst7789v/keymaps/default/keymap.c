#include QMK_KEYBOARD_H
#include "qp.h"

// Variable zur Speicherung des Display-Geräte-Handles
painter_device_t display;

void keyboard_post_init_user(void) {
    // Initialisiert das Display mit den in der config.h definierten Werten
    display = qp_st7789_make_spi_device(LCD_WIDTH, LCD_HEIGHT, ST7789_CS_PIN, ST7789_DC_PIN, ST7789_RESET_PIN, 0, 0);
    
    if (display) {
        qp_init(display, QP_ROTATION_0); // Rotation anpassen falls nötig (QP_ROTATION_90 etc.)
        qp_clear(display);               // Bildschirm leeren
        qp_flush(display);
    }
}