#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <host_driver.h>

void usbd_perpare(void);
void usbd_evt_process(void);


void usb_kbd_keys_send(report_keyboard_t * report);
void usb_mouse_send(report_mouse_t * report);
void usb_system_send(uint16_t report);
void usb_consumer_send(uint16_t report);

extern uint8_t keyboard_led_val_usbd;
extern bool usbd_is_enabled;