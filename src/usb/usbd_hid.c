#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "app_util_platform.h"
#include "nrf.h"
#include "nrf_drv_usbd.h"

#include "nrf_delay.h"

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_generic.h"

#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "usb_hid_descriptor.h"
#include "usbd_hid.h"

#include "kb_evt.h"

#define KBD_REP_QUEUE_LEN 32

#define HID_KBD_INTERFACE 0
#define HID_MOUSE_INTERFACE 1
#define HID_EXTRA_INTERFACE 2

//USB KB LED state
uint8_t keyboard_led_val_usbd = 0;
//flag of usb device is started
static bool usbd_is_started = false;
//flag of usb device is pending report
static bool m_report_pending = false;

//Setting keyboard hid interface
void hid_kbd_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event);

static const app_usbd_hid_subclass_desc_t *keyboard_hid_report_desc[] = {&kbd_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_kbd,
                                HID_KBD_INTERFACE,
                                hid_kbd_user_ev_handler,
                                (NRF_DRV_USBD_EPIN1),
                                keyboard_hid_report_desc,
                                5,
                                1,
                                0,
                                APP_USBD_HID_SUBCLASS_BOOT,
                                APP_USBD_HID_PROTO_KEYBOARD);

// handler of mousekey and extrakey hid evt
void hid_oth_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event);
//Setting mouse hid interface
#ifdef MOUSEKEY_ENABLE
static const app_usbd_hid_subclass_desc_t *mouse_hid_report_desc[] = {&mouse_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_mouse,
                                HID_MOUSE_INTERFACE,
                                hid_oth_user_ev_handler,
                                (NRF_DRV_USBD_EPIN2),
                                mouse_hid_report_desc,
                                1,
                                0,
                                0,
                                APP_USBD_HID_SUBCLASS_NONE,
                                APP_USBD_HID_PROTO_GENERIC);
#endif

//Setting extra hid interface
#ifdef EXTRAKEY_ENABLE
static const app_usbd_hid_subclass_desc_t *extra_hid_report_desc[] = {&extrakey_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_extra,
                                HID_EXTRA_INTERFACE,
                                hid_oth_user_ev_handler,
                                (NRF_DRV_USBD_EPIN3),
                                extra_hid_report_desc,
                                1,
                                0,
                                0,
                                APP_USBD_HID_SUBCLASS_NONE,
                                APP_USBD_HID_PROTO_GENERIC);
#endif

//kbd report buffer queue
static uint8_t kbd_report_queue[KBD_REP_QUEUE_LEN][KEYBOARD_REPORT_SIZE];
static volatile uint8_t kbd_report_queue_wp = 0;
static volatile uint8_t kbd_report_queue_rp = 0;

