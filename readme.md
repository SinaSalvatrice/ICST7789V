# ICST7789V

Minimal QMK test project for an RP2040-Zero and a 1.14-inch ST7789V SPI TFT (135x240).

The first firmware does only one useful thing: it initializes the display and draws a calibration pattern. No keypad logic yet.

## Wiring

| Display | RP2040-Zero | Note |
|---|---|---|
| GND | GND | common ground |
| VCC | 3V3 | use 3.3 V |
| SCL | GP2 | SPI0 SCK |
| SDA | GP3 | SPI0 MOSI |
| RES | GP6 | display reset |
| DC | GP5 | data/command |
| CS | GP4 | chip select |
| BLK | 3V3 | backlight always on |

`SCL` and `SDA` are SPI clock/data on this module, despite the I2C-looking labels.

## Display setup

The panel is treated as 135x240 and rotated 90 degrees to produce a 240x135 landscape canvas. The initial viewport offset is set to `(40, 53)`, which is a common value for this ST7789 panel size. If the image is shifted or clipped, adjust the two offset constants in `keyboards/icst7789v/keymaps/default/keymap.c`.

The startup image is intentionally a calibration pattern: border, center cross and four colored corner blocks. That makes rotation, clipping, offsets and color order obvious immediately.

## QMK source layout

The keyboard definition lives in:

```text
keyboards/icst7789v/
├─ config.h
├─ info.json
├─ mcuconf.h
├─ rules.mk
└─ keymaps/
   └─ default/
      └─ keymap.c
```

This repository stores the custom keyboard definition separately from the main QMK repository. To build it, copy `keyboards/icst7789v` into a QMK Firmware checkout under `keyboards/icst7789v`, then run:

```bash
qmk compile -kb icst7789v -km default
```

The output for RP2040 is a `.uf2` file that can be copied to the board while it is in BOOTSEL mode.

## Current scope

- RP2040 / RP2040-Zero
- ST7789V via SPI0
- Quantum Painter
- display-only test
- one unused placeholder matrix pin on GP26 so QMK has a valid minimal keyboard definition

The 9-key matrix and macros come later.
