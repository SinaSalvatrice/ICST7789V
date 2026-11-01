// 320x240 Layer Map runtime. Included only from keymap.c after SPI routines.
#pragma once
#include "cc_font.h"
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
    cc_dirty = true;
}

static void cc_open_menu(void) {
    cc_display_mode = true;
    cc_menu_view = true;
    cc_help_view = false;
    cc_color_view = false;
    cc_menu_selection = 0;
    cc_dirty = true;
}

static void cc_switch_encoder(bool display_mode) {
    cc_display_mode = display_mode;
    cc_menu_view = false;
    cc_help_view = false;
    cc_color_view = false;
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
        const uint16_t x = 125 + (i % 4u) * 47u;
        const uint16_t y = 40 + (i / 4u) * 55u;
        const uint16_t w = i == 10 ? 90 : 43;
        const char *label = home ? cc_layers[cc_double_target[i]].name :
                                  desc->labels[i];
        const bool highlight = home ? cc_double_target[i] == cc_preview :
                                     cc_last_key == i;
        lcd_fill(x, y, w, 47, highlight ? 0x3A90 : 0x29E8);
        lcd_fill(x, y, w, 2, highlight ? accent : 0x426A);
        cc_draw_text(x + 4, y + 5, "K", 1, 0xAD75);
        if (i < 9) {
            char index_str[2] = {(char)('1' + i), 0};
            cc_draw_text(x + 10, y + 5, index_str, 1, 0xAD75);
        } else {
            cc_draw_text(x + 10, y + 5, i == 9 ? "10" : "11", 1, 0xAD75);
        }
        cc_draw_text(x + 5, y + 25, label, 1, C_WHITE);
    }
}

static void cc_menu_screen(void) {
    static const char *options[5] = {"DISPLAY", "ENCODER", "HOME", "HELP", "COLORS"};
    cc_background();
    lcd_fill(7, 33, 109, 171, 0x2186);
    cc_draw_text(17, 64, "MODULE", 2, C_CYAN);
    cc_draw_text(15, 105, "K0 ONLY", 1, C_WHITE);
    cc_draw_text(16, 164, "NO USB", 1, C_YELLOW);
    for (uint8_t i = 0; i < 5; ++i) {
        const uint16_t y = 36 + i * 34;
        lcd_fill(125, y, 187, 29, cc_menu_selection == i ? 0x2C96 : 0x2145);
        lcd_fill(125, y, 4, 29, cc_menu_selection == i ? C_CYAN : 0x4AAB);
        cc_draw_text(141, y + 10, options[i], 1, C_WHITE);
    }
}

static void cc_help_screen(void) {
    cc_background();
    cc_draw_text(12, 40, "MODULE HELP", 2, C_CYAN);
    cc_draw_text(12, 78, "K0 1X  DISP/ENC", 1, C_WHITE);
    cc_draw_text(12, 96, "K0 2X  HOME", 1, C_WHITE);
    cc_draw_text(12, 114, "K0 HOLD  MENU", 1, C_WHITE);
    cc_draw_text(12, 132, "KEY 1X  MACRO", 1, C_WHITE);
    cc_draw_text(12, 150, "KEY 2X  LAYER", 1, C_WHITE);
    cc_draw_text(12, 168, "KEY HOLD  ALT", 1, C_WHITE);
    cc_draw_text(12, 186, "K11 3X  PREV", 1, C_YELLOW);
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
    if (cc_color_view) cc_color_screen();
    else if (cc_help_view) cc_help_screen();
    else if (cc_menu_view) cc_menu_screen();
    else cc_layer_screen();
    cc_dirty = false;
    cc_last_draw = timer_read();
}

#include "cc_controls.h"
