#include QMK_KEYBOARD_H
#include "qp.h"

// 1.14-inch ST7789 panel geometry.
#define TFT_PANEL_WIDTH 135
#define TFT_PANEL_HEIGHT 240

// Common offsets for a 135x240 ST7789 panel when used in 90-degree rotation.
// If the picture is shifted or clipped, these are the first values to tune.
#define TFT_OFFSET_X 40
#define TFT_OFFSET_Y 53

// RP2040-Zero -> display control pins.
// SPI clock/data are configured globally in config.h as GP2/GP3.
#define TFT_CS_PIN GP4
#define TFT_DC_PIN GP5
#define TFT_RST_PIN GP6

// Conservative first-test SPI settings.
#define TFT_SPI_DIVISOR 4
#define TFT_SPI_MODE 0

static painter_device_t display;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(KC_NO)
};

static void draw_calibration_pattern(void) {
    const uint16_t width = qp_get_width(display);
    const uint16_t height = qp_get_height(display);
    const uint16_t right = width - 1;
    const uint16_t bottom = height - 1;
    const uint16_t cx = width / 2;
    const uint16_t cy = height / 2;

    // Dark background.
    qp_rect(display, 0, 0, right, bottom, 0, 0, 12, true);

    // White outline: if all four sides are visible, the viewport is aligned.
    qp_rect(display, 0, 0, right, bottom, 0, 0, 255, false);

    // Center cross.
    qp_line(display, cx, 0, cx, bottom, 0, 0, 120);
    qp_line(display, 0, cy, right, cy, 0, 0, 120);

    // Four colored corner blocks make orientation and color order obvious.
    qp_rect(display, 6, 6, 29, 29, 0, 255, 255, true);                  // red
    qp_rect(display, right - 29, 6, right - 6, 29, 85, 255, 255, true); // green
    qp_rect(display, 6, bottom - 29, 29, bottom - 6, 170, 255, 255, true); // blue
    qp_rect(display, right - 29, bottom - 29, right - 6, bottom - 6, 43, 255, 255, true); // yellow

    // Bright center marker.
    qp_rect(display, cx - 8, cy - 8, cx + 8, cy + 8, 145, 255, 255, true);

    qp_flush(display);
}

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

    if (display == NULL) {
        return;
    }

    if (!qp_init(display, QP_ROTATION_90)) {
        return;
    }

    qp_set_viewport_offsets(display, TFT_OFFSET_X, TFT_OFFSET_Y);
    qp_power(display, true);
    draw_calibration_pattern();
}
