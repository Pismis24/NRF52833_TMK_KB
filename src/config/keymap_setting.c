/*
Copyright (C) 2018,2019 Jim Jiang <jim@lotlab.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "key_layout.h"
#include "config.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "ble.h"
#include "ble_service.h"

const uint8_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    KEYMAP(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_FN1,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_PSCR,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_DEL,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, KC_UP,
        KC_LCTL, KC_LGUI, KC_LALT,                   KC_SPC,                             KC_RALT, KC_RCTL, KC_FN0,  KC_LEFT, KC_DOWN, KC_RIGHT,
        KC_WH_D, KC_WH_U, KC_MUTE
    ),
    KEYMAP(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_FN2,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_FN4,  KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_FN3,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS, KC_PGUP,
        KC_TRNS, KC_TRNS, KC_TRNS,                   KC_TRNS,                            KC_TRNS, KC_TRNS, KC_FN0,  KC_HOME, KC_PGDN, KC_END,
        KC_VOLU, KC_VOLD, KC_MUTE
        )
};

enum action_func_nme{
    DEL_BOND,
    BLE_CONN_TOGGLE,
};

const action_t fn_actions[] = {
    ACTION_LAYER_MOMENTARY(1),
    ACTION_BACKLIGHT_STEP(),
    ACTION_BACKLIGHT_TOGGLE(),
    ACTION_FUNCTION(DEL_BOND),
    ACTION_FUNCTION(BLE_CONN_TOGGLE)
};


void action_function(keyrecord_t *record, uint8_t id, uint8_t opt)
{
    static int32_t del_bond_time;
    switch(id){
    case DEL_BOND:
        if(record->event.pressed){
            NRF_LOG_INFO("delete bond pressed");
            del_bond_time = record->event.time;
        }
        else{
            int32_t time_sep = record->event.time - del_bond_time;
            if(time_sep < 0){
                time_sep = 0xffff - del_bond_time + record->event.time;
            }
            NRF_LOG_INFO("time sep %d time saved %d", time_sep, del_bond_time);
            if(record->event.time - del_bond_time >= 3000){
                NRF_LOG_INFO("delete bond");
                delete_bonds();
            }
        }
        break;
    case BLE_CONN_TOGGLE:
        if(record->event.pressed){
            ble_conn_toggle();
        }
        break;
    }
}