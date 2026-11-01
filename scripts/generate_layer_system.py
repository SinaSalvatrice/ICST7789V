#!/usr/bin/env python3
"""Generate QMK layer metadata and 320x240 SVG screen previews.
The only editable definition of layer labels/keymaps/icons is design/layer_system.json.
Requires Python 3.9+ stdlib only. Runs locally; never calls GitHub Actions.
"""
import json
import re
from pathlib import Path
from xml.sax.saxutils import escape

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "design/layer_system.json"
OUTPUT = ROOT / "keyboards/icst7789v/keymaps/default/generated_layers.h"
PREVIEWS = ROOT / "design/previews"

def rgb565(h):
    return ((int(h[1:3], 16) >> 3) << 11 |
            (int(h[3:5], 16) >> 2) << 5 |
            int(h[5:7], 16) >> 3)

def q(s):
    return json.dumps(s, ensure_ascii=True)

def icon_rows(layer):
    bitmap = layer["icon"] + ["................"] * 16
    return [int(row.replace("#", "1").replace(".", "0"), 2) for row in bitmap[:16]]

def validate(data):
    layers = data["layers"]
    assert len(layers) == 11 and layers[0]["id"] == "HOME", "Need HOME + 10 user layers"
    assert data["key_layout"] == [4,4,3] and data["wide_key"] == 11
    assert len(data["home_double_targets"]) == 11
    assert data["gestures"]["k0"]["dedicated_module_only"] is True
    ids = [l["id"] for l in layers]
    assert len(ids) == len(set(ids))
    for target in data["home_double_targets"]:
        assert target in ids, target
    for l in layers:
        assert re.fullmatch(r"#[0-9A-Fa-f]{6}", l["accent"])
        assert len(l["name"]) <= 9 and len(l["keys"]) == 11, l["id"]
        assert len(l["icon"]) <= 16
        assert all(len(row) == 16 and set(row) <= set(".#") for row in l["icon"])
        for key in l["keys"]:
            assert 1 <= len(key["label"]) <= 5
            for action in ("tap", "hold"):
                assert re.fullmatch(r"[A-Z0-9_(),]+", key[action]), key[action]
        for action in ("ccw", "cw", "press"):
            assert re.fullmatch(r"[A-Z0-9_(),]+", l["encoder"][action])

def generate_header(data):
    layers = data["layers"]
    ids = [l["id"] for l in layers]
    out = ["// GENERATED from design/layer_system.json: never edit manually.",
           "// Regenerate with: python scripts/generate_layer_system.py",
           "#pragma once", "#include <stdint.h>",
           "#define CC_LAYER_COUNT " + str(len(layers)),
           "#define CC_KEY_COUNT 11",
           "typedef struct {", "  const char *name;", "  uint16_t accent_rgb565;",
           "  const char *labels[CC_KEY_COUNT];", "  uint16_t taps[CC_KEY_COUNT];",
           "  uint16_t holds[CC_KEY_COUNT];", "  uint16_t encoder_ccw;",
           "  uint16_t encoder_cw;", "  uint16_t encoder_press;",
           "  uint16_t icon[16];", "} cc_layer_desc_t;",
           "static const uint8_t cc_double_target[CC_KEY_COUNT] = {" +
           ", ".join(str(ids.index(i)) for i in data["home_double_targets"]) + "};",
           "static const cc_layer_desc_t cc_layers[CC_LAYER_COUNT] = {"]
    for l in layers:
        out.append('  {' + q(l["name"]) +
                   ', 0x' + format(rgb565(l["accent"]), "04X") +
                   ',\n   {' + ", ".join(q(k["label"]) for k in l["keys"]) +
                   '},\n   {' + ", ".join(k["tap"] for k in l["keys"]) +
                   '},\n   {' + ", ".join(k["hold"] for k in l["keys"]) +
                   '},\n   ' + ", ".join(l["encoder"][p] for p in ("ccw", "cw", "press")) +
                   ',\n   {' + ", ".join("0x%04X" % r for r in icon_rows(l)) +
                   '}},')
    out += ["};", ""]
    return "\n".join(out)

def preview(data, selected, filename):
    layers = data["layers"]
    name = "HOME" if selected == 0 else layers[selected]["name"]
    color = layers[selected]["accent"]
    parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="640" height="480" viewBox="0 0 320 240">',
             '<rect width="320" height="240" fill="#10151C"/>',
             '<rect width="320" height="26" fill="#18242E"/>',
             '<text x="8" y="17" font-family="Arial" font-size="10" font-weight="bold" fill="#E8F3F5">CC / LAYERS</text>',
             '<rect x="252" y="5" width="60" height="17" rx="6" fill="#284149"/>',
             '<text x="282" y="17" font-family="Arial" text-anchor="middle" font-size="9" fill="#75E2D5">DISPLAY</text>',
             '<rect x="7" y="33" width="109" height="171" rx="8" fill="#1C2D38"/>',
             '<rect x="7" y="33" width="109" height="4" rx="2" fill="'+color+'"/>']
    for y,row in enumerate((layers[selected]["icon"] + ["................"] * 16)[:16]):
        for x,v in enumerate(row):
            if v == "#":
                parts.append('<rect x="%d" y="%d" width="4" height="4" fill="%s"/>' %
                             (29+x*4, 63+y*4, color))
    parts += ['<text x="62" y="156" text-anchor="middle" font-family="Arial" font-weight="bold" font-size="14" fill="#FFFFFF">'+escape(name)+'</text>',
              '<text x="62" y="173" text-anchor="middle" font-family="Arial" font-size="9" fill="#ACBAC4">'+("2X = LAYER" if selected==0 else "LAYER %02d"%selected)+'</text>']
    for i in range(11):
        x=125+(i%4)*47
        y=40+(i//4)*55
        width=90 if i==10 else 43
        label = (layers[data["home_double_targets"].index(layers[i+1]["id"])+1]["name"] if False else
                 data["home_double_targets"][i] if selected==0 else layers[selected]["keys"][i]["label"])
        label=label[:5]
        parts+=['<rect x="%d" y="%d" width="%d" height="47" rx="4" fill="%s" stroke="%s"/>' %
                (x,y,width, "#254653" if i==10 else "#263744", "#43737F" if i==10 else "#40505E"),
                '<text x="%d" y="%d" font-family="Arial" font-size="8" fill="#8EADBC">K%d</text>'%(x+5,y+11,i+1),
                '<text x="%d" y="%d" text-anchor="middle" font-family="Arial" font-weight="bold" font-size="10" fill="#F4F8FB">%s</text>'%
                 (x+width//2,y+31,escape(label))]
    parts+=['<rect x="0" y="216" width="320" height="24" fill="#18242E"/>',
             '<text x="7" y="231" font-family="Arial" font-size="9" fill="#C6D7DD">K0  1x MODE   2x HOME   HOLD MENU</text>',
             '<text x="312" y="231" font-family="Arial" font-size="8" text-anchor="end" fill="'+color+'">'+("TAP DANCE" if selected==0 else "MACRO MAP")+'</text>',
             '</svg>']
    PREVIEWS.mkdir(parents=True, exist_ok=True)
    (PREVIEWS / filename).write_text("\n".join(parts)+"\n",encoding="utf-8")

def main():
    data=json.loads(DATA.read_text(encoding="utf-8"))
    validate(data)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(generate_header(data),encoding="utf-8")
    preview(data,0,"home.svg")
    preview(data,1,"edit.svg")
    preview(data,3,"media.svg")
    print("Generated layer header and 3 SVG screen previews.")

if __name__ == "__main__":
    main()
