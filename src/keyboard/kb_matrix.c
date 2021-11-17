#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "nrf_queue.h"

#include "keyboard.h"
#include "matrix.h"

#include "kb_evt.h"

#include "config.h"

#ifndef MATRIX_SCAN_VALID_TIMES
#define MATRIX_SCAN_VALID_TIMES 1
#endif

static bool sleep_flag = false;

//消抖队列定义
typedef struct {
    matrix_row_t* frames;
    uint8_t pointer_idx;
} debounce_queue_t;
//消抖队列初始化
static void debounce_queue_init(debounce_queue_t* queue){
    queue->frames = (matrix_row_t*)malloc(MATRIX_SCAN_VALID_TIMES * sizeof(matrix_row_t));
    for(uint8_t idx=0; idx<MATRIX_SCAN_VALID_TIMES; idx++){
        queue->frames[idx] = 0;
    }
    queue->pointer_idx = 0;
}
//消抖队列加入
static void debounce_queue_in(debounce_queue_t* queue, matrix_row_t item){
    queue->frames[queue->pointer_idx] = item;
    queue->pointer_idx++;
    queue->pointer_idx %= MATRIX_SCAN_VALID_TIMES;
}
//输出消抖队列结果
static matrix_row_t debounce_result(debounce_queue_t* queue, matrix_row_t ori_value){
    matrix_row_t need_change = ~(matrix_row_t)0;
    for(uint8_t i=0; i<MATRIX_SCAN_VALID_TIMES; i++){
        need_change &= (ori_value ^ queue->frames[i]);
    }
    return need_change ^ ori_value;
}

//定义消抖队列
static debounce_queue_t debounce_queues[MATRIX_ROWS];
//定义按键矩阵触发情况数组
static matrix_row_t matrix[MATRIX_ROWS];

//矩阵函数
inline uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

inline bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}


//矩阵初始化
static void cols_init(void)
{
    for(uint8_t idx=0; idx<KEY_COLS; idx++){
#ifdef DIODES_ROW2COL
        nrf_gpio_cfg_input((uint32_t)col_pins[idx], NRF_GPIO_PIN_PULLDOWN);
#else
        nrf_gpio_cfg_input((uint32_t)col_pins[idx], NRF_GPIO_PIN_PULLUP);
#endif
//当二极管方向是从行到列时，列管脚开启下拉电阻，高电平触发
    }
}


static void rows_init(void)
{
    for(uint8_t idx=0; idx<KEY_ROWS; idx++){
        nrf_gpio_cfg_output((uint32_t)row_pins[idx]);
#ifdef DIODES_ROW2COL
        nrf_gpio_pin_clear((uint32_t)row_pins[idx]);
#else
        nrf_gpio_pin_set((uint32_t)row_pins[idx]);
#endif
//当二极管方向是从行到列时，行默认输出低，扫描时设为高
    }
}


static matrix_row_t cols_read(void)
{
    matrix_row_t result = 0;
    for(uint8_t i = 0; i<KEY_COLS; i++){
        uint32_t col_stat = nrf_gpio_pin_read(((uint32_t)col_pins[i]));
        #ifdef DIODES_ROW2COL
        if(col_stat){
            result |= 1 << i;
        }
        #else
        if(!col_stat){
            result |= 1 << i;
        }
        #endif
        //二极管行到列，读到高电平触发
        //二极管列到行，读到低电平触发
    }
    return result;
}


static void select_row(uint8_t row)
{
    #ifdef DIODES_ROW2COL
    nrf_gpio_pin_set((uint32_t)row_pins[row]);
    #else
    nrf_gpio_pin_clear((uint32_t)row_pins[row]);
    #endif
}


static void unselect_row(uint8_t row)
{
    #ifdef DIODES_ROW2COL
    nrf_gpio_pin_clear((uint32_t)row_pins[row]);
    #else
    nrf_gpio_pin_set((uint32_t)row_pins[row]);
    #endif
}


