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

#include "kb_storage.h"

#include "config.h"

static store_data_t stored_data;

void eeconfig_init()
{
    //已经在storage init中初始化过了
}

void eeconfig_enable(void)
{
    storage_read(&stored_data);
    stored_data.config_data.magic = EECONFIG_MAGIC_NUMBER;
    storage_write(stored_data);
}

void eeconfig_disable(void)
{
    storage_read(&stored_data);
    stored_data.config_data.magic = 0xFFFF;
    storage_write(stored_data);
}

bool eeconfig_is_enabled(void)
{
    storage_read(&stored_data);
    if(stored_data.config_data.magic == EECONFIG_MAGIC_NUMBER){
        return true;
    }
    else{
        return false;
    }
}

uint8_t eeconfig_read_debug(void)
{
    storage_read(&stored_data);
    return stored_data.config_data.debug;
}
void eeconfig_write_debug(uint8_t val)
{
    storage_read(&stored_data);
    stored_data.config_data.debug = val;
    storage_write(stored_data);
}

uint8_t eeconfig_read_default_layer(void)
{
    storage_read(&stored_data);
    return stored_data.config_data.default_layer;
}

void eeconfig_write_default_layer(uint8_t val)
{
    storage_read(&stored_data);
    stored_data.config_data.default_layer = val;
    storage_write(stored_data);
}

uint8_t eeconfig_read_keymap(void)
{
    storage_read(&stored_data);
    return stored_data.config_data.keymap;
}

void eeconfig_write_keymap(uint8_t val)
{
    storage_read(&stored_data);
    stored_data.config_data.keymap = val;
    storage_write(stored_data);
}

#ifdef BACKLIGHT_ENABLE
uint8_t eeconfig_read_backlight(void)
{
    storage_read(&stored_data);
    return stored_data.config_data.backlight;
}

void eeconfig_write_backlight(uint8_t val)
{
    storage_read(&stored_data);
    stored_data.config_data.backlight = val;
    storage_write(stored_data);
}
#endif