// 320x240 Layer Map runtime. Included only from keymap.c after SPI routines.
#pragma once
#include "cc_font.h"
#include <stdio.h> // Compact numeric diagnostics for the matrix tester.
#define CC_K0_TAPS_MS 250
#define CC_K0_HOLD_MS 620
#define CC_DEBOUNCE_MS 25

static uint8_t cc_active = 0;
static uint8_t cc_previous = 0;
static uint8_t cc_preview = 0;
static uint8_t cc_last_key = 255;
static uint8_t cc_menu_selection = 0;
static bool cc_display_mode = true;
static bool cc_home_view = true;
static bool cc_menu_view = false;
static bool cc_help_view = false;
static bool cc_color_view = false;
static bool cc_matrix_test_view = false;
// Physical switch status is tracked separately from QMK Tap Dance.
static bool cc_matrix_down[CC_KEY_COUNT] = {false};
static uint16_t cc_matrix_changed = 0;
static uint16_t cc_matrix_hits = 0;
static uint8_t cc_matrix_last_key = 255;
static bool cc_matrix_full_redraw = true;
static bool cc_matrix_return_home = true;
static bool cc_matrix_return_encoder_mode = true;
static uint8_t cc_matrix_return_preview = 0;
static bool cc_dirty = true;
static uint16_t cc_last_draw = 0;
static uint16_t cc_held_code[CC_KEY_COUNT] = {0};

typedef struct {
    bool sample;
    bool stable;
    uint16_t changed;
} cc_button_t;
static cc_button_t cc_k0 = {true, true, 0};
static cc_button_t cc_encoder_button = {true, true, 0};
static uint8_t cc_k0_tap_count = 0;
static uint16_t cc_k0_pressed = 0;
static uint16_t cc_k0_released = 0;
static bool cc_k0_held = false;

static const uint8_t *cc_glyph(char ch) {
    if (ch >= '0' && ch <= '9') return font_numbers[ch - '0'];
    if (ch >= 'A' && ch <= 'Z') return font_letters[ch - 'A'];
    static const uint8_t dash[5] = {0, 0x08, 0x08, 0x08, 0};
    static const uint8_t colon[5] = {0, 0x36, 0x36, 0, 0};
    static const uint8_t slash[5] = {0x20, 0x10, 0x08, 0x04, 0x02};
    static const uint8_t plus[5] = {0x08, 0x08, 0x3E, 0x08, 0x08};
    static const uint8_t dot[5] = {0, 0x40, 0x40, 0, 0};
    if (ch == '-') return dash;
    if (ch == ':') return colon;
    if (ch == '/') return slash;
    if (ch == '+') return plus;
    if (ch == '.') return dot;
    return NULL;
}

static void cc_draw_text(uint16_t x, uint16_t y, const char *str, uint8_t scale, uint16_t color) {
    for (uint8_t n = 0; *str && n < 52 && x + 5u * scale < LCD_W; n++, str++) {
        const uint8_t *glyph = cc_glyph(*str);
        if (glyph) {
            for (uint8_t i = 0; i < 5; i++) {
                for (uint8_t j = 0; j < 7; j++) {
                    if (glyph[i] & (1u << j)) {
                        lcd_fill(x + i * scale, y + j * scale, scale, scale, color);
                    }
                }
            }
        }
        x += 6u * scale;
    }
}

static void cc_draw_icon(uint8_t layer_index, uint16_t accent) {
    const cc_layer_desc_t *desc = &cc_layers[layer_index];
    for (uint8_t y = 0; y < 16; ++y) {
        for (uint8_t x = 0; x < 16; ++x) {
            if (desc->icon[y] & ((uint16_t)0x8000u >> x)) {
                lcd_fill(29 + x * 4, 63 + y * 4, 4, 4, accent);
            }
        }
    }
}

static void cc_activate_layer(uint8_t layer_index) {
    if (layer_index >= CC_LAYER_COUNT) return;
    if (cc_active != layer_index) cc_previous = cc_active;
    cc_active = layer_index;
    cc_preview = layer_index;
    cc_home_view = layer_index == 0;
    cc_menu_view = false;
    cc_help_view = false;
    cc_color_view = false;
    cc_matrix_test_view = false;
    cc_last_key = 255;
    layer_move(layer_index); // Actual exclusive QMK layer.
    cc_dirty = true;
}

