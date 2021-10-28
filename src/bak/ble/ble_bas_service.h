#pragma once
#include <stdint.h>

#define BATTERY_LEVEL_MEAS_INTERVAL APP_TIMER_TICKS(10000) 

void battery_service_init(void);