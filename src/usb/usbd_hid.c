#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "nrf.h"
#include "app_util_platform.h"
#include "nrf_drv_usbd.h"

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_generic.h"

#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "usbd_hid.h"
#include "usb_hid_descriptor.h"

#include "kb_evt.h"

/*
    Define USB HID KBD Interface
*/
#define HID_GENERIC_INTERFACE 0

/**
 * @brief HID generic class endpoint number.
 * */
#define HID_GENERIC_EPIN       NRF_DRV_USBD_EPIN1

/**
 * @brief List of HID generic class endpoints.
 * */
#define ENDPOINT_LIST()                                      \
(                                                            \
        HID_GENERIC_EPIN                                     \
)

#define REPORT_OUT_SIZE 1 //指示灯状态8bit
#define REPORT_FEATURE_MAXSIZE 31

enum input_report_index {
    INPUT_REP_KBD_INDEX,
#ifdef MOUSEKEY_ENABLE
    INPUT_REP_MOUSE_INDEX,
#endif
#ifdef EXTRAKEY_ENABLE
    INPUT_REP_SYSTEM_INDEX,
    INPUT_REP_CONSUMER_INDEX,
#endif
    INPUT_REP_COUNT
};

typedef hid_buffer_entry{
    uint8_t * 
}



/**
 * @brief Mark the ongoing transmission
 *
 * Marks that the report buffer is busy and cannot be used until transmission finishes
 * or invalidates (by USB reset or suspend event).
 */
static bool m_report_pending;

/*
    Led data of usb (Capslock etc.)
*/
uint8_t keyboard_led_val_usbd;

void usb_keys_send()





/**
 * @brief User event handler.
 * */
static void hid_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_hid_user_event_t event);

/**
 * @brief Global HID generic instance
 */
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_generic,
                                HID_GENERIC_INTERFACE,
                                hid_user_ev_handler,
                                ENDPOINT_LIST(),
                                usb_hid_descriptor,
                                INPUT_REP_COUNT,
                                REPORT_OUT_MAXSIZE,
                                REPORT_OUT_SIZE,
                                APP_USBD_HID_SUBCLASS_BOOT,
                                APP_USBD_HID_PROTO_KEYBOARD);

/**
 * @brief Class specific event handler.
 *
 * @param p_inst    Class instance.
 * @param event     Class specific event.
 * */
static void hid_kbd_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_hid_user_event_t event)
{
    UNUSED_PARAMETER(p_inst);
    switch (event) {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
            keyboard_led_val_usbd = *(uint8_t*)(app_usbd_hid_generic_out_report_get(p_inst, REPORT_OUT_SIZE));
            break;
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE:
            m_report_pending = false;
            send_next_buffer();
            break;
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO:
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_BOOT_PROTO");
            break;
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO:
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_REPORT_PROTO");
            break;
        default:
            break;
    }
}

static void usbd_user_evt_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SOF:
            break;
        case APP_USBD_EVT_DRV_RESET:
            m_report_pending = false;
            break;
        case APP_USBD_EVT_DRV_SUSPEND:
            m_report_pending = false;
            app_usbd_suspend_req(); // Allow the library to put the peripheral into sleep mode
            break;
        case APP_USBD_EVT_DRV_RESUME:
            m_report_pending = false;
            
            break;
        case APP_USBD_EVT_STARTED:
            m_report_pending = false;
            usbd_is_started = true;
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            usbd_is_started = false;
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


static void usbd_init(void)
{
    ret_code_t err_code;
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_evt_handler};
    err_code = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(err_code);
}

static void usb_hid_class_append(void)
{
    ret_code_t err_code;
    app_usbd_class_inst_t const *class_inst_kbd;
    class_inst_kbd = app_usbd_hid_kbd_class_inst_get(&m_app_hid_kbd);
    err_code = app_usbd_class_append(class_inst_kbd);
    APP_ERROR_CHECK(err_code);
}