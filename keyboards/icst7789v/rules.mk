# Waveshare RP2040-Zero ST7789 GPIO diagnostic.
BOARD = GENERIC_RP_RP2040

# Temporarily bypass the QMK Quantum Painter / hardware SPI drivers.
# Their source stays in git history for restoration after this isolation test.
QUANTUM_PAINTER_ENABLE = no

# Matrix and EC11 QMK modules remain enabled in keyboard.json.
# Optional external WS2812/SK6812 on GP29:
RGBLIGHT_ENABLE = no
WS2812_DRIVER = vendor
