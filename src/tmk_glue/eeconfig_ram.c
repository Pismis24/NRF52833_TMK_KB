#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//Log Headers
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//Error Handler
#include "app_error.h"

#include "eeconfig.h"

#include "config.h"

//TODO
//This file will rewrite with nRF FDS module

//eeconfig 数据结构体
typedef struct
{
    uint16_t magic;
    uint8_t debug;
    uint8_t default_layer;
    uint8_t keymap;
    uint8_t mousekey_accel;
    uint8_t backlight;
}__attribute__((aligned(4))) eeconfig_data_t;


static eeconfig_data_t eeconfig_data = 
{
    .magic = 0,
    .debug = 0,
    .default_layer = 0,
    .keymap = 0,
    .mousekey_accel = 0,
    .backlight = 0,
};


void eeconfig_init(void)
{
    eeconfig_data.magic = EECONFIG_MAGIC_NUMBER;
    eeconfig_data.debug = 0;
    eeconfig_data.default_layer = 0;
    eeconfig_data.keymap = 0;
    eeconfig_data.mousekey_accel = 0;
    eeconfig_data.backlight = 0;
}

void eeconfig_enable(void)
{
    eeconfig_data.magic = EECONFIG_MAGIC_NUMBER;
}

void eeconfig_disable(void)
{
    eeconfig_data.magic = 0xFFFF;
}

bool eeconfig_is_enabled(void)
{
    return (eeconfig_data.magic == EECONFIG_MAGIC_NUMBER);
}

uint8_t eeconfig_read_debug(void)
{
    return eeconfig_data.debug;
}

void eeconfig_write_debug(uint8_t val)
{
    eeconfig_data.debug = val;
}

uint8_t eeconfig_read_default_layer(void)
{
    return eeconfig_data.default_layer;
}

void eeconfig_write_default_layer(uint8_t val)
{
    eeconfig_data.default_layer = val;
}

uint8_t eeconfig_read_keymap(void)
{
    return eeconfig_data.keymap;
}

void eeconfig_write_keymap(uint8_t val)
{
    eeconfig_data.keymap = val;
}

#ifdef BACKLIGHT_ENABLE
uint8_t eeconfig_read_backlight(void)
{
    return eeconfig_data.backlight;
}

void eeconfig_write_backlight(uint8_t val)
{
    eeconfig_data.backlight = val;
}

#endif