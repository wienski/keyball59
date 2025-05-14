/* Copyright 2025 Yegor Wienski
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

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT(
    KC_GRV, KC_1,    KC_2,          KC_3,    KC_4,    KC_5,
                      KC_6,          KC_7,    KC_8,    KC_9,     KC_0,      KC_RBRC,
    KC_LBRC,  KC_Q,    KC_W,          KC_E,    KC_R,    KC_T,
                      KC_Y,          KC_U,    KC_I,    KC_O,     KC_P,      KC_QUOT,
    KC_ESC,  KC_A,    KC_S,          KC_D,    KC_F,    KC_G,
                      KC_H,          KC_J,    KC_K,    KC_L,     KC_SCLN,   KC_BSPC,
    KC_TAB,  KC_Z,    KC_X,          KC_C,    KC_V,    KC_B,     KC_MUTE,
             QK_BOOT, KC_N,          KC_M,    KC_COMM, KC_DOT,   KC_SLSH,   KC_ENT,
    KC_LSFT, KC_LALT, OSM(MOD_RALT), KC_LGUI, KC_LCTL, KC_SPC,   OSL(1),
             OSL(2),  KC_RSFT,                         CK_TURBO,  CK_LOCK
  ),
  [1] = LAYOUT(
    KC_F1,   KC_F2,   KC_F3,         KC_F4,   KC_F5,   KC_F6,
                      KC_F7,         KC_F8,   KC_F9,   KC_F10,   KC_F11,    KC_F12,
    KC_TILD, KC_EXLM, KC_AT,         KC_HASH, KC_DLR,  KC_PERC,
                      KC_CIRC,       KC_AMPR, KC_ASTR, KC_LPRN,  KC_RPRN,   KC_DQUO,
    KC_ESC,  KC_LCBR, KC_RCBR,       KC_MINS, KC_UNDS, KC_PLUS,
                      KC_PIPE,       KC_EQL,  KC_LT,   KC_GT,    KC_QUES,   KC_BSPC,
    KC_TAB,  KC_LBRC, KC_RBRC,       KC_LPRN, KC_RPRN, KC_NO,    KC_TRNS,
             KC_NO,   KC_BSLS,       KC_NO,   KC_COMM, KC_DOT,   KC_SLSH,   KC_ENT,
    KC_LSFT, KC_LALT, OSM(MOD_RALT), KC_LGUI, KC_LCTL, KC_SPC,   TO(1),
             OSL(2),  KC_RSFT,                         CK_TURBO,    CK_LOCK
  ),
  [2] = LAYOUT(
    KC_GRV,    KC_1,    KC_2,          KC_3,    KC_4,    KC_5,
                      KC_F1,         KC_F2,   KC_F3,   KC_F4,    KC_F5,     KC_F6,
    KC_NO,  KC_Q,    KC_W,          KC_PSCR, KC_SCRL, KC_BRK,
                      KC_F7,         KC_F8,   KC_F9,   KC_F10,   KC_F11,    KC_F12,
    KC_ESC,  KC_HOME, KC_PGDN,       KC_PGUP, KC_END,  KC_INS,
                      KC_NO,         KC_BTN1, KC_BTN2, KC_BTN3,  TB_SCROLL, KC_BSPC,
    KC_TAB,  KC_Z,    KC_X,          KC_C,    KC_V,    KC_DEL,   KC_TRNS,
             CK_SAVE, KC_NO,         KC_LEFT, KC_DOWN, KC_UP,    KC_RGHT,   KC_ENT,
    KC_LSFT, KC_LALT, OSM(MOD_RALT), KC_LGUI, KC_LCTL, KC_SPC,   TO(0),
             TO(2),   KC_RSFT,                         CK_TURBO, CK_LOCK
  )
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
  [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(TB_CPI_DOWN, TB_CPI_UP)  },
  [1] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS),  ENCODER_CCW_CW(KC_TRNS, KC_TRNS)  },
  [2] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS),  ENCODER_CCW_CW(KC_TRNS, KC_TRNS)  }
};
#endif
