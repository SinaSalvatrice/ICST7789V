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

## Build UF2 locally on Windows

The repository contains `build_uf2.bat`.

Requirements:

- QMK CLI available in `PATH`
- QMK toolchain installed
- a `qmk_firmware` checkout

The script looks for `qmk_firmware` in these locations, in order:

1. `%QMK_HOME%`
2. `./qmk_firmware`
3. `../qmk_firmware`
4. `%USERPROFILE%/qmk_firmware`
5. `C:/qmk_firmware`

If your checkout lives somewhere else, set it before building, for example:

```bat
set QMK_HOME=C:\GitHub\qmk_firmware
build_uf2.bat
```

A successful build creates:

```text
build/ICST7789V.uf2
```

## Self-hosted GitHub Actions build

`.github/workflows/build-uf2.yml` uses only a Windows self-hosted runner:

```text
[self-hosted, Windows, X64]
```

The runner must already have QMK CLI and the ARM GCC toolchain installed and available in `PATH`. The workflow checks out QMK Firmware locally on that runner, calls `build_uf2.bat`, and uploads the resulting UF2 as the `ICST7789V-UF2` artifact.

No GitHub-hosted runner is used.

If Actions are completely disabled for the repository/account, use `build_uf2.bat` directly instead.

## Manual QMK build

You can also copy `keyboards/icst7789v` into a QMK Firmware checkout under `keyboards/icst7789v`, then run:

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
