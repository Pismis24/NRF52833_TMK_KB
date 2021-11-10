#include <stdint.h>

#include "host_driver.h"
#include "usb_protocol.h"
#include "usbd_hid.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


//#include "usbd_hid.h"

static uint8_t keyboard_leds()
{
    //return keyboard_led_val_usb;
    return keyboard_led_val_usbd;
}

static void send_keyboard(report_keyboard_t * report)
{
    usb_kbd_keys_send(report);
}

static void send_mouse(report_mouse_t * report)
{
    usb_mouse_send(report);
}

static void send_system(uint16_t report)
{
    usb_system_send(report);
}

static void send_consumer(uint16_t data)
{
    usb_consumer_send(data);
}

host_driver_t usb_driver = {
    keyboard_leds,
    send_keyboard,
    send_mouse,
    send_system,
    send_consumer
};