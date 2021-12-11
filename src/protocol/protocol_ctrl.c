#include <stdint.h>
#include <stdbool.h>

#include "app_error.h"

#include "app_usbd.h"

//tmk headers
#include "host_driver.h"
#include "host.h"

#include "kb_evt.h"
#include "kb_storage.h"

#include "ble_main.h"

#include "ble_protocol.h"
#include "usb_protocol.h"

#include "protocol_ctrl.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

typedef enum protocol_lst {
    PROTOCOL_BLE,
    PROTOCOL_USB
} protocol_type_t;

static store_data_t stored_data;
static uint8_t current_protocol = KB_PROTOCOL_UNSET;
static host_driver_t current_driver;

static void protocol_first_set(void)
{
    ret_code_t err_code;
    //read current protocol
    storage_read(&stored_data);
    switch(stored_data.kb_current_protocol)
    {
    case KB_PROTOCOL_BLE:
        trig_kb_event_param(KB_EVT_PROTOCOL_SWITCH, SUBEVT_PROTOCOL_BLE);
        NRF_LOG_INFO("Stored Protocol is BLE");
    break;
    case KB_PROTOCOL_USB:
        trig_kb_event_param(KB_EVT_PROTOCOL_SWITCH, SUBEVT_PROTOCOL_USB);
        NRF_LOG_INFO("Stored Protocol is USB");
    break;
    default:
        trig_kb_event_param(KB_EVT_PROTOCOL_SWITCH, SUBEVT_PROTOCOL_BLE);
        NRF_LOG_INFO("No Protocol data found");
    break;
    }
}


static void protocol_switch(protocol_type_t protocol_type)
{
    switch(protocol_type){
    case PROTOCOL_BLE:
         ble_conn_restart();
         current_driver = ble_driver;
         host_set_driver(&current_driver);

         storage_read(&stored_data);
         stored_data.kb_current_protocol = KB_PROTOCOL_BLE;
         storage_write(stored_data);
         current_protocol = KB_PROTOCOL_BLE;
         NRF_LOG_INFO("BLE HID Protocol Set");
         
    break;
    case PROTOCOL_USB:
         current_driver = usb_driver;
         host_set_driver(&current_driver);

         storage_read(&stored_data);
         stored_data.kb_current_protocol = KB_PROTOCOL_USB;
         storage_write(stored_data);

         ble_conn_close();
         current_protocol = KB_PROTOCOL_USB;
         NRF_LOG_INFO("USB HID Protocol Set");
    default:
        //do nothing
    break;
    }
}


static void protocol_evt_handler(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event){
    case KB_EVT_START:
        protocol_first_set();
        NRF_LOG_INFO("Protocol Inited");
    break;
    case KB_EVT_PROTOCOL_SWITCH:
        switch(param){
            case SUBEVT_PROTOCOL_BLE:
                NRF_LOG_INFO("current protocol %d", current_protocol);
                if(current_protocol != KB_PROTOCOL_BLE){
                    protocol_switch(PROTOCOL_BLE);
                }
            break;
            case SUBEVT_PROTOCOL_USB:
                NRF_LOG_INFO("current protocol %d", current_protocol);
                if(current_protocol != KB_PROTOCOL_USB){
                    protocol_switch(PROTOCOL_USB);
                }
            break;
        }
    break;
    case KB_EVT_USB:
        if(param == KB_USB_STOP){
            trig_kb_event_param(KB_EVT_PROTOCOL_SWITCH, SUBEVT_PROTOCOL_BLE);
        }
    break;
    }
}

KB_EVT_HANDLER(protocol_evt_handler);





