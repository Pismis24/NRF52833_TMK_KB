#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "app_error.h"

#include "nrf_queue.h"
#include "nrf_section.h"

#include "kb_evt.h"

#define MAX_KB_EVT_COUNT 30

typedef struct event_queue_item
{
    kb_event_type_t event;
    void * p_arg;
} event_queue_item_t;


//定义键盘事件Section
NRF_SECTION_DEF(kbd_event, KbEvtHandler);
//定义键盘事件buffer queue
NRF_QUEUE_DEF(event_queue_item_t, kb_event_queue, MAX_KB_EVT_COUNT, NRF_QUEUE_MODE_NO_OVERFLOW);

void kb_event_queue_init(void)
{
    nrf_queue_reset(&kb_event_queue);
}

static void kb_event_handler(kb_event_type_t event, void* p_arg)
{
    uint8_t kb_evt_num = NRF_SECTION_ITEM_COUNT(kbd_event, KbEvtHandler);
    for (uint8_t i=0; i<kb_evt_num; i++){
        KbEvtHandler* p_func_name = NRF_SECTION_ITEM_GET(kbd_event, KbEvtHandler, i);
        (*p_func_name)(event, p_arg);
    }
}

static void trig_event(kb_event_type_t event, void* p_arg){
    ret_code_t err_code;
    event_queue_item_t event_item = {0};
    switch(event){
        case KB_EVT_INIT:
        case KB_EVT_START:      
            kb_event_handler(event, p_arg);//以上事件需要立刻处理
            break;
        default:
            //其他事件先入队，在空闲后处理
            event_item.event = event;
            event_item.p_arg = p_arg;
            err_code = nrf_queue_push(&kb_event_queue, &event_item);
            APP_ERROR_CHECK(err_code);
            break;
    }
}

void trig_kb_event(kb_event_type_t event)
{
    trig_event(event, NULL);
}

void trig_kb_event_param(kb_event_type_t event, uint8_t arg)
{
    trig_event(event, (void*)(uint32_t)arg);
}

void execute_kb_event(void)
{
    ret_code_t err_code;
    event_queue_item_t event_item = {0};
    while(!nrf_queue_is_empty(&kb_event_queue)){
        
        err_code = nrf_queue_pop(&kb_event_queue, &event_item);
        APP_ERROR_CHECK(err_code);
        kb_event_handler(event_item.event, event_item.p_arg);
    }
}