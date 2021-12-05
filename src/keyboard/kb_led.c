#include "config.h"

#include <stdint.h>
#include <stdbool.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"

#include "nrf_gpio.h"

#include "led.h"

#include "kb_evt.h"

static void led_on(uint32_t led_pin_mask, uint8_t led_active_state){
    if(led_active_state == LED_ACTIVE_HIGH){
        nrf_gpio_pin_set(led_pin_mask);
    }
    else{
        nrf_gpio_pin_clear(led_pin_mask);
    }
}

static void led_off(uint32_t led_pin_mask, uint8_t led_active_state){
    if(led_active_state == LED_ACTIVE_HIGH){
        nrf_gpio_pin_clear(led_pin_mask);
    }
    else{
        nrf_gpio_pin_set(led_pin_mask);
    }
}

void led_set(uint8_t usb_led)
{
    if(usb_led & 1 << USB_LED_CAPS_LOCK){
        led_on(LED1, LED1_ACTIVE_STATE);
    }
    else{
        led_off(LED1, LED1_ACTIVE_STATE);
    }
}

static void leds_init(void)
{
    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);
    nrf_gpio_cfg_output(LED3);
    led_off(LED1, LED1_ACTIVE_STATE);
    led_off(LED2, LED2_ACTIVE_STATE);
    led_off(LED3, LED3_ACTIVE_STATE);
}

static void led_config_default(uint32_t led_pin_mask, uint8_t led_active_state)
{
    nrf_gpio_pin_pull_t pull_res_config;
    if(led_active_state == LED_ACTIVE_HIGH){
        pull_res_config = NRF_GPIO_PIN_PULLDOWN;
    }
    else{
        pull_res_config = NRF_GPIO_PIN_PULLUP;
    }
    nrf_gpio_cfg(
        led_pin_mask,
        NRF_GPIO_PIN_DIR_INPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        pull_res_config,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE
    );
}

static void leds_deinit(void)
{
    led_config_default(LED1, LED1_ACTIVE_STATE);
    led_config_default(LED2, LED2_ACTIVE_STATE);
    led_config_default(LED3, LED3_ACTIVE_STATE);
}

static bool usbd_is_started = false;
static bool usbd_current_protocol = false;

static void usb_protocol_led_process(void)
{
    if(usbd_current_protocol && !usbd_is_started){
        led_on(LED3, LED3_ACTIVE_STATE);
    }
    else{
        led_off(LED3, LED3_ACTIVE_STATE);
    }
}

static void keyboard_led_event_handler(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event){
    case KB_EVT_INIT:
        leds_init();
        break;
    case KB_EVT_BLE:
        switch(param){
        case KB_BLE_ADV_FAST:
        case KB_BLE_ADV_SLOW:
            led_on(LED2, LED2_ACTIVE_STATE);
            break;
        case KB_BLE_GAP_CONN:
        case KB_BLE_ADV_STOP:
            led_off(LED2, LED2_ACTIVE_STATE);
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
            usbd_current_protocol = false;
            break;
        case SUBEVT_PROTOCOL_USB:
            usbd_current_protocol = true;
            break;
        }
        usb_protocol_led_process();
        break;
    case KB_EVT_SLEEP:
        leds_deinit();
        break;
    default:
        break;
    }
}

KB_EVT_HANDLER(keyboard_led_event_handler);