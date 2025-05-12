/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

// #define VIAL_UNLOCK_COMBO_ROWS { 0, 4 }
// #define VIAL_UNLOCK_COMBO_COLS { 0, 0 }

#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_MODS_ENABLE
#define SPLIT_WPM_ENABLE

#define SPLIT_POINTING_ENABLE
#define POINTING_DEVICE_RIGHT
#define POINTING_DEVICE_INVERT_Y
#define ROTATIONAL_TRANSFORM_ANGLE 115

#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP22
#define SPI_MISO_PIN GP20
#define SPI_MOSI_PIN GP23
#define POINTING_DEVICE_CS_PIN GP16

#ifdef RGB_MATRIX_ENABLE
#    define RGB_MATRIX_LED_COUNT 59
#endif

// uncomment before flashing the right side
// #define MASTER_RIGHT
