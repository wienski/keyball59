/*
 * Copyright 2025 Yegor Wienski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

#include "transactions.h"

#ifdef RGB_MATRIX_ENABLE
led_config_t g_led_config = {
    {
        {  0,      9,      10,     19,     20,     29,     NO_LED },
        {  1,      8,      11,     18,     21,     28,     NO_LED },
        {  2,      7,      12,     17,     22,     27,     NO_LED },
        {  3,      6,      13,     16,     23,     26,     NO_LED },
        {  4,      5,      14,     15,     24,     25,     30     },

        { 58,     49,      48,     41,     40,     31,    NO_LED },
        { 57,     50,      47,     42,     39,     32,    NO_LED },
        { 56,     51,      46,     43,     38,     33,    NO_LED },
        { 55,     52,      45,     44,     37,     34,    NO_LED },
        { 54,     53,      NO_LED, NO_LED, 36,     35,    NO_LED },
    }, {
        { 0, 13 }, { 0, 26 }, { 0, 38 }, { 0, 51 }, { 0, 64 },
        { 16, 64 }, { 16, 51 }, { 16, 38 }, { 16, 26 }, { 16, 13 },
        { 32, 9 }, { 32, 22 }, { 32, 35 }, { 32, 47 }, { 32, 60 },
        { 48, 58 }, { 48, 45 }, { 48, 32 }, { 48, 19 }, { 48, 6 },
        { 64, 8 }, { 64, 20 }, { 64, 33 }, { 64, 46 }, { 64, 59 },
        { 80, 60 }, { 80, 47 }, { 80, 35 }, { 80, 22 }, { 80, 9 },
        { 96, 64 },

        { 224, 13 }, { 224, 26 }, { 224, 38 }, { 224, 51 }, { 224, 64 },
        { 208, 64 }, { 208, 51 }, { 208, 38 }, { 208, 26 }, { 208, 13 },
        { 192, 9 }, { 192, 22 }, { 192, 35 }, { 192, 47 },
        { 176, 45 }, { 176, 32 }, { 176, 19 }, { 176, 6 },
        { 160, 8 }, { 160, 20 }, { 160, 33 }, { 160, 46 }, { 160, 59 },
        { 144, 60 }, { 144, 47 }, { 144, 35 }, { 144, 22 }, { 144, 9 }
    }, {
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_INDICATOR,
        LED_FLAG_INDICATOR, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_INDICATOR,

        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_INDICATOR,
        LED_FLAG_INDICATOR, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT
    }
};
#endif

#define TB_MAX_CPI 12000
#define TB_MIN_CPI 100
#define TB_SCROLL_SMOOTHNESS 8

int16_t tb_scroll_acc_h = 0;
int16_t tb_scroll_acc_v = 0;
bool tb_cpi_dirty = true;

typedef union {
    uint32_t raw;
    struct {
        uint8_t pointing_multiplier : 7;
        uint8_t scrolling_multiplier : 7;
        uint8_t sniping_multiplier : 7;
        bool do_scroll : 1;
        bool do_snipe : 1;
    } __attribute__((packed));
} keyball_config_t;

keyball_config_t keyball_config = {
    .pointing_multiplier = 14,
    .scrolling_multiplier = 8,
    .sniping_multiplier = 4,
    .do_scroll = false,
    .do_snipe = false
};

static void tb_handle_sync_config(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    keyball_config.raw = *(uint32_t*)in_data;
    tb_cpi_dirty = true;
}

static void tb_update_cpi(void) {
    if (!tb_cpi_dirty) return;

    // if scrolling and sniping at the same time, use sniping cpi
    if (keyball_config.do_snipe) {
        pointing_device_set_cpi(100 * keyball_config.sniping_multiplier);
    } else if (keyball_config.do_scroll) {
        pointing_device_set_cpi(100 * keyball_config.scrolling_multiplier);
    } else {
        pointing_device_set_cpi(100 * keyball_config.pointing_multiplier);
    }

    if (is_keyboard_master()) {
        if (!transaction_rpc_send(TB_SYNC_CONFIG, sizeof(keyball_config.raw), &keyball_config.raw)) {
            return;
        }
    }

    tb_cpi_dirty = false;
}

void keyboard_post_init_kb(void) {
    keyball_config.raw = eeconfig_read_kb();

    if (!is_keyboard_master()) {
        transaction_register_rpc(TB_SYNC_CONFIG, tb_handle_sync_config);
    }

    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    tb_update_cpi();
}

static void persist_config_to_eeprom(void) {
    eeconfig_update_kb(keyball_config.raw);
}

void tb_adjust_cpi(uint8_t steps) {
    if (!keyball_config.do_scroll && !keyball_config.do_snipe) {
        keyball_config.pointing_multiplier += steps;
        while (keyball_config.pointing_multiplier < TB_MIN_CPI / 100) {
            keyball_config.pointing_multiplier += 1;
        }
        while (keyball_config.pointing_multiplier > TB_MAX_CPI / 100) {
            keyball_config.pointing_multiplier -= 1;
        }
    }

    if (keyball_config.do_scroll) {
        keyball_config.scrolling_multiplier += steps;
        while (keyball_config.scrolling_multiplier < TB_MIN_CPI / 100) {
            keyball_config.scrolling_multiplier += 1;
        }
        while (keyball_config.scrolling_multiplier > TB_MAX_CPI / 100) {
            keyball_config.scrolling_multiplier -= 1;
        }
    }

    if (keyball_config.do_snipe) {
        keyball_config.sniping_multiplier += steps;
        while (keyball_config.sniping_multiplier < TB_MIN_CPI / 100) {
            keyball_config.sniping_multiplier += 1;
        }
        while (keyball_config.sniping_multiplier > TB_MAX_CPI / 100) {
            keyball_config.sniping_multiplier -= 1;
        }
    }

    tb_cpi_dirty = true;
}

report_mouse_t pointing_device_task_kb(report_mouse_t report) {
    report = pointing_device_task_user(report);

    if (keyball_config.do_scroll) {
#if defined(POINTING_DEVICE_INVERT_X) == defined(NATURAL_SCROLLING)
        tb_scroll_acc_h += report.x;
#else
        tb_scroll_acc_h -= report.x;
#endif

        if (abs(tb_scroll_acc_h) > TB_SCROLL_SMOOTHNESS) {
            report.x = tb_scroll_acc_h / TB_SCROLL_SMOOTHNESS;
            // I don't know if this is faster than just using modulo
            tb_scroll_acc_h -= report.x * TB_SCROLL_SMOOTHNESS;
            report.h += report.x;
        }

        report.x = 0;

#if defined(POINTING_DEVICE_INVERT_Y) == defined(NATURAL_SCROLLING)
        tb_scroll_acc_v += report.y;
#else
        tb_scroll_acc_v -= report.y;
#endif

        if (abs(tb_scroll_acc_v) > TB_SCROLL_SMOOTHNESS) {
            report.y = tb_scroll_acc_v / TB_SCROLL_SMOOTHNESS;
            // I don't know if this is faster than just using modulo
            tb_scroll_acc_v -= report.y * TB_SCROLL_SMOOTHNESS;
            report.v += report.y;
        }

        report.y = 0;
    }

    return report;
}

// should be a power of 2, and fit into uint8_t
// that is, 2, 4, 8, 16, 32, 64, or 128
#ifndef CK_LOCKED_KEYS_MAX
#    define CK_LOCKED_KEYS_MAX 16
#endif

uint8_t ck_lock_records_head = 0;
uint8_t ck_lock_records_count = 0;
uint16_t ck_locked_keycodes[CK_LOCKED_KEYS_MAX] = {};
// todo: use a bitmap
bool ck_lock_records[CK_LOCKED_KEYS_MAX] = {};

#define LOCKED_LAST ((ck_lock_records_head + ck_lock_records_count - 1) & (CK_LOCKED_KEYS_MAX - 1))
#define LOCKED_NEXT ((ck_lock_records_head + ck_lock_records_count) & (CK_LOCKED_KEYS_MAX - 1))

static void ck_unlock(uint8_t i) {
    ck_locked_keycodes[i] = ck_locked_keycodes[ck_lock_records_head];
    ck_lock_records[i] = ck_lock_records[ck_lock_records_head];
    ck_locked_keycodes[ck_lock_records_head] = CK_LOCK;

    ck_lock_records_count -= 1;
    ck_lock_records_head += 1;
    if (ck_lock_records_head >= CK_LOCKED_KEYS_MAX) {
        ck_lock_records_head -= CK_LOCKED_KEYS_MAX;
    }
}

static bool lock_is_waiting_for_input(void) {
    return ck_lock_records_count > 0
        && ck_locked_keycodes[LOCKED_LAST] == CK_LOCK;
}

static bool ck_lock_preprocess_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == CK_LOCK) return true;

    for (uint8_t i = 0; i < ck_lock_records_count; i++) {
        uint8_t j = (ck_lock_records_head + i) & (CK_LOCKED_KEYS_MAX - 1);
        if (ck_locked_keycodes[j] != keycode) continue;

        if (record->event.pressed) {
            // this is the second press
            ck_lock_records[j] = true;
            return false;
        } else if (ck_lock_records[j]) {
            // this is the second release
            ck_unlock(j);
            return true;
        } else {
            ck_lock_records[j] = false;
            return false;
        }
    }

    return true;
}

static void ck_lock_process_record(uint16_t keycode, keyrecord_t *record) {
    if (
        lock_is_waiting_for_input()
        && record->event.pressed
    ) {
        ck_locked_keycodes[LOCKED_LAST] = keycode;
    }
}

static void process_ck_lock(void) {
    if (lock_is_waiting_for_input()) {
        // cancel locking on second CK_LOCK press
        ck_lock_records_count -= 1;
        return;
    }

    if (ck_lock_records_count < CK_LOCKED_KEYS_MAX) {
        ck_locked_keycodes[LOCKED_NEXT] = CK_LOCK;
        ck_lock_records_count += 1;
    }
}

#ifndef CK_TURBO_PERIOD
#    define CK_TURBO_PERIOD 80
#endif

// should be a power of 2, and fit into uint8_t
// that is, 2, 4, 8, 16, 32, 64, or 128
#ifndef CK_TURBO_KEYS_MAX
#    define CK_TURBO_KEYS_MAX 16
#endif

uint8_t ck_turbo_records_head = 0;
uint8_t ck_turbo_records_count = 0;
uint16_t ck_turbo_keycodes[CK_TURBO_KEYS_MAX] = {};
uint8_t ck_turbo_records[CK_TURBO_KEYS_MAX] = {};
deferred_token ck_turbo_tokens[CK_TURBO_KEYS_MAX] = {};

#define TURBO_LAST ((ck_turbo_records_head + ck_turbo_records_count - 1) & (CK_TURBO_KEYS_MAX - 1))
#define TURBO_NEXT ((ck_turbo_records_head + ck_turbo_records_count) & (CK_TURBO_KEYS_MAX - 1))

enum {
    TURBO_READY = 0,
    TURBO_PRESSED,
    TURBO_RELEASED
};

static uint32_t ck_turbo_tick(uint32_t trigger_time, void *cb_arg) {
    uint8_t i = (uint8_t)(size_t)cb_arg;

    if (ck_turbo_records[i] == TURBO_PRESSED) {
        unregister_code16(ck_turbo_keycodes[i]);
        ck_turbo_records[i] = TURBO_RELEASED;
    } else {
        register_code16(ck_turbo_keycodes[i]);
        ck_turbo_records[i] = TURBO_PRESSED;
    }

    return CK_TURBO_PERIOD >> 1;
}

static void ck_turbo_start(uint8_t i) {
    if (ck_turbo_tokens[i] != INVALID_DEFERRED_TOKEN) return;

    uint32_t next_trigger_time = ck_turbo_tick(0, (void *)(size_t)i);
    ck_turbo_tokens[i] = defer_exec(next_trigger_time, ck_turbo_tick, (void *)(size_t)i);
}

static void ck_turbo_stop(uint8_t i) {
    if (ck_turbo_tokens[i] == INVALID_DEFERRED_TOKEN) return;

    cancel_deferred_exec(ck_turbo_tokens[i]);
    ck_turbo_tokens[i] = INVALID_DEFERRED_TOKEN;

    if (ck_turbo_records[i] == TURBO_PRESSED) {
        unregister_code16(ck_turbo_keycodes[i]);
        ck_turbo_records[i] = TURBO_RELEASED;
    }
}

static void ck_turbo_off(uint8_t i) {
    ck_turbo_stop(i);

    ck_turbo_keycodes[i] = ck_turbo_keycodes[ck_turbo_records_head];
    ck_turbo_records[i] = ck_turbo_records[ck_turbo_records_head];
    ck_turbo_tokens[i] = ck_turbo_tokens[ck_turbo_records_head];
    ck_turbo_keycodes[ck_turbo_records_head] = CK_TURBO;

    ck_turbo_records_count -= 1;
    ck_turbo_records_head += 1;
    if (ck_turbo_records_head >= CK_TURBO_KEYS_MAX) {
        ck_turbo_records_head -= CK_TURBO_KEYS_MAX;
    }
}

static bool turbo_is_waiting_for_input(void) {
    return ck_turbo_records_count > 0
        && ck_turbo_keycodes[TURBO_LAST] == CK_TURBO;
}

static bool ck_turbo_preprocess_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == CK_TURBO) {
        if (record->event.pressed) {
            if (turbo_is_waiting_for_input()) {
                // cancel turbo on second CK_TURBO press
                ck_turbo_records_count -= 1;
            } else if (ck_turbo_records_count < CK_TURBO_KEYS_MAX) {
                ck_turbo_keycodes[TURBO_NEXT] = CK_TURBO;
                ck_turbo_records_count += 1;
            }
        }

        return false;
    }

    if (turbo_is_waiting_for_input() && record->event.pressed) {
        for (uint8_t i = 0; i < ck_turbo_records_count; i++) {
            uint8_t j = (ck_turbo_records_head + i) & (CK_TURBO_KEYS_MAX - 1);
            if (ck_turbo_keycodes[j] != keycode) continue;

            ck_turbo_off(j);
            ck_turbo_records_count -= 1;

            return false;
        }

        ck_turbo_keycodes[TURBO_LAST] = keycode;
        ck_turbo_records[TURBO_LAST] = TURBO_READY;

        return false;
    }

    for (uint8_t i = 0; i < ck_turbo_records_count; i++) {
        uint8_t j = (ck_turbo_records_head + i) & (CK_TURBO_KEYS_MAX - 1);

        if (ck_turbo_keycodes[j] != keycode) continue;

        if (record->event.pressed) {
            ck_turbo_start(j);
            return true;
        } else if (ck_turbo_records[j] == TURBO_READY) {
            // first release
            return false;
        } else {
            ck_turbo_stop(j);
            return true;
        }
    }

    return true;
}

void eeconfig_init_kb(void) {
    persist_config_to_eeprom();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!ck_lock_preprocess_record(keycode, record)) {
        return false;
    }

    if (!ck_turbo_preprocess_record(keycode, record)) {
        return false;
    }

    if (!process_record_user(keycode, record)) {
        return false;
    }

    ck_lock_process_record(keycode, record);

    switch (keycode) {
        case TB_SCROLL:
            keyball_config.do_scroll = record->event.pressed;
            tb_cpi_dirty = true;
            break;
        case TB_SNIPE:
            keyball_config.do_snipe = record->event.pressed;
            tb_cpi_dirty = true;
            break;
        case TB_CPI_DOWN:
            if (record->event.pressed) {
                tb_adjust_cpi(-1);
            }
            break;
        case TB_CPI_UP:
            if (record->event.pressed) {
                tb_adjust_cpi(1);
            }
            break;
        case CK_LOCK:
            if (record->event.pressed) {
                process_ck_lock();
            }
            break;
        case CK_SAVE:
            if (record->event.pressed) {
                persist_config_to_eeprom();
            }
            break;
        default:
            break;
    }

    return true;
}

#ifdef OLED_ENABLE

static void render_logo(void) {
    static const char PROGMEM qmk_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4,
        0x00
    };

    oled_write_P(qmk_logo, false);
}

static bool oled_write_keycode(uint16_t keycode) {
    switch (keycode) {
        case KC_A...KC_Z:
            oled_write_char(keycode - KC_A + 'a', false);
            return true;
        case KC_1...KC_9:
            oled_write_char(keycode - KC_1 + '1', false);
            return true;
        case KC_0:
            oled_write_char('0', false);
            return true;
        case KC_ENTER:
            oled_write_char(0x06, false);
            return true;
        case KC_ESCAPE:
            oled_write_char(0x0a, false);
            return true;
        case KC_BACKSPACE:
            oled_write_char(0x04, false);
            return true;
        case KC_TAB:
            oled_write_char(0x08, false);
            return true;
        case KC_SPACE:
            oled_write_char(0x03, false);
            return true;
        case KC_MINUS:
            oled_write_char('-', false);
            return true;
        case KC_EQUAL:
            oled_write_char('=', false);
            return true;
        case KC_LEFT_BRACKET:
            oled_write_char('[', false);
            return true;
        case KC_RIGHT_BRACKET:
            oled_write_char(']', false);
            return true;
        case KC_BACKSLASH:
            oled_write_char('\\', false);
            return true;
        case KC_SEMICOLON:
            oled_write_char(';', false);
            return true;
        case KC_QUOTE:
            oled_write_char('\'', false);
            return true;
        case KC_GRAVE:
            oled_write_char('`', false);
            return true;
        case KC_COMMA:
            oled_write_char(',', false);
            return true;
        case KC_DOT:
            oled_write_char('.', false);
            return true;
        case KC_SLASH:
            oled_write_char('/', false);
            return true;
        case KC_F1...KC_F9:
            oled_write_char('f', false);
            oled_write_char(keycode - KC_F1 + '1', false);
            return true;
        case KC_F10...KC_F12:
            oled_write_char('f', false);
            oled_write_char(keycode - KC_F1 + 'a', false);
            return true;
        case KC_RIGHT:
            oled_write_char(0x1a, false);
            return true;
        case KC_LEFT:
            oled_write_char(0x1b, false);
            return true;
        case KC_DOWN:
            oled_write_char(0x19, false);
            return true;
        case KC_UP:
            oled_write_char(0x18, false);
            return true;
        case KC_HOME:
            oled_write_char(0x0b, false);
            return true;
        case KC_END:
            oled_write_char(0x0c, false);
            return true;
        case KC_PAGE_UP:
            oled_write_char(0x0e, false);
            return true;
        case KC_PAGE_DOWN:
            oled_write_char(0x0d, false);
            return true;
        case KC_DELETE:
            oled_write_char(0x05, false);
            return true;
        case KC_LEFT_CTRL:
        case KC_RIGHT_CTRL:
            oled_write_char(0x5e, false);
            return true;
        case KC_LEFT_SHIFT:
        case KC_RIGHT_SHIFT:
            oled_write_char(0x01, false);
            return true;
        case KC_LEFT_GUI:
        case KC_RIGHT_GUI:
            oled_write_char(0x09, false);
            return true;
        case KC_LEFT_ALT:
            oled_write_char(0x02, false);
            return true;
        case KC_RIGHT_ALT:
            oled_write_char(0x07, false);
            return true;
        case QK_MOUSE_BUTTON_1...QK_MOUSE_BUTTON_3:
            oled_write_char(0x14 + keycode - QK_MOUSE_BUTTON_1, false);
            return true;
        case QK_MOUSE_BUTTON_4...QK_MOUSE_BUTTON_8:
            oled_write_char(0x17, false);
            oled_write_char(keycode - QK_MOUSE_BUTTON_1 + '1', false);
            return true;
        case TB_SCROLL:
        case TB_SNIPE:
            // handled by the right half of the keyboard,
            // don't show here
        case KC_NO:
        case CK_LOCK:
        case CK_TURBO:
            return false;
        default:
            oled_write_char(0, false);
            return true;
    }
}

oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

bool oled_task_kb(void) {
    static const char PROGMEM pointing_cpi[] = {
        0x20, 0x20, 0x43, 0x50, 0x49, 0x00
    };

    static const char PROGMEM scrolling_cpi[] = {
        0x1c, 0x20, 0x43, 0x50, 0x49, 0x00
    };

    static const char PROGMEM sniping_cpi[] = {
        0x1d, 0x20, 0x43, 0x50, 0x49, 0x00
    };

    static const char PROGMEM hundred[] = {
        0x30, 0x30, 0x00
    };

    static const char PROGMEM big_numbers_hi[][2] = {
        { 0x8d, 0x8e }, { 0x8f, 0x90 }, { 0xcd, 0xce }, { 0x93, 0x94 }, { 0xd1, 0xd2 },
        { 0x97, 0x98 }, { 0xd5, 0xd6 }, { 0x91, 0x92 }, { 0x95, 0x96 }, { 0xb7, 0xb8 }
    };

    static const char PROGMEM big_numbers_lo[][2] = {
        { 0xad, 0xae }, { 0xaf, 0xb0 }, { 0xcf, 0xd0 }, { 0xb3, 0xb4 }, { 0xd3, 0xd4 },
        { 0xb3, 0xb4 }, { 0xb5, 0xb6 }, { 0xb1, 0xb2 }, { 0xb5, 0xb6 }, { 0xd7, 0xd8 }
    };

    if(!oled_task_user()) { return false; }

    oled_clear();

    if (is_keyboard_left()) {
        // the screen is 16 rows x 5 cols

        uint8_t layer = get_highest_layer(layer_state);

        oled_write_char(big_numbers_hi[layer][0], false);
        oled_write_char(big_numbers_hi[layer][1], false);

        oled_write_char(' ', false);

        if (get_mods() & MOD_MASK_GUI) {
            oled_write_char(0xc9, false);
            oled_write_char(0xca, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(big_numbers_lo[layer][0], false);
        oled_write_char(big_numbers_lo[layer][1], false);

        oled_write_char(' ', false);

        if (get_mods() & MOD_MASK_GUI) {
            oled_write_char(0xcb, false);
            oled_write_char(0xcc, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        if (get_mods() & MOD_MASK_CTRL) {
            oled_write_char(0xc5, false);
            oled_write_char(0xc6, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);

        if (get_mods() & MOD_MASK_SHIFT) {
            oled_write_char(0xc5, false);
            oled_write_char(0xc6, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);
        oled_write_char(' ', false);
        oled_write_char(' ', false);

        if (get_mods() & MOD_MASK_SHIFT) {
            oled_write_char(0xc7, false);
            oled_write_char(0xc8, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        if (get_mods() & MOD_BIT(KC_LALT)) {
            oled_write_char(0x89, false);
            oled_write_char(0x8a, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);

        if (get_mods() & MOD_BIT(KC_RALT)) {
            oled_write_char(0x8b, false);
            oled_write_char(0x8c, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        if (get_mods() & MOD_BIT(KC_LALT)) {
            oled_write_char(0xa9, false);
            oled_write_char(0xaa, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);

        if (get_mods() & MOD_BIT(KC_RALT)) {
            oled_write_char(0xab, false);
            oled_write_char(0xac, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        if (turbo_is_waiting_for_input()) {
            oled_write_char(0x85, false);
            oled_write_char(0x86, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);

        if (lock_is_waiting_for_input()) {
            oled_write_char(0x87, false);
            oled_write_char(0x88, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        if (turbo_is_waiting_for_input()) {
            oled_write_char(0xa5, false);
            oled_write_char(0xa6, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);

        if (lock_is_waiting_for_input()) {
            oled_write_char(0xa7, false);
            oled_write_char(0xa8, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(0x0f, false);
        bool need_space = true;
        uint8_t i, j;

        for (i = 0; i < ck_turbo_records_count; i++) {
            j = (ck_turbo_records_head + i) & (CK_TURBO_KEYS_MAX - 1);

            if (need_space) oled_write_char(' ', false);
            need_space = oled_write_keycode(ck_turbo_keycodes[j]);
        }

        oled_set_cursor(0, 12);
        oled_write_char(0x013, false);
        need_space = true;

        for (i = 0; i < ck_lock_records_count; i++) {
            j = (ck_lock_records_head + i) & (CK_LOCKED_KEYS_MAX - 1);

            if (need_space) oled_write_char(' ', false);
            need_space = oled_write_keycode(ck_locked_keycodes[j]);
        }
    } else {
        render_logo();

        oled_set_cursor(0, 5);

        if (keyball_config.do_scroll) {
            oled_write_char(0x9b, false);
            oled_write_char(0x9c, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);

        if (keyball_config.do_snipe) {
            oled_write_char(0x99, false);
            oled_write_char(0x9a, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        if (keyball_config.do_scroll) {
            oled_write_char(0xbb, false);
            oled_write_char(0xbc, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_char(' ', false);

        if (keyball_config.do_snipe) {
            oled_write_char(0xb9, false);
            oled_write_char(0xba, false);
        } else {
            oled_write_char(' ', false);
            oled_write_char(' ', false);
        }

        oled_write_P(pointing_cpi, false);
        oled_write(get_u8_str(keyball_config.pointing_multiplier, ' '), false);
        oled_write_P(hundred, false);

        oled_write_P(scrolling_cpi, false);
        oled_write(get_u8_str(keyball_config.scrolling_multiplier, ' '), false);
        oled_write_P(hundred, false);

        oled_write_P(sniping_cpi, false);
        oled_write(get_u8_str(keyball_config.sniping_multiplier, ' '), false);
        oled_write_P(hundred, false);
    }

    return false;
}

bool shutdown_kb(bool jump_to_bootloader) {
    oled_clear();

    oled_set_cursor(0, 6);
    render_logo();

    oled_render_dirty(true);
    return false;
}

#endif
