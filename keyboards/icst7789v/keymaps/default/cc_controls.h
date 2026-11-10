// QMK Tap Dance + dedicated K0 + dual-mode EC11 controls.
#pragma once

static void cc_encoder_press_action(void) {
    if (cc_matrix_test_view) {
        cc_matrix_test_exit(); // Encoder PUSH leaves the test without host input.
        return;
    }
    if (cc_color_view || cc_help_view) {
        cc_color_view = false;
        cc_help_view = false;
        cc_dirty = true;
        return;
    }
    if (cc_menu_view) {
        switch (cc_menu_selection) {
            case 0: cc_switch_encoder(true); break;
            case 1: cc_switch_encoder(false); break;
            case 2: cc_open_home(); break;
            case 3: cc_matrix_test_enter(); break;
            case 4:
                cc_menu_view = false;
                cc_help_view = true;
                cc_dirty = true;
                break;
            case 5:
                cc_menu_view = false;
                cc_color_view = true;
                cc_dirty = true;
                break;
        }
        return;
    }
    if (cc_display_mode) {
        cc_activate_layer(cc_preview);
    } else {
        uint16_t kc = cc_layers[cc_active].encoder_press;
        if (kc != KC_NO) tap_code16(kc);
    }
}

// All 10 macro keys participate in Tap Dance; K0 is not a matrix key.
static void cc_tap_finished(tap_dance_state_t *state, void *data) {
    (void)data;
    const uint8_t index = state->index;
    if (cc_matrix_test_view || index >= CC_KEY_COUNT) return;
    if (state->count >= 3 && index == 9) {
        cc_activate_layer(cc_previous);
        return;
    }
    if (state->count >= 2) {
        cc_activate_layer(cc_double_target[index]);
        return;
    }
    const cc_layer_desc_t *desc = &cc_layers[cc_active];
    const uint16_t held = desc->holds[index];
    const uint16_t tap = desc->taps[index];
    if (state->pressed && !state->interrupted && held != KC_NO) {
        register_code16(held);
        cc_held_code[index] = held;
    } else if (tap != KC_NO) {
        tap_code16(tap);
    }
    cc_last_key = index;
    cc_dirty = true;
}

static void cc_tap_reset(tap_dance_state_t *state, void *data) {
    (void)data;
    const uint8_t index = state->index;
    if (index < CC_KEY_COUNT && cc_held_code[index] != KC_NO) {
        unregister_code16(cc_held_code[index]);
        cc_held_code[index] = KC_NO;
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [0]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [1]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [2]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [3]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [4]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [5]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [6]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [7]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [8]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
    [9]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cc_tap_finished, cc_tap_reset),
};

void keyboard_post_init_user(void) {
    gpio_set_pin_input_high(ENCODER_SW);
    gpio_set_pin_input_high(K0_SW);
#ifdef RGBLIGHT_ENABLE
    // Standalone 10-NeoPixel bring-up: visible rainbow chase on GP29.
    // The mode and brightness are RAM-only, never written to EEPROM.
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_RAINBOW_SWIRL);
    rgblight_sethsv_noeeprom(0, 255, 64);
#endif
    lcd_init(); // Known-good GPIO bootstrap then QMK HW SPI.
    cc_dirty = true;
}

static void cc_k0_update(void) {
    const bool raw = gpio_read_pin(K0_SW);
    if (raw != cc_k0.sample) {
        cc_k0.sample = raw;
        cc_k0.changed = timer_read();
    }
    if (cc_k0.sample != cc_k0.stable &&
        timer_elapsed(cc_k0.changed) >= CC_DEBOUNCE_MS) {
        cc_k0.stable = cc_k0.sample;
        if (!cc_k0.stable) {
            cc_k0_pressed = timer_read();
            cc_k0_held = false;
        } else if (!cc_k0_held) {
            // Delay 1x and 2x until the tap window closes. Otherwise
            // 2x HOME would trigger before the intended 3x MATRIX.
            ++cc_k0_tap_count;
            cc_k0_released = timer_read();
            if (cc_k0_tap_count >= 3) {
                cc_k0_tap_count = 0;
                cc_matrix_test_toggle();
            }
        }
    }
    if (!cc_k0.stable && !cc_k0_held &&
        timer_elapsed(cc_k0_pressed) >= CC_K0_HOLD_MS) {
        cc_k0_held = true;
        cc_k0_tap_count = 0;
        cc_open_menu();
    }
    if (cc_k0.stable && cc_k0_tap_count &&
        timer_elapsed(cc_k0_released) >= CC_K0_TAPS_MS) {
        const uint8_t taps = cc_k0_tap_count;
        cc_k0_tap_count = 0;
        if (taps == 2) {
            cc_open_home();
        } else if (cc_matrix_test_view) {
            // K0 1x also offers an immediate exit after the tap window.
            cc_matrix_test_exit();
        } else {
            cc_switch_encoder(!cc_display_mode);
        }
    }
}

static void cc_encoder_button_update(void) {
    const bool raw = gpio_read_pin(ENCODER_SW);
    if (raw != cc_encoder_button.sample) {
        cc_encoder_button.sample = raw;
        cc_encoder_button.changed = timer_read();
    }
    if (cc_encoder_button.sample != cc_encoder_button.stable &&
        timer_elapsed(cc_encoder_button.changed) >= CC_DEBOUNCE_MS) {
        cc_encoder_button.stable = cc_encoder_button.sample;
        if (!cc_encoder_button.stable) cc_encoder_press_action();
    }
}

// All key events are consumed locally in MATRIX TEST. This is before
// process_tap_dance in QMK, so no normal macro actions reach the host.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;
    if (!cc_matrix_test_view) return true;
    cc_matrix_test_record(record->event.key.row, record->event.key.col,
                          record->event.pressed);
    return false;
}

void matrix_scan_user(void) {
    // K0 (GP11) and encoder PUSH (GP10) are outside the matrix.
    cc_k0_update();
    cc_encoder_button_update();
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index != 0) return false;
    if (cc_matrix_test_view) {
        return false; // Never emit encoder host actions during diagnosis.
    }
    if (cc_menu_view) {
        cc_menu_selection = (cc_menu_selection + (clockwise ? 1 : 5)) % 6;
        cc_dirty = true;
    } else if (cc_display_mode) {
        if (cc_help_view || cc_color_view) return false;
        cc_preview = (cc_preview + (clockwise ? 1 : CC_LAYER_COUNT - 1)) %
                     CC_LAYER_COUNT;
        cc_home_view = cc_preview == 0;
        cc_dirty = true;
    } else {
        uint16_t kc = clockwise ? cc_layers[cc_active].encoder_cw :
                                  cc_layers[cc_active].encoder_ccw;
        if (kc != KC_NO) tap_code16(kc);
    }
    return false;
}

void housekeeping_task_user(void) {
    // No continuous refresh; input coalescing is important on 3.9MHz SPI.
    if (cc_dirty && timer_elapsed(cc_last_draw) >= 35) cc_redraw();
}
