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

#include "kb_evt.h"

/*
    大小写灯直接控制电平高低
    双色灯用类似pwm的方法控制，轮流亮半周，通过控制这半周是否应点亮来控制闪烁
*/

#define BNR_DRIVE_INTERVAL APP_TIMER_TICKS(5) // 5ms 半周，100hz
#define BNR_BLINK_INTERVAL_FAST APP_TIMER_TICKS(200)
#define BNR_BLINK_INTERVAL_SLOW APP_TIMER_TICKS(1000) // 闪烁周期1000ms（1s）

enum bnr_drive_turn
{
    BNR_TURN_R,
    BNR_TURN_B
};


enum bnr_light_color 
{
    BNR_COLOR_RED,
    BNR_COLOR_BLUE,
    BNR_COLOR_BOTH
};

enum bnr_blink_state
{
    BNR_BLINK_FAST,
    BNR_BLINK_SLOW
};

//Dual color light drive timer
APP_TIMER_DEF(bnr_drive_timer);
//Dual color light blink timer
APP_TIMER_DEF(bnr_blink_timer);

static uint8_t bnr_drive_turn;
static uint8_t bnr_blink_turn;
static uint32_t bnr_blink_interval;
static bool red_active;
static bool blue_active;
static bool red_blink;
static bool blue_blink;
static bool led_deinit = false;


static void bnr_drive_timeout_handler(void * p_context)
{
    if(led_deinit){ return; }
    switch(bnr_drive_turn){
        case BNR_TURN_R:
            nrf_gpio_pin_clear(BNR_LED_B);
            if(red_active){
                nrf_gpio_pin_set(BNR_LED_R);
            }
            bnr_drive_turn = BNR_TURN_B;
        break;
        case BNR_TURN_B:
            nrf_gpio_pin_clear(BNR_LED_R);
            if(blue_active){
                nrf_gpio_pin_set(BNR_LED_B);
            }
            bnr_drive_turn = BNR_TURN_R;
        break;
    }
    ret_code_t err_code;
    err_code = app_timer_start(bnr_drive_timer, BNR_DRIVE_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

static void bnr_blink_timeout_handler(void * p_context)
{
    if(led_deinit){ return; }
    switch(bnr_blink_turn){
        case BNR_TURN_R:
        blue_active = false;
        if(red_blink){
            red_active = true;
        }
        bnr_blink_turn = BNR_TURN_B;
        break;
        case BNR_TURN_B:
        red_active = false;
        if(blue_blink){
            blue_active = true;
        }
        bnr_blink_turn = BNR_TURN_R;
        break;
    }
    ret_code_t err_code;
    err_code = app_timer_start(bnr_blink_timer, bnr_blink_interval, NULL);
    APP_ERROR_CHECK(err_code);
}


//Init keyboard led
static void kb_led_init(void)
{
    //Capslock led
    nrf_gpio_cfg_output(CAPS_LED);
    //Dual color light
    bnr_drive_turn = BNR_TURN_B;
    bnr_blink_turn = BNR_TURN_B;
    bnr_blink_interval = BNR_BLINK_INTERVAL_SLOW;
    red_active = false;
    blue_active = false;
    red_blink = false;
    blue_active = false;
    //Config output with pulldown resister
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
    // init timers
    ret_code_t err_code;
    err_code = app_timer_create(
        &bnr_drive_timer,
        APP_TIMER_MODE_SINGLE_SHOT,
        bnr_drive_timeout_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(
        &bnr_blink_timer,
        APP_TIMER_MODE_SINGLE_SHOT,
        bnr_blink_timeout_handler);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Keyboard led inited");
}

//deinit keyboard led
static void kb_led_deinit(void)
{
    led_deinit = true; // if the flag is set true, timer will not start again after expired
    nrf_gpio_cfg_default(CAPS_LED);
    nrf_gpio_pin_clear(BNR_LED_B);
    nrf_gpio_pin_clear(BNR_LED_R);
}


void led_set(uint8_t usb_led)
{
    if(usb_led & 1 << USB_LED_CAPS_LOCK){
#ifdef CAPS_LED_HIGH_ACT
        nrf_gpio_pin_set(CAPS_LED);
#else
        nrf_gpio_pin_clear(CAPS_LED);
#endif
    }
    else {
#ifdef CAPS_LED_HIGH_ACT
        nrf_gpio_pin_clear(CAPS_LED);
#else
        nrf_gpio_pin_set(CAPS_LED);
#endif
    }
}


static void bnr_led_start(void)
{
    ret_code_t err_code;
    err_code = app_timer_start(bnr_drive_timer, BNR_DRIVE_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(bnr_blink_timer, bnr_blink_interval, NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Keyboard led started");
}

static void bnr_led_color_set(uint8_t color)
{
    switch(color){
        case BNR_COLOR_BLUE:
            blue_blink = true;
        break;
        case BNR_COLOR_RED:
            red_blink = true;
        break;
        case BNR_COLOR_BOTH:
            red_blink = true;
            blue_blink = true;
        break;
    }
}

static void bnr_led_color_clear(uint8_t color)
{
    switch(color){
        case BNR_COLOR_BLUE:
            blue_blink = false;
        break;
        case BNR_COLOR_RED:
            red_blink = false;
        break;
        case BNR_COLOR_BOTH:
            red_blink = false;
            blue_blink = false;
        break;
    }
}



static void bnr_led_freq_set(uint8_t freq)
{
    switch(freq){
        case BNR_BLINK_FAST:
            bnr_blink_interval = BNR_BLINK_INTERVAL_FAST;
        break;
        case BNR_BLINK_SLOW:
            bnr_blink_interval = BNR_BLINK_INTERVAL_SLOW;
        break;
    }
}

static bool usbd_is_started = false;
static bool current_protocol_is_usb;

static void usb_protocol_led_process(void)
{
    if(current_protocol_is_usb && !usbd_is_started){
        bnr_led_color_set(BNR_COLOR_RED);
        bnr_led_freq_set(BNR_BLINK_FAST);
    }
    else{
        bnr_led_color_clear(BNR_COLOR_RED);
    }
}

static void keyboard_led_event_handler(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event){
        case KB_EVT_INIT:
            kb_led_init();
            bnr_led_start();
        break;
        case KB_EVT_BLE:
            switch(param){
                case KB_BLE_ADV_FAST:
                    bnr_led_freq_set(BNR_BLINK_FAST);
                    bnr_led_color_set(BNR_COLOR_BLUE);
                break;
                case KB_BLE_ADV_SLOW:
                    bnr_led_freq_set(BNR_BLINK_SLOW);
                    bnr_led_color_set(BNR_COLOR_BLUE);
                break;
                case KB_BLE_GAP_CONN:
                case KB_BLE_ADV_STOP:
                    bnr_led_color_clear(BNR_COLOR_BLUE);
                break;
            }
        break;
        case KB_EVT_USB:
            switch(param){
                case KB_USB_START:
                    usbd_is_started = true;
                break;
                case KB_USB_STOP:
                    usbd_is_started = false;
                break;
            }
            usb_protocol_led_process();
        break;
        case KB_EVT_PROTOCOL_SWITCH:
            switch(param){
                case SUBEVT_PROTOCOL_BLE:
                    current_protocol_is_usb = false;
                break;
                case SUBEVT_PROTOCOL_USB:
                    current_protocol_is_usb = true;
                break;
            }
            usb_protocol_led_process();
        break;
        case KB_EVT_SLEEP:
            kb_led_deinit();
        break;
        default:
        break;
    }
}

KB_EVT_HANDLER(keyboard_led_event_handler);