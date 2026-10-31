# Waveshare RP2040-Zero; no extra Pico header pads required.
BOARD = GENERIC_RP_RP2040

QUANTUM_PAINTER_ENABLE = yes
QUANTUM_PAINTER_DRIVERS += st7789_spi

# Hardware module test: external LED chain is NOT required.
# Switch to yes after wiring a WS2812/SK6812 DIN to GP29.
RGBLIGHT_ENABLE = no
WS2812_DRIVER = vendor
