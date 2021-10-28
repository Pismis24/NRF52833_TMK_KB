#include <stdint.h>

#include "ble_svc_hid.h"
#include "ble_service.h"
#include "host_driver.h"
#include "ble_protocol.h"


static uint8_t keyboard_leds()
{
    return keyboard_led_val_ble;
}

static void send_keyboard(report_keyboard_t * report)
{
    keys_send(0, 8, report->raw);
}

static void send_mouse(report_mouse_t * report)
{
    keys_send(REPORT_ID_MOUSE, sizeof(report_mouse_t), (uint8_t*)report);
}

static void send_system(uint16_t report)
{
    keys_send(REPORT_ID_SYSTEM, 2, (uint8_t*)&report);
}

static void send_consumer(uint16_t data)
{
    keys_send(REPORT_ID_CONSUMER, 2, (uint8_t*)&data);
}

host_driver_t ble_driver = {
    keyboard_leds,
    send_keyboard,
    send_mouse,
    send_system,
    send_consumer
};