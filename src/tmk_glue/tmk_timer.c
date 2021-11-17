#include <stdint.h>

#include "timer.h"

#include "app_error.h"

#include "app_timer.h"

#include "config.h"

#define TICK_PER_MILLISECOND APP_TIMER_TICKS(1)

void timer_init(void)
{
    //do nothing
    //timer has already inited
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