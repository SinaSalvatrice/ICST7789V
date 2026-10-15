#include QMK_KEYBOARD_H
#include "qp.h"

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
    display = qp_st7789_make_spi_device(
        TFT_PANEL_WIDTH,
        TFT_PANEL_HEIGHT,
        TFT_CS_PIN,
        TFT_DC_PIN,
        TFT_RST_PIN,
        TFT_SPI_DIVISOR,
        TFT_SPI_MODE
    );

    if (display == NULL) return;
    if (!qp_init(display, QP_ROTATION_0)) return;
    if (!qp_power(display, true)) return;

    qp_rect(display, 0, 0, TFT_PANEL_WIDTH - 1, TFT_PANEL_HEIGHT - 1, 0, 255, 255, true);
    qp_flush(display);
}
