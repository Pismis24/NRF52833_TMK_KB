#pragma once

#include <stdint.h>
#include <stdbool.h>

#define STORE_FILE (0x1024)
#define STORE_KEY (0x0001)

typedef struct
{
    uint16_t magic;
    uint8_t debug;
    uint8_t default_layer;
    uint8_t keymap;
    uint8_t mousekey_accel;
    uint8_t backlight;
} config_data_t;

typedef struct
{
    config_data_t config_data;
    uint8_t kb_current_protocol;
}__attribute__((aligned(4))) store_data_t;

enum protocol_types {
    KB_PROTOCOL_UNSET = 0,
    KB_PROTOCOL_BLE,
    KB_PROTOCOL_USB
};

//初始化存储
void storage_init(void);
//将data写入存储
void storage_write(store_data_t data);
//将data读出存储
bool storage_read(store_data_t * data);
//删除存储
void storage_del(void);