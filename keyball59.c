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
        { 0, 0 }, { 0, 16 }, { 0, 32 }, { 0, 48 }, { 0, 64 },
        { 45, 64 }, { 45, 48 }, { 45, 32 }, { 45, 16 }, { 45, 0 },
        { 90, 0 }, { 90, 16 }, { 90, 32 }, { 90, 48 }, { 90, 64 },
        { 134, 64 }, { 134, 48 }, { 134, 32 }, { 134, 16 }, { 134, 0 },
        { 179, 0 }, { 179, 16 }, { 179, 32 }, { 179, 48 }, { 179, 64 },
        { 224, 64 }, { 224, 48 }, { 224, 32 }, { 224, 16 }, { 224, 0 },
        { 224, 64 },

        { 224, 0 }, { 224, 16 }, { 224, 32 }, { 224, 48 }, { 224, 64 },
        { 179, 64 }, { 179, 48 }, { 179, 32 }, { 179, 16 }, { 179, 0 },
        { 134, 0 }, { 134, 16 }, { 134, 32 }, { 134, 48 },
        { 90, 48 }, { 90, 32 }, { 90, 16 }, { 90, 0 },
        { 45, 0 }, { 45, 16 }, { 45, 32 }, { 45, 48 }, { 45, 64 },
        { 0, 64 }, { 0, 48 }, { 0, 32 }, { 0, 16 }, { 0, 0 }
    }, {
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT,

        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT,
        LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT, LED_FLAG_KEYLIGHT
    }
};
#endif

#define TB_MAX_CPI 12000
#define TB_MIN_CPI 100
#define TB_SCROLL_SMOOTHNESS 8

uint16_t tb_cpi = 1400;
bool tb_do_scroll = false;
uint16_t tb_scroll_cpi = 800;
int16_t tb_scroll_acc_h = 0;
int16_t tb_scroll_acc_v = 0;
bool tb_do_snipe = false;
uint16_t tb_snipe_cpi = 400;

void tb_adjust_cpi(uint16_t cpi) {
    if (!tb_do_scroll && !tb_do_snipe) {
        tb_cpi += cpi;
        if (tb_cpi < TB_MIN_CPI) {
            tb_cpi = TB_MIN_CPI;
        }
        if (tb_cpi > TB_MAX_CPI) {
            tb_cpi = TB_MAX_CPI;
        }
    }

    if (tb_do_scroll) {
        tb_scroll_cpi += cpi;
        if (tb_scroll_cpi < TB_MIN_CPI) {
            tb_scroll_cpi = TB_MIN_CPI;
        }
        if (tb_scroll_cpi > TB_MAX_CPI) {
            tb_scroll_cpi = TB_MAX_CPI;
        }
    }

    if (tb_do_snipe) {
        tb_snipe_cpi += cpi;
        if (tb_snipe_cpi < TB_MIN_CPI) {
            tb_snipe_cpi = TB_MIN_CPI;
        }
        if (tb_snipe_cpi > TB_MAX_CPI) {
            tb_snipe_cpi = TB_MAX_CPI;
        }
    }
}

