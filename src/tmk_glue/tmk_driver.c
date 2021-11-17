//Std Headers
#include <stdint.h>
#include <string.h>
//nRF LOG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//tmk
#include "keyboard.h"
//nrf
#include "app_timer.h"
//config
#include "config.h"
//event
#include "kb_evt.h"

static bool stop_flag = false;
static uint32_t task_interval_ticks;

APP_TIMER_DEF(kbd_task_timer);

static void keyboard_task_handler(void * p_context)
{
    keyboard_task();
    ret_code_t err_code;
    err_code = app_timer_start(kbd_task_timer, task_interval_ticks, NULL);
    APP_ERROR_CHECK(err_code);
}

void keyboard_task_start(void)
{
    ret_code_t err_code;
    err_code = app_timer_create(&kbd_task_timer, APP_TIMER_MODE_SINGLE_SHOT, keyboard_task_handler);
    APP_ERROR_CHECK(err_code);
    task_interval_ticks = APP_TIMER_TICKS(KEYBOARD_TASK_NORMAL_INTERVAL);
    err_code = app_timer_start(kbd_task_timer, task_interval_ticks, NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("keyboard task start");
}

static void kb_evt_kbd_driver_handler(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event) {
        case KB_EVT_POWERSAVE:
            switch(param){
                case KB_POWERSAVE_ENTER:
                    task_interval_ticks = APP_TIMER_TICKS(KEYBOARD_TASK_SLOW_INTERVAL);
                    NRF_LOG_INFO("scan interval is %d ms", KEYBOARD_TASK_SLOW_INTERVAL);
                break;
                case KB_POWERSAVE_EXIT:
                    task_interval_ticks = APP_TIMER_TICKS(KEYBOARD_TASK_NORMAL_INTERVAL);
                    NRF_LOG_INFO("scan interval is %d ms", KEYBOARD_TASK_NORMAL_INTERVAL);
                break;
            }
        break;
    }
}

KB_EVT_HANDLER(kb_evt_kbd_driver_handler);