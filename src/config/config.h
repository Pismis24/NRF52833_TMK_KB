#pragma once

#include "board_def.h"

/*Scan Config*/
#define KEYBOARD_TASK_INTERVAL 1 // in ms

/*Matrix*/

/* key matrix size */
#define KEY_ROWS 6
#define KEY_COLS 15

/*matrix pin config*/
/*diodes dir*/
#define DIODES_ROW2COL
/*row pins*/
static const uint8_t row_pins[KEY_ROWS] = {  
    PIN27, PIN26, PIN25, PIN24, PIN23, PIN22};
/*col pins*/
static const uint8_t col_pins[KEY_COLS] = {
    PIN21, PIN20, PIN19, PIN18, PIN17, PIN16, PIN15, PIN14, PIN13, PIN12, PIN11, PIN10, PIN09, PIN08, PIN07};
/*matrix debounce*/
#define MATRIX_SCAN_VALID_TIMES 10


/*Encoder*/
#define EC11_ENCODER
#define EC11_WITH_SWITCH

#ifdef EC11_ENCODER

/*encoder pin config*/
    #define ENCA PIN05
    #define ENCB PIN04
    #define ENCODER_TN_POS KEY_ROWS,0
    #define ENCODER_TN_NEG KEY_ROWS,1

    #ifdef EC11_WITH_SWITCH
        #define ENCS PIN06
        #define ENCODER_SWH KEY_ROWS,2
    #endif
#endif


#if defined(EC11_ENCODER) || defined(LUA)
#define MATRIX_EXTRAKEY_EXIST
#endif


/*Keymap Size*/
#ifdef MATRIX_EXTRAKEY_EXIST //按键矩阵之外的按键单列一行
#define MATRIX_ROWS (KEY_ROWS + 1)
#else
#define MATRIX_ROWS KEY_ROWS
#endif
#define MATRIX_COLS KEY_COLS

/*自动休眠设置*/
#define AUTO_SLEEP_ENABLE
#define AUTO_SLEEP_TIMEOUT 600  // in second
#if defined(AUTO_SLEEP_ENABLE) && !defined(AUTO_SLEEP_TIMEOUT)
    #define AUTO_SLEEP_TIMEOUT 300
#endif
#define AUTO_SLEEP_TIMEOUT_MIL_SECOND (AUTO_SLEEP_TIMEOUT*1000)

/*进入省电状态设置*/
#define POWERSAVE_ENABLE
#define POWERSAVE_TIMEOUT 60 // in second
#if defined(POWERSAVE_ENABLE) && !defined(POWERSAVE_TIMEOUT)
    #define POWERSAVE_TIMEOUT 60
#endif
#define POWERSAVE_TIMEOUT_MIL_SECOND (POWERSAVE_TIMEOUT*1000)

/*大小写灯*/
#define CAPS_LED PIN01
#define CAPS_LED_ACT 1 //高电平点亮

//两脚双色LED灯
//蓝灯引脚（用于指示蓝牙状态）
#define BNR_LED_B PIN02
//红灯引脚（用于指示USB状态）
#define BNR_LED_R PIN03

/*背光灯设置*/
#define BACKLIGHT_ENABLE
#define BACKLIGHT_LEVELS 3

/* key combination for command */
#define IS_COMMAND() (keyboard_report->mods == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT)))