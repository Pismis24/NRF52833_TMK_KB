#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "nrf_section.h"

enum kb_event {
    KB_EVT_INIT,
    KB_EVT_START,
    KB_EVT_SLEEP
};

void trig_event(enum kb_event event);

void execute_event(void);

typedef void (*KbEventHandler)(uint8_t);

#define KB_EVENT_HANDLER(_func) NRF_SECTION_ITEM_REGISTER(kb_event_section, const KbEventHandler _pt_##_func) = &_func