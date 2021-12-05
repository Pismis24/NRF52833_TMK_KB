#pragma once

#include "board_def.h"

/*Scan Config*/
// The intervals of keyboard task defined by tmk
#define KEYBOARD_TASK_NORMAL_INTERVAL 1 // in ms
#define KEYBOARD_TASK_SLOW_INTERVAL 10 // in ms

/*Matrix*/

/* key matrix size */
#define KEY_ROWS 6
#define KEY_COLS 15

/*matrix pin config*/
/*diodes dir*/
#define DIODES_ROW2COL // if not defined, means the diodes is col to row
/*row pins*/
static const uint8_t row_pins[KEY_ROWS] = {  
    PIN27, PIN26, PIN25, PIN24, PIN23, PIN22};
/*col pins*/
static const uint8_t col_pins[KEY_COLS] = {
    PIN21, PIN20, PIN19, PIN18, PIN17, PIN16, PIN15, PIN14, PIN13, PIN12, PIN11, PIN10, PIN09, PIN08, PIN07};
/*matrix debounce*/
// See kb_matrix.c for debounce method
#define MATRIX_SCAN_VALID_TIMES 5


/*Encoder*/
#define EC11_ENCODER
#define EC11_WITH_SWITCH

#ifdef EC11_ENCODER

/*encoder pin config*/
    #define ENCA PIN05 // pin to read encoder pin A
    #define ENCB PIN04 // pin to read encoder pin B
    #define ENCODER_TN_POS KEY_ROWS,0 // the virtual key map position of turning forward
    #define ENCODER_TN_NEG KEY_ROWS,1 // the virtual key map position of turning backward

    #ifdef EC11_WITH_SWITCH
        #define ENCS PIN06 //pin to read encoder pin switch
        #define ENCODER_SWH KEY_ROWS,2 // the virtual key map position of switch
    #endif
#endif


#if defined(EC11_ENCODER) || defined(LUA)
#define MATRIX_EXTRAKEY_EXIST
#endif


/*Keymap Size*/
// If using things like encoder, 
// to turn its movement to keycodes, 
// it should to be mapped to additional virtual keys outside normal keys
#ifdef MATRIX_EXTRAKEY_EXIST
#define MATRIX_ROWS (KEY_ROWS + 1)
#else
#define MATRIX_ROWS KEY_ROWS
#endif
#define MATRIX_COLS KEY_COLS

/*自动休眠设置*/
#define AUTO_SLEEP_ENABLE
#define AUTO_SLEEP_TIMEOUT 300  // in second
#if defined(AUTO_SLEEP_ENABLE) && !defined(AUTO_SLEEP_TIMEOUT)
    #define AUTO_SLEEP_TIMEOUT 300
#endif
#define AUTO_SLEEP_TIMEOUT_MIL_SECOND (AUTO_SLEEP_TIMEOUT*1000)

/*进入省电状态设置*/
#define POWERSAVE_ENABLE
#define POWERSAVE_TIMEOUT 180 // in second
#if defined(POWERSAVE_ENABLE) && !defined(POWERSAVE_TIMEOUT)
    #define POWERSAVE_TIMEOUT 60
#endif
#define POWERSAVE_TIMEOUT_MIL_SECOND (POWERSAVE_TIMEOUT*1000)

/*背光灯设置*/
#define BACKLIGHT_ENABLE
#define BACKLIGHT_LEVELS 3 // three stages of backlight

/* key combination for command */
#define IS_COMMAND() (keyboard_report->mods == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT)))