#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "nrf_section.h"


#include "kb_evt.h"

NRF_SECTION_DEF(kb_event_section, KbEventHandler);

static void kb_event_handler(enum kb_event event)
{
    int vars_cnt = NRF_SECTION_ITEM_COUNT(kb_event_section, KbEventHandler);
    for(int i = 0; i < vars_cnt; i++){
        KbEventHandler * p_var_name = NRF_SECTION_ITEM_GET(kb_event_section, KbEventHandler, i);
        (*p_var_name)(event);
    }
}

void execute_event(void)
{
}

void trig_event(enum kb_event event)
{
    kb_event_handler(event);
}