static void cc_open_home(void) {
    // K0 exclusively changes the module UI; it never emits a keycode.
    // The active PC macro layer remains unchanged.
    cc_display_mode = true;
    cc_home_view = true;
    cc_preview = 0;
    cc_menu_view = false;
    cc_help_view = false;
    cc_color_view = false;
    cc_matrix_test_view = false;
    cc_dirty = true;
}

static void cc_open_menu(void) {
    cc_display_mode = true;
    cc_menu_view = true;
    cc_help_view = false;
    cc_color_view = false;
    cc_matrix_test_view = false;
    cc_menu_selection = 0;
    cc_dirty = true;
}

static void cc_switch_encoder(bool display_mode) {
    cc_display_mode = display_mode;
    cc_menu_view = false;
    cc_help_view = false;
    cc_color_view = false;
    cc_matrix_test_view = false;
    if (!display_mode) {
        cc_preview = cc_active;
        cc_home_view = cc_active == 0;
    }
    cc_dirty = true;
}

static void cc_background(void) {
    lcd_fill(0, 0, LCD_W, LCD_H, C_BLACK);
    lcd_fill(0, 0, LCD_W, 27, 0x18C7);
    lcd_fill(0, 216, LCD_W, 24, 0x18C7);
    cc_draw_text(7, 9, "CC / LAYERS", 1, C_WHITE);
    cc_draw_text(265, 9, cc_display_mode ? "DISP" : "ENC", 1,
                 cc_display_mode ? C_CYAN : C_YELLOW);
    cc_draw_text(7, 225, "K0 1X MODE 2X HOME HOLD MENU", 1, 0xD69A);
}

static void cc_layer_screen(void) {
    const bool home = cc_home_view;
    const uint8_t selected = home ? 0 : cc_preview;
    const cc_layer_desc_t *desc = &cc_layers[selected];
    const uint16_t accent = desc->accent_rgb565;
    cc_background();
    lcd_fill(7, 33, 109, 171, 0x2186);
    lcd_fill(7, 33, 109, 4, accent);
    cc_draw_icon(selected, accent);
    cc_draw_text(12, 151, desc->name, 2, C_WHITE);
    cc_draw_text(12, 180, home ? "2X: LAYER" : "TAP: ACTION", 1, accent);
    cc_draw_text(12, 192, home ? "HOME MAP" :
                 (selected == cc_active ? "ACTIVE" : "PREVIEW"), 1, C_WHITE);
    for (uint8_t i = 0; i < CC_KEY_COUNT; ++i) {
        // Coordinates come from the physical-key geometry in layer_system.json.
        // K5 is double-height; K9 is double-width; K10 is bottom-right.
        const cc_key_shape_t *shape = &cc_key_shapes[i];
        const uint16_t x = shape->x;
        const uint16_t y = shape->y;
        const uint16_t w = shape->w;
        const uint16_t h = shape->h;
        const char *label = home ? cc_layers[cc_double_target[i]].name :
                                  desc->labels[i];
        const bool highlight = home ? cc_double_target[i] == cc_preview :
                                     cc_last_key == i;
        lcd_fill(x, y, w, h, highlight ? 0x3A90 : 0x29E8);
        lcd_fill(x, y, w, 2, highlight ? accent : 0x426A);
        cc_draw_text(x + 4, y + 5, "K", 1, 0xAD75);
        if (i < 9) {
            char number[2] = {(char)('1' + i), 0};
            cc_draw_text(x + 10, y + 5, number, 1, 0xAD75);
        } else {
            cc_draw_text(x + 10, y + 5, "10", 1, 0xAD75);
        }
        // Shorten long layer names inside 43-pixel single-key tiles.
        char short_label[6] = {0};
        for (uint8_t j = 0; j < 5 && label[j]; ++j) {
            short_label[j] = label[j];
        }
        cc_draw_text(x + 5, y + (h / 2u), short_label, 1, C_WHITE);
    }
}

