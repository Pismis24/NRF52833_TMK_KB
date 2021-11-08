#include <stdint.h>

#include "host_driver.h"
#include "usb_protocol.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


//#include "usbd_hid.h"

static uint8_t keyboard_leds()
{
    //return keyboard_led_val_usb;
    return 0;
}

static void send_keyboard(report_keyboard_t * report)
{
    //usb_keys_send(0, 8, report->raw);
    NRF_LOG_INFO("send keyboard");
}

static void send_mouse(report_mouse_t * report)
{
    //usb_keys_send(REPORT_ID_MOUSE, sizeof(report_mouse_t), (uint8_t*)report);
    NRF_LOG_INFO("send mouse");
}

static void send_system(uint16_t report)
{
    //usb_keys_send(REPORT_ID_SYSTEM, 2, (uint8_t*)&report);
    NRF_LOG_INFO("send system");
}

static void send_consumer(uint16_t data)
{
    //usb_keys_send(REPORT_ID_CONSUMER, 2, (uint8_t*)&data);
    NRF_LOG_INFO("send consumer");
}

host_driver_t usb_driver = {
    keyboard_leds,
    send_keyboard,
    send_mouse,
    send_system,
    send_consumer
};