// ICST7789V - ST7789 / EC11 / K0 hardware diagnostics.
// No external matrix keys or NeoPixels are required to boot.
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_st7789.h"
#include "timer.h"
#include <stdio.h>

#ifdef RGBLIGHT_ENABLE
#    include "rgblight.h"
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_1, KC_2, KC_3, KC_4,
        KC_5, KC_6, KC_7, KC_8,
        KC_9, KC_0, KC_ENT
    )
};

#define SCREEN_W 320
#define SCREEN_H 240
#define PAGE_COUNT 5
#define COLOR_COUNT 5
#define BUTTON_DEBOUNCE_MS 25

static painter_device_t display = NULL;
static bool display_ready = false;
static bool display_dirty = true;
static uint16_t last_draw = 0;

static uint8_t page = 0;
static uint8_t color_index = 0;
static uint16_t cw_count = 0;
static uint16_t ccw_count = 0;
static uint16_t key_count = 0;
static uint8_t last_key = 0;
#ifdef RGBLIGHT_ENABLE
static uint8_t neo_hue = 0;
#endif

typedef struct {
    bool sample;
    bool stable;
    uint16_t changed;
    uint16_t presses;
} button_t;

static button_t encoder_push = {true, true, 0, 0};
static button_t k0_button = {true, true, 0, 0};

// Compact built-in 5x7 diagnostics font: no font conversion or assets needed.
// Each byte represents a vertical column, bit 0 is the top pixel.
static const uint8_t font_numbers[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
};
static const uint8_t font_letters[26][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

static void paint_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t h, uint8_t s, uint8_t v) {
    qp_rect(display, x0, y0, x1, y1, h, s, v, true);
}

static const uint8_t *glyph(char c) {
    if (c >= '0' && c <= '9') return font_numbers[c - '0'];
    if (c >= 'A' && c <= 'Z') return font_letters[c - 'A'];
    static const uint8_t dash[5] = {0, 0x08, 0x08, 0x08, 0};
    static const uint8_t colon[5] = {0, 0x36, 0x36, 0, 0};
    static const uint8_t slash[5] = {0x20, 0x10, 0x08, 0x04, 0x02};
    static const uint8_t dot[5] = {0, 0x40, 0x40, 0, 0};
    switch (c) {
        case '-': return dash;
        case ':': return colon;
        case '/': return slash;
        case '.': return dot;
        default: return NULL;
    }
}

static void paint_text(uint16_t x, uint16_t y, const char *str, uint8_t scale, uint8_t hue, uint8_t sat, uint8_t val) {
    for (; *str && x + 5 * scale < SCREEN_W; ++str, x += 6 * scale) {
        const uint8_t *bitmap = glyph(*str);
        if (!bitmap) continue;
        for (uint8_t col = 0; col < 5; ++col) {
            for (uint8_t row = 0; row < 7; ++row) {
                if (bitmap[col] & (1u << row)) {
                    uint16_t px = x + col * scale;
                    uint16_t py = y + row * scale;
                    paint_rect(px, py, px + scale - 1, py + scale - 1, hue, sat, val);
                }
            }
        }
    }
}

static void draw_header(const char *title) {
    paint_rect(0, 0, SCREEN_W - 1, 34, 145, 255, 105);
    paint_text(10, 9, title, 2, 0, 0, 255);
}

static void draw_status(void) {
    char buf[30];
    draw_header("ICST7789V TEST");
    paint_text(12, 48, "ENCODER", 2, 125, 255, 255);
    snprintf(buf, sizeof(buf), "CW:%u CCW:%u", (unsigned)cw_count, (unsigned)ccw_count);
    paint_text(12, 76, buf, 2, 0, 0, 255);
    snprintf(buf, sizeof(buf), "PUSH:%u K0:%u", (unsigned)encoder_push.presses, (unsigned)k0_button.presses);
    paint_text(12, 110, buf, 2, 0, 0, 255);
    snprintf(buf, sizeof(buf), "KEYS:%u LAST:%u", (unsigned)key_count, (unsigned)last_key);
    paint_text(12, 144, buf, 2, 0, 0, 255);
    paint_text(12, 202, "TURN TO CHANGE PAGE", 1, 125, 255, 235);
    paint_text(12, 216, "K0 BACK TO STATUS", 1, 125, 255, 235);
}

static void draw_colors(void) {
    static const char *names[COLOR_COUNT] = {"RED", "GREEN", "BLUE", "WHITE", "BLACK"};
    static const uint8_t hues[COLOR_COUNT] = {0, 85, 170, 0, 0};
    static const uint8_t sats[COLOR_COUNT] = {255, 255, 255, 0, 0};
    static const uint8_t vals[COLOR_COUNT] = {255, 255, 255, 255, 0};
    paint_rect(0, 0, 319, 239, hues[color_index], sats[color_index], vals[color_index]);
    uint8_t text_val = color_index == 3 ? 0 : 255;
    paint_text(14, 20, "COLOR TEST", 3, 0, 0, text_val);
    paint_text(14, 72, names[color_index], 3, 0, 0, text_val);
    paint_text(14, 198, "PUSH NEXT COLOR", 2, 0, 0, text_val);
}

static void draw_geometry(void) {
    draw_header("GEOMETRY");
    qp_rect(display, 1, 35, 318, 238, 0, 0, 255, false);
    qp_line(display, 1, 35, 318, 238, 0, 255, 255);
    qp_line(display, 318, 35, 1, 238, 85, 255, 255);
    paint_rect(8, 42, 22, 56, 0, 255, 255);
    paint_rect(297, 42, 311, 56, 85, 255, 255);
    paint_rect(8, 216, 22, 230, 170, 255, 255);
    paint_rect(297, 216, 311, 230, 0, 0, 255);
    paint_text(75, 109, "320 X 240", 2, 0, 0, 255);
}

