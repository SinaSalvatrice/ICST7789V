#include "pico/stdlib.h"

#define PIN_SCK 2
#define PIN_MOSI 3
#define PIN_CS 4
#define PIN_DC 5
#define PIN_RST 6

static inline void bb_delay(void) { sleep_us(3); }

static void bb_write_byte(uint8_t v) {
    for (int i = 7; i >= 0; --i) {
        gpio_put(PIN_SCK, 0);
        gpio_put(PIN_MOSI, (v >> i) & 1u);
        bb_delay();
        gpio_put(PIN_SCK, 1);
        bb_delay();
    }
    gpio_put(PIN_SCK, 0);
}

static void tft_cmd(uint8_t c) {
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_CS, 0);
    bb_write_byte(c);
    gpio_put(PIN_CS, 1);
}

static void tft_data_byte(uint8_t d) {
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    bb_write_byte(d);
    gpio_put(PIN_CS, 1);
}

static void tft_reset(void) {
    gpio_put(PIN_RST, 1); sleep_ms(20);
    gpio_put(PIN_RST, 0); sleep_ms(50);
    gpio_put(PIN_RST, 1); sleep_ms(200);
}

static void st7789_init(void) {
    tft_reset();
    tft_cmd(0x01); sleep_ms(180);
    tft_cmd(0x11); sleep_ms(180);
    tft_cmd(0x3A); tft_data_byte(0x55);
    tft_cmd(0x36); tft_data_byte(0x00);
    tft_cmd(0x21);
    tft_cmd(0x13);
    tft_cmd(0x29); sleep_ms(120);
}

static void set_window_full(void) {
    tft_cmd(0x2A);
    tft_data_byte(0x00); tft_data_byte(0x00); tft_data_byte(0x00); tft_data_byte(0xEF);
    tft_cmd(0x2B);
    tft_data_byte(0x00); tft_data_byte(0x00); tft_data_byte(0x01); tft_data_byte(0x3F);
    tft_cmd(0x2C);
}

int main(void) {
    gpio_init(PIN_SCK); gpio_set_dir(PIN_SCK, GPIO_OUT); gpio_put(PIN_SCK, 0);
    gpio_init(PIN_MOSI); gpio_set_dir(PIN_MOSI, GPIO_OUT); gpio_put(PIN_MOSI, 0);
    gpio_init(PIN_CS); gpio_set_dir(PIN_CS, GPIO_OUT); gpio_put(PIN_CS, 1);
    gpio_init(PIN_DC); gpio_set_dir(PIN_DC, GPIO_OUT); gpio_put(PIN_DC, 1);
    gpio_init(PIN_RST); gpio_set_dir(PIN_RST, GPIO_OUT); gpio_put(PIN_RST, 1);

    sleep_ms(100);
    st7789_init();
    set_window_full();

    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    for (int i = 0; i < 240 * 320; ++i) {
        bb_write_byte(0xF8);
        bb_write_byte(0x00);
    }
    gpio_put(PIN_CS, 1);

    while (true) tight_loop_contents();
}
