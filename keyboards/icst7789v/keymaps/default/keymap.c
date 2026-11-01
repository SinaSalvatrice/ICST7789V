// ICST7789V - QMK independent GPIO diagnostic for ST7789 (240x320 native).
// Deliberately bypasses Quantum Painter and hardware SPI. No rewiring required.
// SPI mode 0, write-only on GP2 SCK / GP3 MOSI, 3.3V signals.
#include QMK_KEYBOARD_H
#include "gpio.h"
#include "timer.h"
#include <stdbool.h>
#include <stdint.h>

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

#define LCD_SCK   GP2
#define LCD_MOSI  GP3
#define LCD_RST   GP4
#define LCD_DC    GP5
#define LCD_CS    GP6
#define LCD_BL    GP7
#define ENCODER_SW GP10
#define K0_SW      GP11

#define LCD_W 320
#define LCD_H 240

// RGB565 color values.
#define C_BLACK   0x0000u
#define C_WHITE   0xFFFFu
#define C_RED     0xF800u
#define C_GREEN   0x07E0u
#define C_BLUE    0x001Fu
#define C_YELLOW  0xFFE0u
#define C_CYAN    0x07FFu
#define C_MAGENTA 0xF81Fu

static bool panel_ready = false;
static uint8_t selected_color = 0;
static uint8_t current_key = 0;
static bool dirty_color = false;
static bool dirty_buttons = false;
static bool dirty_matrix = false;
static bool push_indicator = false;
static bool k0_indicator = false;

typedef struct {
    bool sample;
    bool stable;
    uint16_t changed;
} debounced_button_t;
static debounced_button_t encoder_button = {true, true, 0};
static debounced_button_t k0_button = {true, true, 0};

static const uint16_t palette[] = {
    C_BLACK, C_WHITE, C_YELLOW, C_CYAN, C_MAGENTA, C_RED, C_GREEN, C_BLUE
};
#define PALETTE_SIZE (sizeof(palette) / sizeof(palette[0]))

// Direct software SPI, MSB first, CPOL=0, CPHA=0.
// Each clock pulse drives a bit into the ST7789. No SPI peripheral is used.
static inline void lcd_write_byte(uint8_t value) {
    for (uint8_t bit = 0; bit < 8; ++bit) {
        gpio_write_pin_low(LCD_SCK);
        gpio_write_pin(LCD_MOSI, (value & 0x80u) != 0);
        gpio_write_pin_high(LCD_SCK);
        value <<= 1;
    }
    gpio_write_pin_low(LCD_SCK);
}

static void lcd_command(uint8_t command) {
    gpio_write_pin_low(LCD_CS);
    gpio_write_pin_low(LCD_DC);
    lcd_write_byte(command);
    gpio_write_pin_high(LCD_CS);
}

static void lcd_command_data(uint8_t command, const uint8_t *data, uint8_t length) {
    gpio_write_pin_low(LCD_CS);
    gpio_write_pin_low(LCD_DC);
    lcd_write_byte(command);
    if (length) {
        gpio_write_pin_high(LCD_DC);
        for (uint8_t i = 0; i < length; ++i) {
            lcd_write_byte(data[i]);
        }
    }
    gpio_write_pin_high(LCD_CS);
}

static void lcd_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    const uint8_t cols[] = {x0 >> 8, x0 & 0xFFu, x1 >> 8, x1 & 0xFFu};
    const uint8_t rows[] = {y0 >> 8, y0 & 0xFFu, y1 >> 8, y1 & 0xFFu};
    lcd_command_data(0x2Au, cols, 4);
    lcd_command_data(0x2Bu, rows, 4);
}

static void lcd_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565) {
    if (!panel_ready || !w || !h || x >= LCD_W || y >= LCD_H) {
        return;
    }
    if (w > LCD_W - x) w = LCD_W - x;
    if (h > LCD_H - y) h = LCD_H - y;

    lcd_window(x, y, x + w - 1, y + h - 1);
    gpio_write_pin_low(LCD_CS);
    gpio_write_pin_low(LCD_DC);
    lcd_write_byte(0x2Cu);  // RAMWR
    gpio_write_pin_high(LCD_DC);
    const uint8_t hi = rgb565 >> 8;
    const uint8_t lo = rgb565 & 0xFFu;
    const uint32_t pixels = (uint32_t)w * (uint32_t)h;
    for (uint32_t n = 0; n < pixels; ++n) {
        lcd_write_byte(hi);
        lcd_write_byte(lo);
    }
    gpio_write_pin_high(LCD_CS);
}

