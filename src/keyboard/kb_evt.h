#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "nrf_section.h"

typedef enum kb_event_lst {
    KB_EVT_INIT,
    KB_EVT_START,
    KB_EVT_POWERSAVE,
    KB_EVT_SLEEP,
    KB_EVT_USB,
    KB_EVT_BLE,
    KB_EVT_TMK_HOOK
} kb_event_type_t;

enum kb_ble_evt_lst {
    KB_BLE_ADV_FAST,
    KB_BLE_ADV_SLOW,
    KB_BLE_ADV_STOP,
    KB_BLE_GAP_CONN,
    KB_BLE_GAP_DISCONN,
    KB_BLE_GATT_TX_DONE,
};

enum kb_powersave_subevt {
    KB_POWERSAVE_ENTER,
    KB_POWERSAVE_EXIT
};

enum tmk_hook_evt_lst {
    TMK_HOOK_LOOP,
    TMK_HOOK_MATRIX_CHG,
};


typedef void (*KbEvtHandler)(kb_event_type_t, void*);

void execute_kb_event(void);
void trig_kb_event(kb_event_type_t event);
void trig_kb_event_param(kb_event_type_t event, uint8_t arg);

#define KB_EVT_HANDLER(_func) NRF_SECTION_ITEM_REGISTER(kbd_event, const KbEvtHandler _pt_##_func) = &_func