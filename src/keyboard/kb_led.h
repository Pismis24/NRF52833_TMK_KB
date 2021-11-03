#pragma once

#include <stdint.h>
#include "app_timer.h"

#define BNR_LED_DRIVE_INTERVAL APP_TIMER_TICKS(5) // 5ms 半周 10ms周期 100hz
#define BNR_LED_BLINK_FAST_INTERVAL APP_TIMER_TICKS(200) //闪烁周期
#define BNR_LED_BLINK_SLOW_INTERVAL APP_TIMER_TICKS(1000)

enum bnr_light_color
{
    BNR_COLOR_NO,
    BNR_COLOR_RED,
    BNR_COLOR_BLUE,
    BNR_COLOR_BOTH
};

enum bnr_blink_color
{
    BNR_BLINK_RED,
    BNR_BLINK_BLUE,
    BNR_BLINK_BOTH
};

enum bnr_blink_freq
{
    BNR_BLINK_FAST,
    BNR_BLINK_SLOW
};


//设置灯颜色 
void bnr_led_set(enum bnr_light_color color);
//闪烁
void bnr_led_blink_set(enum bnr_blink_color color, enum bnr_blink_freq freq);
void bnr_led_blink_stop(void);