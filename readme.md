# ICST7789V

Minimal QMK test project for an RP2040-Zero and a 1.14-inch ST7789V SPI TFT (135x240).

The first firmware initializes the display and draws a calibration pattern. No keypad logic yet.

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

The panel is treated as 135x240 and rotated 90 degrees to produce a 240x135 landscape canvas. The initial viewport offset is `(40, 53)`. If the image is shifted or clipped, adjust the two offset constants in `keyboards/icst7789v/keymaps/default/keymap.c`.

The startup image is a calibration pattern: border, center cross and four colored corner blocks. That makes rotation, clipping, viewport offsets and color order obvious immediately.

## QMK source layout

```text
keyboards/icst7789v/
├─ config.h
├─ keyboard.json
├─ mcuconf.h
├─ rules.mk
└─ keymaps/
   └─ default/
      └─ keymap.c
```

The project uses QMK's current `keyboard.json` format.

## Build UF2 locally on Windows

Required on the machine:

- QMK MSYS installed at `C:\QMK_MSYS`
- a QMK Firmware checkout, by default `%USERPROFILE%\qmk_firmware`

Then simply run from this repository:

```bat
build_uf2.bat
```

The build deliberately initializes only the QMK submodules required for this RP2040 target:

- `lib/chibios`
- `lib/chibios-contrib`
- `lib/pico-sdk`
- `lib/lufa`
- `lib/printf`

It does **not** recursively download unrelated platform SDKs such as the large NXP `mcux-sdk`.

A successful build creates:

```text
build/ICST7789V.uf2
```

If QMK Firmware lives somewhere else:

```bat
set QMK_HOME=C:\path\to\qmk_firmware
build_uf2.bat
```

## Self-hosted GitHub Actions build

`.github/workflows/build-uf2.yml` uses only:

```text
[self-hosted, Windows, X64]
```

It uses the persistent QMK installation on the self-hosted Windows runner and calls the same `build_uf2.bat`. It does not check out a second full QMK tree on every run.

If Actions are disabled entirely, the BAT file is the normal local build path.

## Manual QMK build

After copying `keyboards/icst7789v` into `qmk_firmware/keyboards/icst7789v`, the equivalent QMK build is:

```bash
make SKIP_GIT=1 icst7789v:default
```

The `SKIP_GIT=1` flag skips QMK's full recursive submodule scan; `build_uf2.sh` initializes the required RP2040 dependencies explicitly before building.

## Current scope

- RP2040 / RP2040-Zero
- ST7789V via SPI0
- Quantum Painter
- display-only calibration test
- one unused placeholder matrix pin on GP26 so QMK has a valid minimal keyboard definition

The 9-key matrix and macros come later.
