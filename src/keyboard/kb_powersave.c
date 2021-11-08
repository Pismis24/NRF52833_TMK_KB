#include <stdint.h>
#include <stdbool.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"

#include "app_timer.h"

#include "kb_evt.h"

#include "config.h"

APP_TIMER_DEF(powersave_timer);

static bool in_powersave = false;

static void powersave_timeout_handle(void * p_context)
{
    trig_kb_event_param(KB_EVT_POWERSAVE, KB_POWERSAVE_ENTER);
    NRF_LOG_INFO("powersave mode enter");
    in_powersave = true;
}

static void powersave_timer_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_create(&powersave_timer, 
        APP_TIMER_MODE_SINGLE_SHOT, 
        powersave_timeout_handle);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Powersave timer init");
}

static void powersave_timer_start(void)
{
    ret_code_t err_code;
    err_code = app_timer_start(powersave_timer, 
        APP_TIMER_TICKS(POWERSAVE_TIMEOUT_MIL_SECOND), 
        NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Powersave timer start");
}

static void powersave_timer_reset(void)
{
    ret_code_t err_code;
    err_code = app_timer_stop(powersave_timer);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(powersave_timer, 
        APP_TIMER_TICKS(POWERSAVE_TIMEOUT_MIL_SECOND), 
        NULL);
    if(in_powersave){
        trig_kb_event_param(KB_EVT_POWERSAVE, KB_POWERSAVE_EXIT);
        powersave_timer_start();
        NRF_LOG_INFO("powersave mode exit");
        in_powersave = false;
    }
}

static void powersave_event_handle(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event){
    case KB_EVT_INIT:
        powersave_timer_init();
        break;
    case KB_EVT_START:
        powersave_timer_start();
        break;
    case KB_EVT_TMK_HOOK:
        if(param == TMK_HOOK_MATRIX_CHG){
            powersave_timer_reset();
        }
        break;
    case KB_EVT_SLEEP:
        err_code = app_timer_stop(powersave_timer);
        APP_ERROR_CHECK(err_code);
    break;
    }
    
}

KB_EVT_HANDLER(powersave_event_handle);