// Physical matrix test: highlight held switches, count presses and display R/C.
// No host keycodes or Tap Dance effects may escape while this view is active.
static void cc_matrix_test_enter(void) {
    if (cc_matrix_test_view) return;
    cc_matrix_return_home = cc_home_view;
    cc_matrix_return_preview = cc_preview;
    cc_matrix_return_encoder_mode = cc_display_mode;
    cc_matrix_test_view = true;
    cc_menu_view = false;
    cc_help_view = false;
    cc_color_view = false;
    cc_display_mode = true;
    cc_matrix_last_key = 255;
    cc_matrix_hits = 0;
    for (uint8_t i = 0; i < CC_KEY_COUNT; ++i) cc_matrix_down[i] = false;
    cc_matrix_changed = (1u << CC_KEY_COUNT) - 1u;
    cc_matrix_full_redraw = true;
    cc_dirty = true;
}

static void cc_matrix_test_exit(void) {
    if (!cc_matrix_test_view) return;
    cc_matrix_test_view = false;
    cc_home_view = cc_matrix_return_home;
    cc_preview = cc_matrix_return_preview;
    cc_display_mode = cc_matrix_return_encoder_mode;
    cc_matrix_changed = 0;
    cc_matrix_full_redraw = false;
    cc_dirty = true;
}

static void cc_matrix_test_toggle(void) {
    if (cc_matrix_test_view) cc_matrix_test_exit();
    else cc_matrix_test_enter();
}

// The physical row/column mapping is generated from design/layer_system.json.
static void cc_matrix_test_record(uint8_t row, uint8_t col, bool pressed) {
    for (uint8_t i = 0; i < CC_KEY_COUNT; ++i) {
        if (row == cc_matrix_row[i] && col == cc_matrix_col[i]) {
            if (cc_matrix_down[i] != pressed) {
                cc_matrix_down[i] = pressed;
                cc_matrix_changed |= (uint16_t)(1u << i);
                if (pressed) {
                    cc_matrix_last_key = i;
                    if (cc_matrix_hits != UINT16_MAX) ++cc_matrix_hits;
                }
                cc_dirty = true;
            }
            return;
        }
    }
}

static void cc_matrix_key_tile(uint8_t i) {
    const cc_key_shape_t *shape = &cc_key_shapes[i];
    const uint16_t x = shape->x;
    const uint16_t y = shape->y;
    const uint16_t w = shape->w;
    const uint16_t h = shape->h;
    const bool down = cc_matrix_down[i];
    const uint16_t background = down ? 0x05E7 : 0x29E8;
    const uint16_t foreground = down ? C_WHITE : 0xC618;
    lcd_fill(x, y, w, h, background);
    lcd_fill(x, y, w, down ? 5 : 2, down ? C_GREEN : 0x426A);
    char id[4] = {'K', (char)('0' + (i + 1) / 10),
                       (char)('0' + (i + 1) % 10), 0};
    char position[5] = {'R', (char)('0' + cc_matrix_row[i]),
                        'C', (char)('0' + cc_matrix_col[i]), 0};
    cc_draw_text(x + 5, y + 11, id, 1, foreground);
    cc_draw_text(x + 5, y + 27, position, 1, foreground);
    if (h > 70) cc_draw_text(x + 5, y + 65, down ? "DOWN" : "UP", 1, foreground);
}

static void cc_matrix_test_status(void) {
    lcd_fill(8, 115, 107, 89, 0x2186);
    uint8_t active = 0;
    for (uint8_t i = 0; i < CC_KEY_COUNT; ++i) {
        if (cc_matrix_down[i]) ++active;
    }
    char count[6];
    (void)snprintf(count, sizeof(count), "%02u", (unsigned)active);
    cc_draw_text(13, 126, "DOWN", 1, C_CYAN);
    cc_draw_text(71, 126, count, 1, C_WHITE);
    (void)snprintf(count, sizeof(count), "%u", (unsigned)cc_matrix_hits);
    cc_draw_text(13, 143, "HITS", 1, C_CYAN);
    cc_draw_text(13, 157, count, 1, C_WHITE);
    if (cc_matrix_last_key == 255) {
        cc_draw_text(13, 175, "LAST -", 1, C_WHITE);
    } else {
        const uint8_t i = cc_matrix_last_key;
        char last[5] = {'K', (char)('0' + (i + 1) / 10),
                            (char)('0' + (i + 1) % 10), 0};
        char rc[5] = {'R', (char)('0' + cc_matrix_row[i]),
                          'C', (char)('0' + cc_matrix_col[i]), 0};
        cc_draw_text(13, 175, last, 1, C_YELLOW);
        cc_draw_text(13, 190, rc, 1, C_WHITE);
    }
}

