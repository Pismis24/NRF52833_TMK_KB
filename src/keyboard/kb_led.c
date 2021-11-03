#include "config.h"

#include <stdint.h>
#include <stdbool.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led.h"

#include "kb_led.h"

#include "kb_evt.h"

//大小写灯
static void led_active(uint32_t pin, bool active_state)
{
    if(active_state){
        nrf_gpio_pin_set(pin);
    }
    else{
        nrf_gpio_pin_clear(pin);
    }
}

static void led_inactive(uint32_t pin, bool active_state)
{
    if(active_state){
        nrf_gpio_pin_clear(pin);
    }
    else{
        nrf_gpio_pin_set(pin);
    }
}


void led_set(uint8_t usb_led)
{    
    NRF_LOG_INFO("led set");
    if(usb_led & 1 << USB_LED_CAPS_LOCK){
        led_active(CAPS_LED, CAPS_LED_ACT);
    }
    else{
        led_inactive(CAPS_LED, CAPS_LED_ACT);
    }
}

//两脚红蓝双色LED

//开启定时器轮流控制灯两脚的电平，保证两个脚不会同时置高
//根据此时需要判定该脚是否应该置高

static bool light_turn; //0:蓝色灯半周；1：红色灯半周
static bool blink_turn; //闪烁时标记当前为哪个颜色的半周
static bool blink_timer_state; //当前是否在闪烁
static bool red_active; //红色灯在当前半周是否应该点亮
static bool blue_active;//蓝色灯在当前半周是否应该点亮
static bool red_blink; //红灯当前是否闪烁
static bool blue_blink;//蓝灯当前是否闪烁
static bool red_state; //闪烁前红灯状态
static bool blue_state; //闪烁前蓝灯状态

APP_TIMER_DEF(bnr_led_timer);
APP_TIMER_DEF(bnr_led_blink_timer);

static void bnr_led_timer_timeout_handler(void * p_context)
{
    if(light_turn){
        nrf_gpio_pin_clear(BNR_LED_R);
        if(blue_active){
            nrf_gpio_pin_set(BNR_LED_B);
        }
    }
    else{
        nrf_gpio_pin_clear(BNR_LED_B);
        if(red_active){
            nrf_gpio_pin_set(BNR_LED_R);
        }
    }
    //下个半周
    light_turn = !light_turn;
}

static void bnr_led_blink_timer_timeout_handler(void * p_context)
{
    if(blink_turn){
        red_active = false;
        if(blue_blink){
            blue_active = true;
        }
    }
    else{
        blue_active = false;
        if(red_blink){
            red_active = true;
        }
    }
    blink_turn = !blink_turn;
}


static void bnr_led_init(void)
{
    nrf_gpio_cfg(
        BNR_LED_B,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_PULLDOWN,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE
    );
    nrf_gpio_cfg(
        BNR_LED_R,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_PULLDOWN,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE
    );
    light_turn = false;
    blink_turn = false;
    blink_timer_state = false;
    red_active = false;
    blue_active = false;
    red_blink = false;
    blue_blink = false;

    ret_code_t err_code;
    err_code = app_timer_create(
        &bnr_led_timer, 
        APP_TIMER_MODE_REPEATED, 
        bnr_led_timer_timeout_handler
    );
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(bnr_led_timer, BNR_LED_DRIVE_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(
        &bnr_led_blink_timer,
        APP_TIMER_MODE_REPEATED,
        bnr_led_blink_timer_timeout_handler
    );
    APP_ERROR_CHECK(err_code);
}

static void bnr_led_deinit(void)
{
    ret_code_t err_code;
    if(blink_timer_state){
        err_code = app_timer_stop(bnr_led_blink_timer);
        APP_ERROR_CHECK(err_code);
        blink_timer_state = false;
    }
    err_code = app_timer_stop(bnr_led_timer);
    APP_ERROR_CHECK(err_code);
    nrf_gpio_cfg_default(BNR_LED_B);
    nrf_gpio_cfg_default(BNR_LED_R);
}

void bnr_led_set(enum bnr_light_color color)
{
    switch(color){
        case BNR_COLOR_NO:
            red_active = false;
            blue_active = false;
            break;
        case BNR_COLOR_RED:
            red_active = true;
            blue_active = false;
            break;
        case BNR_COLOR_BLUE:
            red_active = false;
            blue_active = true;
            break;
        case BNR_COLOR_BOTH:
            red_active = true;
            blue_active = true;
            break;
        default:
            break;
    }
}

void bnr_led_blink_set(enum bnr_blink_color color, enum bnr_blink_freq freq)
{
    ret_code_t err_code;
    //若原来处于闪烁状态则停止并重新设置闪烁间隔
    if(blink_timer_state){
        err_code = app_timer_stop(bnr_led_blink_timer);
        APP_ERROR_HANDLER(err_code);
        blink_timer_state = false;
        red_active = red_state;
        blue_active = blue_state;
    }
    //保存原来的点亮状态
    red_state = red_active;
    blue_state = blue_active;
    //重设闪烁频率
    switch(freq){
        case BNR_BLINK_FAST:
            err_code = app_timer_start(bnr_led_blink_timer, BNR_LED_BLINK_FAST_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
            blink_timer_state = true;
            break;
        case BNR_BLINK_SLOW:
            err_code = app_timer_start(bnr_led_blink_timer, BNR_LED_BLINK_SLOW_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
            blink_timer_state = true;
            break;
        default:
            break;
    }
    //设置颜色
    switch(color){
        case BNR_BLINK_RED:
            red_blink = true;
            blue_blink = false;
            break;
        case BNR_BLINK_BLUE:
            red_blink = false;
            blue_blink = true;
            break;
        case BNR_BLINK_BOTH:
            red_blink = true;
            blue_blink = true;
            break;
        default:
            red_blink = false;
            blue_blink = false;
            break;
    }
}


void bnr_led_blink_stop(void)
{
    ret_code_t err_code;
    if(blink_timer_state){
        err_code = app_timer_stop(bnr_led_blink_timer);
        APP_ERROR_HANDLER(err_code);
        blink_timer_state = false;
        //恢复原点亮状态
        red_active = red_state;
        blue_active = blue_state;
        red_blink = false;
        blue_blink = false;
    }
}


static void keyboard_led_init(void)
{
    NRF_LOG_INFO("keyboard led init");
    nrf_gpio_cfg_output(CAPS_LED);
    bnr_led_init();
}

static void keyboard_led_deinit(void)
{
    nrf_gpio_cfg_default(CAPS_LED);
    bnr_led_deinit();
}

static void keyboard_led_event_handler(kb_event_type_t event, void * p_arg)
{
    switch(event)
    {
        case KB_EVT_INIT:
            keyboard_led_init();
            break;
        case KB_EVT_SLEEP:
            keyboard_led_deinit();
            break;
        default:
            break;
    }
}

KB_EVT_HANDLER(keyboard_led_event_handler);