uint8_t matrix_scan(void)
{
    if(sleep_flag){ return 0; }
    //遍历行
    for(uint8_t idx=0; idx<KEY_ROWS; idx++){
        //将该行激活
        select_row(idx);
        //等待稳定
        nrf_delay_us(25);
        //读取行的列触发情况
        matrix_row_t row_value = cols_read();
        //存入队列，之前已经设置队列满后自动剔除最先进入的数值
        debounce_queue_in(&debounce_queues[idx], row_value);
        //输出消抖结果
        matrix[idx] = debounce_result(&debounce_queues[idx], matrix[idx]);
        //结束行激活状态
        unselect_row(idx);
        //等待稳定
        nrf_delay_us(25);
    }
    return 1;
}

#ifdef MATRIX_EXTRAKEY_EXIST
static bool matrix_oneshot_send[MATRIX_ROWS];
static matrix_row_t matrix_extra_oneshot[MATRIX_ROWS];
static matrix_row_t matrix_extra[MATRIX_ROWS];

void matrix_extra_add_oneshot(uint8_t row, uint8_t col)
{
    if(row >= MATRIX_ROWS){
        return;
    }
    if(col >= sizeof(matrix_row_t) * 8){
        return;
    }
    matrix_extra_oneshot[row] |= (1 << col);
    matrix_oneshot_send[row] = true;
    //NRF_LOG_INFO("One shot pos %d %d", row, col);
}

void matrix_extra_set(uint8_t row, uint8_t col, bool press)
{
    if(row >= MATRIX_ROWS){
        return;
    }
    if(col >= sizeof(matrix_row_t) * 8){
        return;
    }
    if(press){
        matrix_extra[row] |= (1 << col);
    }
    else{
        matrix_extra[row] &= ~(1 << col);
    }
    //NRF_LOG_INFO("EXTRA KEY SET %d %d", row, col);
}

inline matrix_row_t matrix_get_row(uint8_t row)
{
    matrix_row_t value = matrix[row] | matrix_extra[row];

    if(matrix_oneshot_send[row]){
        value |= matrix_extra_oneshot[row];
        matrix_extra_oneshot[row] = 0;
        matrix_oneshot_send[row] = false;
    }
    return value;
}
#else
inline matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}
#endif

void matrix_init(void)
{
    cols_init();
    rows_init();
    for(uint8_t idx=0; idx<MATRIX_ROWS; idx++){
        debounce_queue_init(&debounce_queues[idx]);
        matrix[idx] = 0;
#ifdef MATRIX_EXTRAKEY_EXIST
        matrix_oneshot_send[idx] = false;
        matrix_extra_oneshot[idx] = 0;
        matrix_extra[idx] = 0;
#endif
    }
}


static void matrix_wakeup_prepare(void)
{
#ifdef DIODES_ROW2COL
    for(uint8_t idx=0; idx<KEY_COLS; idx++){
        nrf_gpio_cfg_sense_input(col_pins[idx], NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    }
    for(uint8_t idx=0; idx<KEY_ROWS; idx++){
        nrf_gpio_cfg_output(row_pins[idx]);
        nrf_gpio_pin_set(row_pins[idx]);
    }
#else
    for(uint8_t idx=0; idx<MATRIX_COLS; idx++){
        nrf_gpio_cfg_sense_input(col_pins[idx], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    }
    for(uint8_t idx=0; idx<MATRIX_ROWS; idx++){
        nrf_gpio_cfg_output(row_pins[idx]);
        nrf_gpio_pin_clear(row_pins[idx]);
    }
#endif
    NRF_LOG_INFO("matrix wake up perpared");
}

static void matrix_event_handle(kb_event_type_t event, void * p_arg)
{
    switch(event){
        case KB_EVT_SLEEP:
            sleep_flag = true;
            matrix_wakeup_prepare();
        break;
    }
}

KB_EVT_HANDLER(matrix_event_handle);