static bool kbd_report_queue_is_empty(void)
{
    if (kbd_report_queue_wp == kbd_report_queue_rp)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool kbd_report_queue_is_full(void)
{
    if ((kbd_report_queue_wp + 1) % KBD_REP_QUEUE_LEN == kbd_report_queue_rp)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void kbd_report_queue_init(void)
{
    memset(kbd_report_queue, 0, KBD_REP_QUEUE_LEN * KEYBOARD_REPORT_SIZE * sizeof(uint8_t));
    kbd_report_queue_wp = 0;
    kbd_report_queue_rp = 0;
}

/*function of sending current*/
static void kbd_key_buffer_send(void)
{
    if (kbd_report_queue_is_empty())
    {
        return;
    }
    if (m_report_pending)
    {
        return;
    }
    ret_code_t err_code;
    err_code = app_usbd_hid_generic_in_report_set(
        &m_app_hid_kbd,
        kbd_report_queue[kbd_report_queue_rp],
        KEYBOARD_REPORT_SIZE);
    m_report_pending = true;
    if (err_code == NRF_SUCCESS)
    {
        NRF_LOG_INFO("usb buffer out");
        kbd_report_queue_rp++;
        kbd_report_queue_rp %= KBD_REP_QUEUE_LEN;
    }
    else if (err_code == NRF_ERROR_BUSY){
        //do nothing
    }
    else
    {
        APP_ERROR_CHECK(err_code);
    }
}

/*
    Function of sending kbd keys
*/
void usb_kbd_keys_send(report_keyboard_t *report)
{
    if (!NRF_USBD->ENABLE)
    {
        return;
    }
    ret_code_t err_code;
    if(!kbd_report_queue_is_empty())
    {
        NRF_LOG_INFO("usb buffer in");
        memcpy(kbd_report_queue[kbd_report_queue_wp], report, KEYBOARD_REPORT_SIZE);
        kbd_report_queue_wp++;
        kbd_report_queue_wp %= KBD_REP_QUEUE_LEN;
        kbd_key_buffer_send();
        return;
    }
    if (m_report_pending)
    {
        NRF_LOG_INFO("usb buffer in");
        memcpy(kbd_report_queue[kbd_report_queue_wp], report, KEYBOARD_REPORT_SIZE);
        kbd_report_queue_wp++;
        kbd_report_queue_wp %= KBD_REP_QUEUE_LEN;
    }
    else
    {
        err_code = app_usbd_hid_generic_in_report_set(
            &m_app_hid_kbd,
            report,
            KEYBOARD_REPORT_SIZE);
        if(err_code == NRF_ERROR_BUSY){
            NRF_LOG_INFO("usb buffer in");
            memcpy(kbd_report_queue[kbd_report_queue_wp], report, KEYBOARD_REPORT_SIZE);
            kbd_report_queue_wp++;
            kbd_report_queue_wp %= KBD_REP_QUEUE_LEN;
        }
        else if(err_code == NRF_SUCCESS){
        m_report_pending = true;        
        }
        else{
            APP_ERROR_CHECK(err_code);
        }
    }
}

/*
    Function of sending mouse keys
*/
#ifdef MOUSEKEY_ENABLE
void usb_mouse_send(report_mouse_t *report)
{
    ret_code_t err_code;
    if (!NRF_USBD->ENABLE)
    {
        return;
    }
    if (m_report_pending)
    {
        return;
    }
    else
    {
        err_code = app_usbd_hid_generic_in_report_set(
            &m_app_hid_mouse,
            &report->buttons,
            5);
        APP_ERROR_CHECK(err_code);
    }
}
#endif

#ifdef EXTRAKEY_ENABLE
typedef struct{
    uint8_t report_id;
    uint16_t usage;
} __attribute__ ((packed)) report_extra_t;

static void extra_key_send(uint8_t report_id, uint16_t data)
{
    if (!NRF_USBD->ENABLE)
    {
        return;
    }
    if (m_report_pending)
    {
        return;
    }
    ret_code_t err_code;

    static report_extra_t report;
    report.report_id = report_id;
    report.usage = data;

    err_code = app_usbd_hid_generic_in_report_set(
        &m_app_hid_extra,
        &report,
        sizeof(report_extra_t));
    APP_ERROR_CHECK(err_code);
}

void usb_system_send(uint16_t report)
{
    extra_key_send(REPORT_ID_SYSTEM, report);
}
void usb_consumer_send(uint16_t report)
{
    extra_key_send(REPORT_ID_CONSUMER, report);
}
#endif

/*
    Function of setting keyboard_led_value
*/
static void set_usb_led_stat(void)
{
    size_t led_stats_size = 0;
    const uint8_t *p_buff;
    p_buff = app_usbd_hid_generic_out_report_get(&m_app_hid_kbd, &led_stats_size);
    keyboard_led_val_usbd = *(p_buff+1);
}

/**
 * @brief Class specific event handler.
 *
 * @param p_inst    Class instance.
 * @param event     Class specific event.
 * */
void hid_kbd_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
    {
        m_report_pending = false;
        set_usb_led_stat();
        break;
    }
    case APP_USBD_HID_USER_EVT_IN_REPORT_DONE:
    {
        m_report_pending = false;
        kbd_key_buffer_send();
        break;
    }
    case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO:
    {
        UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
        kbd_report_queue_init();
        NRF_LOG_INFO("SET_BOOT_PROTO");
        break;
    }
    case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO:
    {
        UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
        kbd_report_queue_init();
        NRF_LOG_INFO("SET_REPORT_PROTO");
        break;
    }
    default:
        break;
    }
}

