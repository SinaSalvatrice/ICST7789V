#!/usr/bin/env bash
set -euo pipefail

KEYBOARD="icst7789v"
KEYMAP="default"

# Values passed from build_uf2.bat may still be Windows paths.
to_msys_path() {
    local value="$1"
    if [[ "$value" =~ ^[A-Za-z]:[\\/] ]]; then
        cygpath -u "$value"
    else
        printf '%s\n' "$value"
    fi
}

PROJECT_ROOT="$(to_msys_path "${PROJECT_ROOT:-$(cd "$(dirname "$0")" && pwd)}")"
QMK_HOME="$(to_msys_path "${QMK_HOME:-$HOME/qmk_firmware}")"

if [[ ! -f "$QMK_HOME/Makefile" || ! -d "$QMK_HOME/keyboards" ]]; then
    echo "ERROR: QMK firmware checkout not found at: $QMK_HOME" >&2
    exit 2
fi

# QMK's make system calls the qmk CLI internally. Make sure Python can resolve
# a real home directory when this script is launched from a Windows runner.
if [[ -n "${USERPROFILE:-}" ]]; then
    export HOME="$(cygpath -u "$USERPROFILE")"
fi

# The QMK header must reflect the editable design spec before copying sources.
if command -v python3 >/dev/null 2>&1; then
    python3 "$PROJECT_ROOT/scripts/generate_layer_system.py"
elif [[ ! -f "$PROJECT_ROOT/keyboards/$KEYBOARD/keymaps/$KEYMAP/generated_layers.h" ]]; then
    echo "ERROR: Python 3 required to generate layer metadata" >&2
    exit 4
else
    echo "WARNING: python3 unavailable here; using existing generated header"
fi

cd "$QMK_HOME"

echo "QMK_HOME: $QMK_HOME"
echo "Initializing only the submodules needed by RP2040 + USB + Quantum Painter..."
for module in \
    lib/chibios \
    lib/chibios-contrib \
    lib/pico-sdk \
    lib/lufa \
    lib/printf
do
    git submodule update --init --checkout "$module"
done

echo "Installing keyboard definition..."
rm -rf "keyboards/$KEYBOARD"
cp -R "$PROJECT_ROOT/keyboards/$KEYBOARD" "keyboards/$KEYBOARD"

rm -f "${KEYBOARD}_${KEYMAP}.uf2"

echo "Building $KEYBOARD:$KEYMAP..."
# SKIP_GIT avoids QMK's normal full recursive submodule scan. The required
# RP2040 submodules were initialized explicitly above.
make SKIP_GIT=1 "$KEYBOARD:$KEYMAP"

UF2="$QMK_HOME/${KEYBOARD}_${KEYMAP}.uf2"
if [[ ! -f "$UF2" ]]; then
    echo "ERROR: Build finished but UF2 was not found at: $UF2" >&2
    exit 3
fi

mkdir -p "$PROJECT_ROOT/build"
cp -f "$UF2" "$PROJECT_ROOT/build/ICST7789V.uf2"

echo
echo "SUCCESS: $PROJECT_ROOT/build/ICST7789V.uf2"
