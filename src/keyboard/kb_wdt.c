#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "config.h"

#include "app_error.h"

#include "nrfx_wdt.h"

#include "hook.h"

#include "kb_evt.h"

#define WDT_RELOAD_TIME (KEYBOARD_TASK_INTERVAL * 100) // 100倍keyboard task运行周期 (100ms)

// watchdog timer expired handle
static void kb_watchdog_handler(void)
{
   APP_ERROR_HANDLER(0xFEEDF00D);
}

static nrfx_wdt_channel_id kb_wdt_channel_id;

static void watchdog_init(void)
{
    ret_code_t err_code;
    static nrfx_wdt_config_t kb_wdt_config = NRFX_WDT_DEAFULT_CONFIG;
    kb_wdt_config.reload_value = WDT_RELOAD_TIME;

    err_code = nrfx_wdt_init(&kb_wdt_config, kb_watchdog_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_wdt_channel_alloc(&kb_wdt_channel_id);
    APP_ERROR_CHECK(err_code);
}

static void kb_watchdog_evt_handler(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event){
    case KB_EVT_INIT:
        watchdog_init();
        NRF_LOG_INFO("keyboard wdt init");
        break;
    case KB_EVT_START:
        nrfx_wdt_enable();
        NRF_LOG_INFO("keyboard wdt start");
        break;
    case KB_EVT_TMK_HOOK:
        if(param == TMK_HOOK_LOOP){
            nrfx_wdt_channel_feed(kb_wdt_channel_id);
        }
    default:
        break;
    }
}

KB_EVT_HANDLER(kb_watchdog_evt_handler);