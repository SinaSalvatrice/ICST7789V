#include "pico/stdlib.h"
#include "hardware/spi.h"

#define TFT_SPI spi0
#define PIN_SCK 2
#define PIN_MOSI 3
#define PIN_CS 4
#define PIN_DC 5
#define PIN_RST 6

static inline void tft_select(void) { gpio_put(PIN_CS, 0); }
static inline void tft_deselect(void) { gpio_put(PIN_CS, 1); }

static void tft_cmd(uint8_t cmd) {
    gpio_put(PIN_DC, 0);
    tft_select();
    spi_write_blocking(TFT_SPI, &cmd, 1);
    tft_deselect();
}

static void tft_data(const uint8_t *data, size_t len) {
    gpio_put(PIN_DC, 1);
    tft_select();
    spi_write_blocking(TFT_SPI, data, len);
    tft_deselect();
}

static void tft_cmd_data(uint8_t cmd, const uint8_t *data, size_t len) {
    tft_cmd(cmd);
    if (len) tft_data(data, len);
}

static void tft_reset(void) {
    gpio_put(PIN_RST, 1);
    sleep_ms(10);
    gpio_put(PIN_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_RST, 1);
    sleep_ms(150);
}

static void st7789_init(void) {
    tft_reset();

    tft_cmd(0x01); // SWRESET
    sleep_ms(150);
    tft_cmd(0x11); // SLPOUT
    sleep_ms(120);

    const uint8_t colmod = 0x55; // RGB565
    tft_cmd_data(0x3A, &colmod, 1);

    const uint8_t madctl = 0x00;
    tft_cmd_data(0x36, &madctl, 1);

    tft_cmd(0x21); // INVON
    tft_cmd(0x13); // NORON
    tft_cmd(0x29); // DISPON
    sleep_ms(100);
}

static void fill_entire_controller_red(void) {
    // Fill the entire 240x320 ST7789 GRAM. This deliberately avoids any
    // 135x240 panel-offset assumptions: wherever the visible panel sits in
    // controller memory, it should become red if SPI/control wiring works.
    const uint8_t cols[] = {0x00, 0x00, 0x00, 0xEF}; // 0..239
    const uint8_t rows[] = {0x00, 0x00, 0x01, 0x3F}; // 0..319
    tft_cmd_data(0x2A, cols, sizeof(cols));
    tft_cmd_data(0x2B, rows, sizeof(rows));
    tft_cmd(0x2C); // RAMWR

    uint8_t pixels[512]; // 256 red RGB565 pixels
    for (size_t i = 0; i < sizeof(pixels); i += 2) {
        pixels[i] = 0xF8;
        pixels[i + 1] = 0x00;
    }

    gpio_put(PIN_DC, 1);
    tft_select();
    for (int chunk = 0; chunk < 300; ++chunk) {
        spi_write_blocking(TFT_SPI, pixels, sizeof(pixels));
    }
    tft_deselect();
}

int main(void) {
    spi_init(TFT_SPI, 4000 * 1000);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_init(PIN_DC);
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_RST, 1);

    sleep_ms(50);
    st7789_init();
    fill_entire_controller_red();

    while (true) {
        tight_loop_contents();
    }
}
