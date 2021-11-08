#pragma once

#include <stdint.h>

void usb_keys_send(uint8_t report_index, uint8_t key_pattern_len, uint8_t* p_key_pattern);

extern uint8_t keyboard_led_val_usbd;
extern bool usbd_is_started;