// Hardware reset and standard ST7789 16-bit initialization.
// MADCTL=0x60 corresponds to landscape orientation (320x240).
static void lcd_init(void) {
    gpio_set_pin_output(LCD_BL);
    gpio_write_pin_low(LCD_BL); // QMK heartbeat: visible 1-second backlight OFF phase.
    wait_ms(1000);
    gpio_set_pin_output(LCD_CS);
    gpio_set_pin_output(LCD_DC);
    gpio_set_pin_output(LCD_RST);
    gpio_set_pin_output(LCD_SCK);
    gpio_set_pin_output(LCD_MOSI);
    gpio_write_pin_high(LCD_CS);
    gpio_write_pin_high(LCD_DC);
    gpio_write_pin_low(LCD_SCK);
    gpio_write_pin_low(LCD_MOSI);

    gpio_write_pin_high(LCD_RST);
    wait_ms(20);
    gpio_write_pin_low(LCD_RST);
    wait_ms(40);
    gpio_write_pin_high(LCD_RST);
    wait_ms(160);

    lcd_command(0x01u); // SWRESET
    wait_ms(160);
    lcd_command(0x11u); // SLPOUT
    wait_ms(160);
    const uint8_t pixel_format = 0x55u; // RGB565
    lcd_command_data(0x3Au, &pixel_format, 1);
    const uint8_t orientation = 0x60u; // MY=0 MX=1 MV=1, RGB
    lcd_command_data(0x36u, &orientation, 1);
    lcd_command(0x21u); // INVON, standard ST7789 setup
    lcd_command(0x13u); // NORON
    wait_ms(15);
    lcd_command(0x29u); // DISPON
    wait_ms(100);
    panel_ready = true;
    gpio_write_pin_high(LCD_BL);
}

static void lcd_render_reference(void) {
    // Four distinctive colored quadrants. Red begins immediately after init.
    lcd_fill(0, 0, 160, 120, C_RED);
    lcd_fill(160, 0, 160, 120, C_GREEN);
    lcd_fill(0, 120, 160, 120, C_BLUE);
    lcd_fill(160, 120, 160, 120, C_BLACK);

    // Indicators for encoder press (yellow), K0 (cyan), and future matrix (magenta).
    lcd_fill(130, 10, 20, 20, C_BLACK);
    lcd_fill(290, 10, 20, 20, C_BLACK);
    lcd_fill(130, 210, 20, 20, C_WHITE);
}

static void lcd_update_status(void) {
    if (!panel_ready) return;
    if (dirty_color) {
        lcd_fill(160, 120, 160, 120, palette[selected_color]);
        dirty_color = false;
    }
    if (dirty_buttons) {
        lcd_fill(130, 10, 20, 20, push_indicator ? C_YELLOW : C_BLACK);
        lcd_fill(290, 10, 20, 20, k0_indicator ? C_CYAN : C_BLACK);
        dirty_buttons = false;
    }
    if (dirty_matrix) {
        lcd_fill(130, 210, 20, 20, (current_key & 1u) ? C_MAGENTA : C_WHITE);
        dirty_matrix = false;
    }
}

static bool poll_button(pin_t pin, debounced_button_t *state) {
    const bool sampled = gpio_read_pin(pin);
    if (sampled != state->sample) {
        state->sample = sampled;
        state->changed = timer_read();
    }
    if (sampled != state->stable && timer_elapsed(state->changed) >= 25) {
        state->stable = sampled;
        if (!sampled) return true; // Press event.
    }
    return false;
}

void keyboard_post_init_user(void) {
    gpio_set_pin_input_high(ENCODER_SW);
    gpio_set_pin_input_high(K0_SW);
    lcd_init();
    lcd_render_reference();
}

void matrix_scan_user(void) {
    if (poll_button(ENCODER_SW, &encoder_button)) {
        push_indicator = !push_indicator;
        selected_color = (selected_color + 1) % PALETTE_SIZE;
        dirty_buttons = true;
        dirty_color = true;
#ifdef RGBLIGHT_ENABLE
        rgblight_sethsv_noeeprom((uint8_t)(selected_color * 32), 255, 80);
#endif
    }
    if (poll_button(K0_SW, &k0_button)) {
        k0_indicator = !k0_indicator;
        selected_color = 0;
        dirty_buttons = true;
        dirty_color = true;
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        selected_color = (selected_color + (clockwise ? 1 : PALETTE_SIZE - 1)) % PALETTE_SIZE;
        dirty_color = true;
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed && record->event.key.row < 3 && record->event.key.col < 4) {
        ++current_key;
        dirty_matrix = true;
    }
    return true;
}

void housekeeping_task_user(void) {
    if (dirty_color || dirty_buttons || dirty_matrix) {
        lcd_update_status();
    }
}
