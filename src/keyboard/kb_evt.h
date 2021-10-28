#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "nrf_section.h"

typedef enum kb_event_lst {
    KB_EVT_INIT,
    KB_EVT_START,
    KB_EVT_SLEEP,
    KB_EVT_USB
} kb_event_type_t;

typedef void (*KbEvtHandler)(kb_event_type_t, void*);

void execute_kb_event(void);
void trig_kb_event(kb_event_type_t event, void* p_arg);
void trig_kb_event_param(kb_event_type_t event, uint8_t arg);

#define KB_EVT_HANDLER(_func) NRF_SECTION_ITEM_REGISTER(kbd_event, const KbEvtHandler _pt_##_func) = &_func