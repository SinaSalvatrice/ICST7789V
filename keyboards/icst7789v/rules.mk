# Waveshare RP2040-Zero ST7789 GPIO diagnostic.
BOARD = GENERIC_RP_RP2040

# Keep the working QMK display logic but use the SPI master directly.
# Quantum Painter stays disabled until native hardware SPI is confirmed.
QUANTUM_PAINTER_ENABLE = no
SPI_DRIVER_REQUIRED = yes

# Matrix and EC11 QMK modules remain enabled in keyboard.json.
# Optional external WS2812/SK6812 on GP29:
RGBLIGHT_ENABLE = no
WS2812_DRIVER = vendor
