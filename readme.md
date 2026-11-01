# ICST7789V — Waveshare RP2040-Zero / ST7789 hardware tester

## Hardware-SPI test (current)

The working firmware is saved as
[`backup/qmk-st7789-gpio-working`](https://github.com/SinaSalvatrice/ICST7789V/tree/backup/qmk-st7789-gpio-working).
This version keeps the exact same ST7789 init, four quadrants, encoder/PUSH/K0
and matrix logic, but sends data using QMK's `spi_master.h` over SPI0 GP2/GP3.
No change in wiring. Quantum Painter remains off.

A **magenta stripe** through the middle distinguishes this firmware from the
previous GPIO-only build. On a successful flash you should see the original
red/green/blue/black quadrants with that stripe. If still white, the QMK SPI
path is suspect. No GitHub Actions are used.

Local build:
```powershell
git pull --ff-only origin main
.\build_uf2.bat
```

For instant code-level rollback, in `keyboards/icst7789v/config.h` change
`#define ICST_USE_HW_SPI 1` to `#define ICST_USE_HW_SPI 0` and rebuild.
For a full unchanged rollback, use the saved backup branch.


## Current firmware: QMK direct-GPIO ST7789 isolation test

**Why:** The prior Quantum Painter firmware produced a white screen despite the
same hardware working with an Arduino sketch. The current default QMK keymap
therefore sends ST7789 commands and RGB565 pixels on GP2 (SCK) and GP3 (MOSI)
using direct GPIO output, avoiding QMK hardware SPI and Quantum Painter entirely.

**Wiring remains unchanged:** GND/3V3, SCK GP2, MOSI GP3, RST GP4,
DC GP5, CS GP6, BL GP7, encoder A GP8/B GP9/PUSH GP10, K0 GP11.

**Expected after flashing:** four colored quadrants:
red top left, green top right, blue bottom left, black bottom right.
This deliberate simple firmware is not the final Layer Map UI.

- Turn encoder: lower-right quadrant cycles through colors.
- Press encoder: same quadrant advances, and top-left yellow indicator toggles.
- Press K0: lower-right quadrant resets to black; top-right cyan indicator toggles.
- Later attach 11-key 3×4 matrix: bottom-left square changes between white
  and magenta on each matrix key press.
- Optional external NeoPixels on GP29: set `RGBLIGHT_ENABLE = yes` in
  `rules.mk` after wiring an LED and power; QMK RGB lighting stays independent.

This diagnostic operates deliberately slower than hardware SPI and redraws
only small areas after boot. Matrix and NeoPixel allocations remain reserved.

Build locally (NO GitHub Actions):
```powershell
git pull --ff-only origin main
.\build_uf2.bat
```

If the screen still stays all white, confirm that **this new UF2** was flashed.
A white screen would mean that even basic GPIO toggling is not reaching the
controller, or that QMK initialization is not running; do not guess which
without further evidence.

The earlier Quantum Painter implementation is preserved in git history and
can be reinstated once the GPIO isolation test has succeeded.


The **QMK module diagnostic** in `keyboards/icst7789v/` targets the **new 320×240 landscape ST7789 with EC11 encoder + PUSH + K0**. It is intentionally separate from the older **135×240 8-pin Arduino test** in `arduino/`, which remains untouched as a reference.

## Hardware already connected

Use the GPIO labels printed on the RP2040-Zero board, **not Raspberry Pi Pico 40-pin header numbers**.

| Module signal | RP2040-Zero | Notes |
|---|---|---|
| GND | GND | Common ground |
| VCC | 3V3 | Safe 3.3-V supply |
| SCL / SCK | GP2 | SPI0 clock |
| SDA / MOSI | GP3 | SPI0 TX, **not I2C** |
| RES / RST | GP4 | Display reset |
| DC | GP5 | Data / Command |
| CS | GP6 | SPI chip select |
| BLK | GP7 | Backlight driven HIGH |
| Encoder A | GP8 | EC11 |
| Encoder B | GP9 | EC11 |
| Encoder PUSH | GP10 | Switch to GND |
| K0 | GP11 | Switch to GND |

**The display uses native 240×320 geometry and QP rotation 90° for 320×240 landscape.** The GP4/GP6 CS/RST mapping is for the **new module**, not the old Arduino sketch.

## What the QMK test firmware does

- **Status:** display boot/init, encoder CW/CCW counters, encoder push count, K0 count, and matrix key events. The display only redraws on events (no continual full-screen flashing).
- **Colors:** RED / GREEN / BLUE / WHITE / BLACK. Rotate the encoder to enter the page and press its shaft to advance the color.
- **Geometry:** borders, diagonals, and all four corners to check display rotation/clipping.
- **Matrix:** a 3×4 grid for **eleven keys** and event counter. It is already defined but does not require switches to be wired at boot.
- **NeoPixel:** an optional WS2812/SK6812 chain on GP29. The screen shows 'not enabled' until RGBLIGHT is enabled. Press the encoder shaft on this page to cycle colors after LEDs are connected.
- **K0** always returns to the status page. Rotate the encoder to change pages.

The diagnostics firmware still uses normal QMK key scanning. When you attach switches, they send the default keycodes defined in `keymaps/default/keymap.c` (currently number keys / Enter). The dedicated panel buttons are **not** part of the keyboard matrix.

## GPIO expansion plan

| Function | GPIOs | Status |
|---|---|---|
| TFT SPI + control | GP2–GP7 | Active |
| Encoder A/B + PUSH + K0 | GP8–GP11 | Active |
| Matrix rows (3) | GP12, GP13, GP14 | Reserved/configured |
| Matrix columns (4) | GP15, GP26, GP27, GP28 | Reserved/configured |
| Optional external NeoPixel DIN | GP29 | Reserved; disabled in firmware |
| Spare | GP0, GP1 | Unused |

**Matrix diode direction: COL2ROW.** The diode cathode (stripe) faces the row wire. Matrix key positions are 4 + 4 + 3; the last key is wide physically but electrically still a single switch, with [row 2, column 3] unused. This is a provisional electrical layout and can be adapted to the eventual enclosure.

NeoPixels require a proper shared ground, adequate **external power** for the number of LEDs, and preferably a 3.3-to-5 V logic-level shifter if powered at 5 V. **Do not feed 5 V into RP2040 GPIO.**

## QMK UF2 build

The repository is a **QMK keyboard definition**, not a full QMK checkout. The Windows self-hosted CI and `build_uf2.bat`/`build_uf2.sh` copy `keyboards/icst7789v` into a separate QMK checkout. Windows:

```bat
build_uf2.bat
```

With an already configured QMK checkout, after copying `keyboards/icst7789v` into its `keyboards/` folder:

```sh
qmk compile -kb icst7789v -km default
```

This produces a UF2 file (the Windows script copies to `build/ICST7789V.uf2`). To flash, unplug the RP2040-Zero, hold BOOT while plugging USB in, and copy the UF2 to the RPI-RP2 drive.

### Optional NeoPixels

Only **after wiring the LED data line to GP29**, change `RGBLIGHT_ENABLE = no` to `RGBLIGHT_ENABLE = yes` in `keyboards/icst7789v/rules.mk`. The RP2040 uses the **PIO WS2812 driver** (`WS2812_DRIVER = vendor`). Set the real LED count in `config.h` (`RGBLIGHT_LED_COUNT`, initially 1). Rebuild and flash. Matrix and NeoPixels have non-overlapping pins.

### Troubleshooting

- **Black screen**: verify 3V3/GND, BLK=GP7, SPI0 GP2/GP3, DC=GP5, CS=GP6 and RST=GP4. If the Arduino sketch runs but QMK doesn't, check the QMK SPI/Quantum Painter build logs before moving wires.
- **Screen rotated wrong**: change `QP_ROTATION_90` to `QP_ROTATION_270` in `keymap.c`.
- **Encoder backwards**: swap GP8/GP9 wiring or add `#define ENCODER_DIRECTION_FLIP` in `config.h`.
- **No encoder steps**: try changing `resolution` from 4 to 2 in `keyboard.json`.
- **Build fails**: the original keymap had a duplicated device variable, stray statements outside a function, mismatched panel size, and the wrong SPI driver. Those have been corrected in this branch.

## Legacy independent tests

The old `arduino/ST7789V_RP2040_Zero_Test/ST7789V_RP2040_Zero_Test.ino` still targets a **different 135×240 panel** with different CS/RES wiring. `diag_pico/` is also preserved.
