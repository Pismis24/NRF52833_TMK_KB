#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_util_platform.h"
#include "nrf.h"
#include "nrf_drv_usbd.h"

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_generic.h"

#include "app_usbd_cdc_acm.h"

#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "usb_hid_descriptor.h"
#include "usbd_hid.h"

#include "kb_evt.h"

uint8_t keyboard_led_val_usbd = 0;
bool usbd_is_enabled = false;


/*
    Function of setting keyboard_led_value
*/
static void set_usb_led_stat(void)
{
    NRF_LOG_INFO("USB led stat set");
}

/**
 * @brief USBD library specific event handler.
 *
 * @param event     USBD library event.
 * */
static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SOF:
            break;
        case APP_USBD_EVT_DRV_SUSPEND:
            app_usbd_suspend_req(); // Allow the library to put the peripheral into sleep mode
            
            break;
        case APP_USBD_EVT_DRV_RESUME:
            
            set_usb_led_stat(); /* Restore LED state - during SUSPEND all LEDS are turned off */
            break;
        case APP_USBD_EVT_STARTED:
            
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_INFO("USB power detected");
            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            NRF_LOG_INFO("USB power removed");
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            NRF_LOG_INFO("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}

/*
    Function of init the usb device
*/
static void usbd_init(void)
{
    ret_code_t err_code;
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler,
    };
    err_code = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(err_code);
}

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    //do nothing
}
#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2
#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1
/**
   @brief CDC_ACM class instance
 */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

/*
    Function of appending the hid classes
*/
static void usbd_classes_append(void)
{
    ret_code_t ret;
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);
}

/*
    Function of parepare usb device
*/
void usbd_perpare(void)
{
    usbd_init();
    usbd_classes_append();
    ret_code_t err_code;
    err_code = app_usbd_power_events_enable();
    APP_ERROR_CHECK(err_code);
}

/*
    Function of process usbd events
*/
void usbd_evt_process(void)
{
    while(app_usbd_event_queue_process()){
        // do nothing
    }
}



void usb_kbd_keys_send(report_keyboard_t * report)
{

}
void usb_mouse_send(report_mouse_t * report)
{

}
void usb_system_send(uint16_t report)
{

}
void usb_consumer_send(uint16_t report)
{
    
}