/**
 * @brief Class specific event handler.
 *
 * @param p_inst    Class instance.
 * @param event     Class specific event.
 * */
void hid_oth_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
    {
        m_report_pending = false;

        break;
    }
    case APP_USBD_HID_USER_EVT_IN_REPORT_DONE:
    {
        m_report_pending = false;

        break;
    }
    case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO:
    {
        UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
        NRF_LOG_INFO("SET_BOOT_PROTO");
        break;
    }
    case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO:
    {
        UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
        NRF_LOG_INFO("SET_REPORT_PROTO");
        break;
    }
    default:
        break;
    }
}

#ifdef MOUSEKEY_ENABLE
static ret_code_t mousekey_idle_handle(app_usbd_class_inst_t const *p_inst, uint8_t report_id)
{
    switch (report_id)
    {
    case 0:
    {
        uint8_t report[] = {0xBE, 0xEF};
        return app_usbd_hid_generic_idle_report_set(&m_app_hid_mouse, report, sizeof(report));
    }
    default:
        return NRF_ERROR_NOT_SUPPORTED;
    }
}
#endif


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
    case APP_USBD_EVT_DRV_RESET:
        m_report_pending = false;
        break;
    case APP_USBD_EVT_DRV_SUSPEND:
        m_report_pending = false;
        app_usbd_suspend_req(); // Allow the library to put the peripheral into sleep mode
        break;
    case APP_USBD_EVT_DRV_RESUME:
        m_report_pending = false;
        set_usb_led_stat(); /* Restore LED state - during SUSPEND all LEDS are turned off */
        break;
    case APP_USBD_EVT_STARTED:
        m_report_pending = false;
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
        trig_kb_event_param(KB_EVT_USB, KB_USB_POWER_CONN);
        break;
    case APP_USBD_EVT_POWER_REMOVED:
        NRF_LOG_INFO("USB power removed");
        app_usbd_stop();
        trig_kb_event_param(KB_EVT_USB, KB_USB_POWER_DISCONN);
        trig_kb_event_param(KB_EVT_USB, KB_USB_STOP);
        break;
    case APP_USBD_EVT_POWER_READY:
        NRF_LOG_INFO("USB ready");
        app_usbd_start();
        trig_kb_event_param(KB_EVT_USB, KB_USB_START);
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

/*
    Function of appending the hid classes
*/
static void usbd_classes_append(void)
{
    ret_code_t err_code;
    app_usbd_class_inst_t const *class_inst_kbd;
    class_inst_kbd = app_usbd_hid_generic_class_inst_get(&m_app_hid_kbd);
    err_code = app_usbd_class_append(class_inst_kbd);
    APP_ERROR_CHECK(err_code);
    kbd_report_queue_init();
#ifdef MOUSEKEY_ENABLE
    app_usbd_class_inst_t const *class_inst_mouse;
    class_inst_mouse = app_usbd_hid_generic_class_inst_get(&m_app_hid_mouse);
    err_code = hid_generic_idle_handler_set(class_inst_mouse, mousekey_idle_handle);
    err_code = app_usbd_class_append(class_inst_mouse);
    APP_ERROR_CHECK(err_code);
#endif
#ifdef EXTRAKEY_ENABLE
    app_usbd_class_inst_t const *class_inst_extra;
    class_inst_extra = app_usbd_hid_generic_class_inst_get(&m_app_hid_extra);
    err_code = app_usbd_class_append(class_inst_extra);
    APP_ERROR_CHECK(err_code);
#endif
}

/*
    Function of parepare usb device
*/
void usbd_prepare(void)
{
    usbd_init();
    usbd_classes_append();
    nrf_delay_ms(50);
    ret_code_t err_code;
    err_code = app_usbd_power_events_enable();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("USB HID perpared");
}

/*
    Function of process usbd events
*/
void usbd_evt_process(void)
{
    while (app_usbd_event_queue_process())
    {
        // do nothing
    }
}
