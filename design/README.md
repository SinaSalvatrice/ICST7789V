# CircuitCurios Macro Deck – Layer UI (Entwurf)

**Quelle für die Bearbeitung:** `design/layer_system.json`.
Diese Datei ist der Ausgangspunkt für die Displaybeschriftungen, 16×16-Piktogramme,
Farben, QMK-Layerbelegung, Tap-/Hold-Keymaps und die Encoder-Makros.
Es sind ausdrücklich **editierbare Beispielbelegungen**, keine behaupteten
fertigen Produktiv-Keymaps. Der endgültige globale Source-of-Truth-Abgleich
erfolgt erst, wenn die Inhalte bestätigt sind.

## Bedienung (Hardware: ST7789 320×240, Bedienelemente rechts)

- Links große Layeridentität: 16×16-Icon in 4-facher Darstellung, Akzent,
  Titel und Aktiv-/Vorschauzustand.
- Rechts feste Matrix aus 4 + 4 + 3 Displayfeldern. K11 ist doppelt breit.
- Home zeigt die **globale** Doppel-Tipp-Zuordnung für K1–K11.
- In den Layern zeigen diese Felder die **einzelnen** Aktionen der Tasten.
- Jedes Makrofeld: **1×** = Aktion des aktuellen Layers,
  **2×** = aus jedem Layer direkt zum global fest zugeordneten Layer,
  **Halten** = die optionale Hold-Aktion des aktuellen Layers.
- Nur **K11 3×** = zum vorherigen Makrolayer (kein Triple auf allen Tasten).
- QMK-Layer werden exklusiv über `layer_move()` gewechselt.

## K0: unveränderlicher Modul-Super-Key

K0 (GP11) ist **niemals Teil der 11er-Keymatrix** und sendet keine Host-Codes.

| K0 | Aktion |
|---|---|
| 1× | Encoder zwischen Display- und Makromodus umschalten |
| 2× | Home-Display aufrufen, ohne den aktiven PC-Makrolayer zu verändern |
| Halten (620ms) | Modulmenü aufrufen und in den Displaymodus wechseln |

K0-Einzel-/Doppeltipp werden getrennt mit 250ms ausgewertet.
Das ist unabhängig vom QMK-Tap-Dance-Fenster der elf Tasten (200ms).

## Gemeinsamer Drehencoder

- **DISP:** Drehen = Layer durchsuchen, Encoder drücken =
  sichtbaren Layer aktivieren. Im Modulmenü = Menüpunkt wählen/aktivieren.
  Ein Layer wird durch Navigation allein **nicht** aktiviert.
- **ENC:** Drehen = Encoder-Funktionen des aktiven Makrolayers,
  Drücken = dessen Encoder-Press-Makro.
- Das Display zeigt die aktive Encoder-Betriebsart (DISP/ENC) dauerhaft.
  K0 1× ist die jederzeit verfügbare Umschaltung; es wird kein
  zweiter Encoder vorausgesetzt.

## Modulmenü

K0 halten öffnet: DISPLAY, ENCODER, HOME, HELP, COLORS.
Das Diagnose-Farbbild bleibt als separate Menüseite verfügbar.

## Sicherer lokaler Build

Alle Änderungen liegen auf `feature/layermap-tapdance-k0-module`.
`main` enthält weiterhin die funktionierende Hardware-SPI-Diagnose.

```powershell
git fetch origin
git switch feature/layermap-tapdance-k0-module
git pull --ff-only
.\build_uf2.bat
```

Das Windows-Buildskript ruft `scripts/generate_layer_system.py` auf.
Es generiert `generated_layers.h` und die SVG-Designvorschauen.
Kein GitHub Actions, keine Cloud-Builds, keine Tests durch GitHub.

Ablauf zur Änderung:
1. `design/layer_system.json` bearbeiten (Layernamen, Farben, Icon-Pixel,
   `keys[].tap`, `keys[].hold`, `encoder.*`).
2. `python scripts/generate_layer_system.py` für eine direkte Vorschau
   oder `.\build_uf2.bat` für Generierung und lokalen UF2-Build.
3. **Erst nach Designabnahme** Daten in die übergeordnete Source of Truth
   übernehmen – keine separaten widersprüchlichen Keymaps pflegen.

## Hardwarestand

Wiederverwendet: GP2/3 SPI0, GP4 Reset, GP5 DC, GP6 CS, GP7 BL,
GP8/9 Encoder A/B, GP10 Encoder-PUSH, GP11 dedizierter K0.
Matrix-Reservierung wie bisher 3×4, RGB/NeoPixel bleibt optional.
Die GPIO-Startinitialisierung und Hardware-SPI-Übergabe wurden nicht ersetzt.

**Status:** Source-/UI-Prototyp. Bis zum lokalen Compile und Hardwaretest
nicht als erfolgreich geflashte Firmware bewerten.
