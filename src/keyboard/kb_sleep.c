#include <stdint.h>
#include <stdbool.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_soc.h"

#include "app_error.h"

#include "app_timer.h"
#include "nrfx_timer.h"

#include "kb_evt.h"

#include "config.h"

#define FINAL_SLEEP_TIMEOUT APP_TIMER_TICKS(1000)

APP_TIMER_DEF(auto_sleep_timer);
APP_TIMER_DEF(final_sleep_timer);

static void auto_sleep_timeout_handle(void * p_context)
{
    NRF_LOG_INFO("Sleep mode prepare");
    trig_kb_event(KB_EVT_SLEEP); 
}

static void sleep_actual_enter(void * p_context)
{
    //deinit kbd task hardware timer
    //watch dog is feeded by keyboard task so timer need to be stop lastly
    const nrfx_timer_t kbd_task_timer = NRFX_TIMER_INSTANCE(KBD_TASK_TIMER);
    nrfx_timer_uninit(&kbd_task_timer);
    //enter sleep mode
    ret_code_t err_code = sd_power_system_off();
    
}

static void auto_sleep_timer_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_create(&auto_sleep_timer, 
        APP_TIMER_MODE_SINGLE_SHOT, 
        auto_sleep_timeout_handle);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Auto sleep timer init");
    err_code = app_timer_create(&final_sleep_timer,
        APP_TIMER_MODE_SINGLE_SHOT,
        sleep_actual_enter
    );
}

static void auto_sleep_timer_start(void)
{
    ret_code_t err_code;
    err_code = app_timer_start(auto_sleep_timer, 
        APP_TIMER_TICKS(AUTO_SLEEP_TIMEOUT_MIL_SECOND), 
        NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Auto sleep timer start");
}

static void auto_sleep_timer_reset(void)
{
    ret_code_t err_code;
    err_code = app_timer_stop(auto_sleep_timer);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(auto_sleep_timer, 
        APP_TIMER_TICKS(AUTO_SLEEP_TIMEOUT_MIL_SECOND), 
        NULL);
}

static void auto_sleep_event_handle(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event){
    case KB_EVT_INIT:
        auto_sleep_timer_init();
        break;
    case KB_EVT_START:
        auto_sleep_timer_start();
        break;
    case KB_EVT_TMK_HOOK:
        if(param == TMK_HOOK_MATRIX_CHG){
            auto_sleep_timer_reset();
        }
        break;
    case KB_EVT_SLEEP:
        err_code = app_timer_start(final_sleep_timer, FINAL_SLEEP_TIMEOUT, NULL);
        APP_ERROR_CHECK(err_code);
    }
}

KB_EVT_HANDLER(auto_sleep_event_handle);