static void cc_matrix_test_screen(void) {
    if (cc_matrix_full_redraw) {
        cc_background();
        lcd_fill(7, 33, 109, 171, 0x2186);
        lcd_fill(7, 33, 109, 4, C_CYAN);
        cc_draw_text(13, 55, "MATRIX", 2, C_WHITE);
        cc_draw_text(13, 87, "4-4-2", 1, C_CYAN);
        cc_draw_text(13, 102, "KEY TEST", 1, C_WHITE);
        lcd_fill(0, 216, LCD_W, 24, 0x18C7);
        cc_draw_text(7, 225, "K0 1X EXIT 3X TOGGLE / PUSH EXIT", 1, C_CYAN);
        cc_matrix_changed = (1u << CC_KEY_COUNT) - 1u;
        cc_matrix_full_redraw = false;
    }
    const uint16_t changed = cc_matrix_changed;
    cc_matrix_changed = 0;
    for (uint8_t i = 0; i < CC_KEY_COUNT; ++i) {
        if (changed & (1u << i)) cc_matrix_key_tile(i);
    }
    cc_matrix_test_status();
}

static void cc_menu_screen(void) {
    static const char *options[6] = {"DISPLAY", "ENCODER", "HOME", "MATRIX", "HELP", "COLORS"};
    cc_background();
    lcd_fill(7, 33, 109, 171, 0x2186);
    cc_draw_text(17, 64, "MODULE", 2, C_CYAN);
    cc_draw_text(15, 105, "K0 ONLY", 1, C_WHITE);
    cc_draw_text(16, 164, "NO USB", 1, C_YELLOW);
    for (uint8_t i = 0; i < 6; ++i) {
        const uint16_t y = 34 + i * 29;
        lcd_fill(125, y, 187, 25, cc_menu_selection == i ? 0x2C96 : 0x2145);
        lcd_fill(125, y, 4, 25, cc_menu_selection == i ? C_CYAN : 0x4AAB);
        cc_draw_text(141, y + 9, options[i], 1, C_WHITE);
    }
}

static void cc_help_screen(void) {
    cc_background();
    cc_draw_text(12, 40, "MODULE HELP", 2, C_CYAN);
    cc_draw_text(12, 78, "K0 1X  DISP/ENC", 1, C_WHITE);
    cc_draw_text(12, 96, "K0 2X  HOME", 1, C_WHITE);
    cc_draw_text(12, 114, "K0 3X  MATRIX", 1, C_CYAN);
    cc_draw_text(12, 129, "K0 HOLD  MENU", 1, C_WHITE);
    cc_draw_text(12, 147, "KEY 1X  MACRO", 1, C_WHITE);
    cc_draw_text(12, 162, "KEY 2X  LAYER", 1, C_WHITE);
    cc_draw_text(12, 177, "KEY HOLD  ALT", 1, C_WHITE);
    cc_draw_text(12, 192, "K10 3X  PREV", 1, C_YELLOW);
}

static void cc_color_screen(void) {
    lcd_fill(0, 0, 160, 120, C_RED);
    lcd_fill(160, 0, 160, 120, C_GREEN);
    lcd_fill(0, 120, 160, 120, C_BLUE);
    lcd_fill(160, 120, 160, 120, C_BLACK);
#if ICST_USE_HW_SPI
    lcd_fill(0, 116, 320, 8, C_MAGENTA);
#endif
    cc_draw_text(12, 206, "PUSH TO RETURN", 1, C_WHITE);
}

static void cc_redraw(void) {
    if (!panel_ready) return;
    if (cc_matrix_test_view) cc_matrix_test_screen();
    else if (cc_color_view) cc_color_screen();
    else if (cc_help_view) cc_help_screen();
    else if (cc_menu_view) cc_menu_screen();
    else cc_layer_screen();
    cc_dirty = false;
    cc_last_draw = timer_read();
}

#include "cc_controls.h"
