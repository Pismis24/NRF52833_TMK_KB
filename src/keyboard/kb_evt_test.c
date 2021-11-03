#include "kb_evt.h"
#include <stdint.h>
#include <stdbool.h>

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

APP_TIMER_DEF(test_timer);

static void test_evt_trigger(void * p_context)
{
    trig_kb_event(KB_EVT_USB, NULL);
}

static void kb_test_event_handle(kb_event_type_t event, void * p_arg)
{
    uint8_t num = (uint32_t)p_arg;
    switch(event)
    {
        case KB_EVT_INIT:
            NRF_LOG_INFO("kb init");
            app_timer_create(&test_timer, APP_TIMER_MODE_REPEATED, test_evt_trigger);
            break;
        case KB_EVT_START:
            NRF_LOG_INFO("kb start %d", num);
            app_timer_start(test_timer, APP_TIMER_TICKS(3000), NULL);
            break;
        case KB_EVT_USB:
            NRF_LOG_INFO("test");
            break;
        default:
            break;
    }
}

static void kb_test_event_handle2(kb_event_type_t event, void * p_arg)
{
    uint8_t num = (uint32_t)p_arg;
    switch(event)
    {
        case KB_EVT_INIT:
            NRF_LOG_INFO("kb init 2");
            break;
        case KB_EVT_START:
            NRF_LOG_INFO("kb 2 start %d", num);
            break;
        default:
            break;
    }
}

KB_EVT_HANDLER(kb_test_event_handle);
KB_EVT_HANDLER(kb_test_event_handle2);