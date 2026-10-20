# ICST7789V — ST7789V an RP2040-Zero testen

**Neuer Referenztest (ohne QMK):** Der am ESP32 erfolgreich getestete Adafruit-Farbwechsel wurde für den **Waveshare RP2040-Zero** portiert. Der Arduino-Sketch liegt in [`arduino/ST7789V_RP2040_Zero_Test/ST7789V_RP2040_Zero_Test.ino`](arduino/ST7789V_RP2040_Zero_Test/ST7789V_RP2040_Zero_Test.ino).

Die älteren QMK-/Pico-SDK-Tests bleiben im Repository erhalten. Sie werden durch den Arduino-Test nicht überschrieben.

## Verkabelung (RP2040-Zero ↔ ST7789V 135×240, 8 Pins)

| ST7789V-Pin | RP2040-Zero | Funktion |
|---|---|---|
| GND | GND | Masse |
| VCC | **3V3** | 3,3-V-Versorgung, **nicht 5 V** |
| SCL | **GP2** | SPI0 Clock / SCK |
| SDA | **GP3** | SPI0 MOSI / TX |
| CS | **GP4** | Chip Select |
| DC | **GP5** | Data / Command |
| RES | **GP6** | Hardware Reset |
| BLK | **3V3** | Hintergrundbeleuchtung dauerhaft an |

Auf diesem Display sind **SCL und SDA SPI-Leitungen, nicht I²C**. Falls CS beim vorigen Versuch an GND und RES an 3V3 lagen, **bei ausgeschalteter Versorgung** wieder auf GP4 und GP6 zurückstecken. GP2/GP3 sind die **GPIO-Nummern**, nicht Position 2/3 am Board.

## Arduino IDE — ohne QMK

1. Unter **Datei → Voreinstellungen → Zusätzliche Boardverwalter-URLs** hinzufügen:
   `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
2. Im Boardverwalter das Paket **Raspberry Pi Pico/RP2040** von **Earle F. Philhower** installieren.
3. Als Board **Waveshare RP2040 Zero** auswählen (nicht ESP32).
4. Im Bibliotheksverwalter installieren: **Adafruit ST7735 and ST7789 Library**, **Adafruit GFX Library** und ggf. deren Abhängigkeit **Adafruit BusIO**.
5. Die `.ino`-Datei öffnen, RP2040-Zero per USB verbinden und **Hochladen** wählen. Bei Uploadproblemen: BOOT-Taste beim USB-Anstecken gedrückt halten, dann erneut hochladen (UF2-Bootloader).
6. Optional den seriellen Monitor mit **115200 Baud** öffnen.

**Erwartung:** Der Bildschirm wechselt alle zwei Sekunden zwischen **Rot → Grün → Blau → Weiß → Schwarz**. Der Sketch benutzt **SPI0** auf GP2/GP3, **keine QMK-Firmware**, dieselben Adafruit-Bibliotheken und dieselbe Displayinitialisierung `init(135, 240)` / `setRotation(1)` wie der erfolgreiche ESP32-Test.

## Fehlerdiagnose

- **Farben erscheinen:** Display, RP2040-SPI0 und Displayleitungen sind grundsätzlich okay; den Fehler danach im QMK-Treiber bzw. dessen Initialisierung suchen.
- **Weiterhin schwarz:** Die ESP32-Verdrahtung bzw. -Pins funktionieren nicht automatisch am RP2040. RP2040-GPIO-Belegung und Spannung/Signalfluss am Display prüfen; wenn nötig Hardware-SPI mit langsamem GPIO-Test vergleichen.
- **Bild verschoben:** Erst nach erfolgreichem Farbtest Rotation / Panel-Offsets prüfen.

## Ältere QMK-Testfirmware (unverändert)

Das Projekt enthält unter `keyboards/icst7789v/` den bisherigen QMK-/Quantum-Painter-Test und die bisherigen Windows-/Shell-Buildskripte `build_uf2.bat` und `build_uf2.sh`. **Diese bauen QMK, nicht den neuen Arduino-Sketch.**

QMK-Minimalbuild aus einem vorhandenen QMK-Firmware-Checkout:

```bash
make SKIP_GIT=1 icst7789v:default
```

Die vorhandenen Tests schalten den ST7789V derzeit auf eine rote Testfläche (je nach lokalem Stand sind weitere Pico-SDK-Diagnosen separat vorhanden). Sie sind **nicht** das jetzt empfohlene Referenzverfahren zum Hardware-Bring-up.

**Hinweis:** Dieser Arduino-Sketch prüft nur das Display. Die 9-Tasten-Matrix, VIA, Makros und Tastaturfunktionen des „Nine-Key Macro Master“ werden damit nicht geflasht.
