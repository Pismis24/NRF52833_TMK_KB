#include <stdint.h>

#include "timer.h"
#include "keyboard.h"

#include "app_error.h"

#include "app_timer.h"
#include "nrfx_timer.h"

#include "config.h"
#include "kb_evt.h"


//使用32.768k晶振，因此1s计数32768
#define TICK_PER_MILLISECOND (32768 / (APP_TIMER_CONFIG_RTC_FREQUENCY + 1) / 1000)

//主扫描计时器，使用硬件timer
const nrfx_timer_t kbd_task_timer = NRFX_TIMER_INSTANCE(KBD_TASK_TIMER);

static void kbd_task_timeout_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch(event_type){
        case NRF_TIMER_EVENT_COMPARE0:
            keyboard_task();
            break;
        default:
        break;
    }
}

void timer_init(void)
{
    ret_code_t err_code;
    // init app_timer
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    // init keyboard task timer
    nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG;
    err_code = nrfx_timer_init(&kbd_task_timer, &timer_cfg, kbd_task_timeout_handler);
    APP_ERROR_CHECK(err_code);
    uint32_t time_ticks = nrfx_timer_ms_to_ticks(&kbd_task_timer, KEYBOARD_TASK_INTERVAL);
    nrfx_timer_extended_compare(&kbd_task_timer, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}

void timer_clear()
{
    // do nothing
}

inline uint16_t timer_read()
{
    return (uint16_t)(timer_read32() % 0xFFFF);
}

inline uint32_t timer_read32()
{
    uint32_t time;
    time = app_timer_cnt_get();
    return time / TICK_PER_MILLISECOND;
}

inline uint16_t timer_elapsed(uint16_t last)
{
    return TIMER_DIFF_16(timer_read(), last);
}

inline uint32_t timer_elapsed32(uint32_t last)
{
    uint32_t time = app_timer_cnt_get();
    uint32_t elapsed = app_timer_cnt_diff_compute(time, last * TICK_PER_MILLISECOND);
    return elapsed / TICK_PER_MILLISECOND;
}