static void draw_matrix(void) {
    char buf[25];
    draw_header("MATRIX 3X4");
    for (uint8_t row = 0; row < 3; ++row) {
        matrix_row_t pressed = matrix_get_row(row);
        for (uint8_t col = 0; col < 4; ++col) {
            if (row == 2 && col == 3) continue;
            uint8_t key = row * 4 + col + 1;
            uint16_t x = 10 + col * 77;
            uint16_t y = 45 + row * 49;
            bool down = (pressed & ((matrix_row_t)1 << col)) != 0;
            paint_rect(x, y, x + 68, y + 39, down ? 85 : 150, down ? 255 : 115, down ? 255 : 95);
            snprintf(buf, sizeof(buf), "K%u", (unsigned)key);
            paint_text(x + 10, y + 13, buf, 2, 0, 0, 255);
        }
    }
    snprintf(buf, sizeof(buf), "EVENTS:%u", (unsigned)key_count);
    paint_text(10, 202, buf, 2, 0, 0, 255);
}

static void draw_neopixel(void) {
    draw_header("NEOPIXEL");
#ifdef RGBLIGHT_ENABLE
    char buf[25];
    paint_text(12, 58, "ENABLED GP29", 2, 85, 255, 255);
    snprintf(buf, sizeof(buf), "HUE:%u", (unsigned)neo_hue);
    paint_text(12, 100, buf, 2, 0, 0, 255);
    paint_text(12, 184, "PUSH NEXT HUE", 2, 0, 0, 255);
#else
    paint_text(12, 68, "NOT ENABLED", 2, 0, 255, 255);
    paint_text(12, 114, "GP29 RESERVED", 2, 125, 255, 255);
    paint_text(12, 180, "RGBLIGHT YES IN RULES", 1, 0, 0, 220);
#endif
}

static void draw_page(void) {
    if (!display_ready) return;
    // ST7789 Quantum Painter's qp_clear() reinitializes the panel; it does NOT
    // clear the visible framebuffer. Explicitly paint black instead.
    paint_rect(0, 0, SCREEN_W - 1, SCREEN_H - 1, 0, 0, 0);
    switch (page) {
        case 0: draw_status(); break;
        case 1: draw_colors(); break;
        case 2: draw_geometry(); break;
        case 3: draw_matrix(); break;
        case 4: draw_neopixel(); break;
    }
    qp_flush(display);
    display_dirty = false;
    last_draw = timer_read();
}

static void process_button(pin_t pin, button_t *button, bool is_encoder_push) {
    bool raw = gpio_read_pin(pin); // HIGH idle, LOW pressed.
    if (raw != button->sample) {
        button->sample = raw;
        button->changed = timer_read();
    }
    if (raw != button->stable && timer_elapsed(button->changed) >= BUTTON_DEBOUNCE_MS) {
        button->stable = raw;
        if (!raw) {
            ++button->presses;
            if (is_encoder_push) {
                if (page == 1) color_index = (color_index + 1) % COLOR_COUNT;
#ifdef RGBLIGHT_ENABLE
                if (page == 4) {
                    neo_hue += 32;
                    rgblight_sethsv_noeeprom(neo_hue, 255, 80);
                }
#endif
            } else {
                page = 0; // K0 always returns home.
            }
            display_dirty = true;
        }
    }
}

void keyboard_post_init_user(void) {
    gpio_set_pin_output(TFT_BL_PIN);
    gpio_write_pin_high(TFT_BL_PIN);
    gpio_set_pin_input_high(ENCODER_PUSH_PIN);
    gpio_set_pin_input_high(TFT_K0_PIN);

    // Give the panel time to stabilize after USB power-up.
    wait_ms(150);

    // Lower SPI clock to ~3.9 MHz for reliable wiring during bring-up.
    display = qp_st7789_make_spi_device(TFT_WIDTH, TFT_HEIGHT, TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN, 32, 0);
    if (display && qp_init(display, QP_ROTATION_90)) {
        if (qp_power(display, true)) {
            display_ready = true;

            // Visual SPI probe: a red screen for one second proves that QMK
            // initialization and SPI pixel writes reach the ST7789.
            paint_rect(0, 0, SCREEN_W - 1, SCREEN_H - 1, 0, 255, 255);
            wait_ms(1000);
            draw_page();
        }
    }
}

void matrix_scan_user(void) {
    process_button(ENCODER_PUSH_PIN, &encoder_push, true);
    process_button(TFT_K0_PIN, &k0_button, false);
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index != 0) return false;
    if (clockwise) {
        ++cw_count;
        page = (page + 1) % PAGE_COUNT;
    } else {
        ++ccw_count;
        page = (page + PAGE_COUNT - 1) % PAGE_COUNT;
    }
    display_dirty = true;
    return false; // Diagnostics only; no host keycodes.
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uint8_t row = record->event.key.row;
        uint8_t col = record->event.key.col;
        if (row < 3 && col < 4) {
            ++key_count;
            last_key = row * 4 + col + 1;
            display_dirty = true;
        }
    }
    return true;
}

void housekeeping_task_user(void) {
    // QP paints only on events; coalesce very fast encoder pulses.
    if (display_dirty && timer_elapsed(last_draw) >= 30) {
        draw_page();
    }
}