report_mouse_t pointing_device_task_kb(report_mouse_t report) {
    report = pointing_device_task_user(report);

    if (tb_do_scroll) {
#ifdef POINTING_DEVICE_INVERT_X
        tb_scroll_acc_h -= report.x;
#else
        tb_scroll_acc_h += report.x;
#endif

        if (abs(tb_scroll_acc_h) > TB_SCROLL_SMOOTHNESS) {
            report.x = tb_scroll_acc_h / TB_SCROLL_SMOOTHNESS;
            // I don't know if this is faster than just using modulo
            tb_scroll_acc_h -= report.x * TB_SCROLL_SMOOTHNESS;
            report.h += report.x;
        }

        report.x = 0;

#ifdef POINTING_DEVICE_INVERT_Y
        tb_scroll_acc_v -= report.y;
#else
        tb_scroll_acc_v += report.y;
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

void ck_unlock(uint8_t i) {
    ck_locked_keycodes[i] = ck_locked_keycodes[ck_lock_records_head];
    ck_lock_records[i] = ck_lock_records[ck_lock_records_head];

    ck_lock_records_count -= 1;
    ck_lock_records_head += 1;
    if (ck_lock_records_head >= CK_LOCKED_KEYS_MAX) {
        ck_lock_records_head -= CK_LOCKED_KEYS_MAX;
    }
}

bool ck_lock_preprocess_record(uint16_t keycode, keyrecord_t *record) {
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

void ck_lock_process_record(uint16_t keycode, keyrecord_t *record) {
    if (
        ck_lock_records_count > 0
        && record->event.pressed
    ) {
        uint8_t ck_lock_records_tail =
            (ck_lock_records_head + ck_lock_records_count - 1)
            & (CK_LOCKED_KEYS_MAX - 1);

        if (ck_locked_keycodes[ck_lock_records_tail] == CK_LOCK) {
            ck_locked_keycodes[ck_lock_records_tail] = keycode;
        }
    }
}

void process_ck_lock(void) {
    if (
        ck_lock_records_count > 0
    ) {
        uint8_t ck_lock_records_tail =
            (ck_lock_records_head + ck_lock_records_count - 1)
            & (CK_LOCKED_KEYS_MAX - 1);

        if (ck_locked_keycodes[ck_lock_records_tail] == CK_LOCK) {
            // cancel locking on second CK_LOCK press
            ck_lock_records_count -= 1;
            return;
        }
    }

    if (ck_lock_records_count < CK_LOCKED_KEYS_MAX) {
        uint8_t ck_lock_records_tail =
            (ck_lock_records_head + ck_lock_records_count)
            & (CK_LOCKED_KEYS_MAX - 1);

        ck_locked_keycodes[ck_lock_records_tail] = CK_LOCK;
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

enum {
    TURBO_READY = 0,
    TURBO_PRESSED,
    TURBO_RELEASED
};

uint32_t ck_turbo_tick(uint32_t trigger_time, void *cb_arg) {
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

void ck_turbo_start(uint8_t i) {
    if (ck_turbo_tokens[i] != INVALID_DEFERRED_TOKEN) return;

    uint32_t next_trigger_time = ck_turbo_tick(0, (void *)(size_t)i);
    ck_turbo_tokens[i] = defer_exec(next_trigger_time, ck_turbo_tick, (void *)(size_t)i);
}

void ck_turbo_stop(uint8_t i) {
    if (ck_turbo_tokens[i] == INVALID_DEFERRED_TOKEN) return;

    cancel_deferred_exec(ck_turbo_tokens[i]);
    ck_turbo_tokens[i] = INVALID_DEFERRED_TOKEN;

    if (ck_turbo_records[i] == TURBO_PRESSED) {
        unregister_code16(ck_turbo_keycodes[i]);
        ck_turbo_records[i] = TURBO_RELEASED;
    }
}

void ck_turbo_off(uint8_t i) {
    ck_turbo_stop(i);

    ck_turbo_keycodes[i] = ck_turbo_keycodes[ck_turbo_records_head];
    ck_turbo_records[i] = ck_turbo_records[ck_turbo_records_head];
    ck_turbo_tokens[i] = ck_turbo_tokens[ck_turbo_records_head];

    ck_turbo_records_count -= 1;
    ck_turbo_records_head += 1;
    if (ck_turbo_records_head >= CK_TURBO_KEYS_MAX) {
        ck_turbo_records_head -= CK_TURBO_KEYS_MAX;
    }
}

bool ck_turbo_preprocess_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == CK_TURBO) return true;

    if (
        ck_turbo_records_count > 0
        && record->event.pressed
    ) {
        uint8_t ck_turbo_records_tail =
            (ck_turbo_records_head + ck_turbo_records_count - 1)
            & (CK_TURBO_KEYS_MAX - 1);

        if (ck_turbo_keycodes[ck_turbo_records_tail] == CK_TURBO) {
            for (uint8_t i = 0; i < ck_turbo_records_count; i++) {
                uint8_t j = (ck_turbo_records_head + i) & (CK_TURBO_KEYS_MAX - 1);
                if (ck_turbo_keycodes[j] != keycode) continue;

                ck_turbo_off(j);
                // cancel turbo caused by last CK_TURBO press
                ck_turbo_records_count -= 1;

                return false;
            }
        }
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

bool ck_turbo_process_record(uint16_t keycode, keyrecord_t *record) {
    if (
        ck_turbo_records_count > 0
        && record->event.pressed
    ) {
        uint8_t ck_turbo_records_tail =
            (ck_turbo_records_head + ck_turbo_records_count - 1)
            & (CK_TURBO_KEYS_MAX - 1);

        if (ck_turbo_keycodes[ck_turbo_records_tail] == CK_TURBO) {
            ck_turbo_keycodes[ck_turbo_records_tail] = keycode;
            ck_turbo_records[ck_turbo_records_tail] = TURBO_READY;
            return false;
        }
    }

    return true;
}

void process_ck_turbo(void) {
    if (
        ck_turbo_records_count > 0
    ) {
        uint8_t ck_turbo_records_tail =
            (ck_turbo_records_head + ck_turbo_records_count - 1)
            & (CK_TURBO_KEYS_MAX - 1);

        if (ck_turbo_keycodes[ck_turbo_records_tail] == CK_TURBO) {
            // cancel turbo on second CK_TURBO press
            ck_turbo_records_count -= 1;
            return;
        }
    }

    if (ck_turbo_records_count < CK_TURBO_KEYS_MAX) {
        uint8_t ck_turbo_records_tail =
            (ck_turbo_records_head + ck_turbo_records_count)
            & (CK_TURBO_KEYS_MAX - 1);

        ck_turbo_keycodes[ck_turbo_records_tail] = CK_TURBO;
        ck_turbo_records_count += 1;
    }
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

    if (!ck_turbo_process_record(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case TB_SCROLL:
            tb_do_scroll = record->event.pressed;
            break;
        case TB_SNIPE:
            tb_do_snipe = record->event.pressed;
            break;
        case TB_CPI_DOWN:
            if (record->event.pressed) {
                tb_adjust_cpi(-100);
            }
            break;
        case TB_CPI_UP:
            if (record->event.pressed) {
                tb_adjust_cpi(100);
            }
            break;
        case CK_LOCK:
            if (record->event.pressed) {
                process_ck_lock();
            }
            break;
        case CK_TURBO:
            if (record->event.pressed) {
                process_ck_turbo();
            }
            break;
        default:
            break;
    }

    // if scrolling and sniping at the same time, use sniping cpi
    if (tb_do_snipe) {
        pointing_device_set_cpi(tb_snipe_cpi);
    } else if (tb_do_scroll) {
        pointing_device_set_cpi(tb_scroll_cpi);
    } else {
        pointing_device_set_cpi(tb_cpi);
    }

    return true;
}
