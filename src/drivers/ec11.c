/*EC11的驱动程序*/
/*
使用QDEC外设对编码器的旋转进行读取，使用间隔扫描方式读取按键的触发情况
外设通过KB_EVT_INIT事件初始化
在KB_EVT_START事件后开始运行

按键触发扫描与去抖：
每隔ENCODER_SWITCH_SCAN_INTERVAL读取一次按键对应引脚的电平
当连续ENCODER_SWITCH_SCAN_VALID_TIMES次发现引脚的触发情况改变后改变虚拟按键的触发情况来进行去抖
也就意味着只有稳定触发超过ENCODER_SWITCH_SCAN_INTERVAL * ENCODER_SWITCH_SCAN_VALID_TIMES毫秒后判定按键摁下或抬起

编码器旋转的扫描与处理：
QDEC外设有自己的去抖
每次A端或B端发生电位改变时会产生一个NRF_QDEC_EVENT_REPORTRDY事件，event.data.report.acc会告知QDEC读取的编码器旋转方向，
由于1定位1脉冲的编码器每转动一格会分别在A端和B端产生2次电平变化（1下1上），因此每连续四次记录到同方向旋转认为其转了一格
判断出转动了一格后，产生虚拟按键一次摁下-抬起的模拟触发情况

*/

#include "config.h"

#ifdef EC11_ENCODER

#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"

#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "nrf_log_default_backends.h"

#include "app_timer.h"

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrfx_qdec.h"

#include "kb_matrix.h"
#include "kb_evt.h"

#ifdef EC11_WITH_SWITCH

#define ENCODER_SWITCH_SCAN_INTERVAL 2 //in ms
#define ENCODER_SWITCH_SCAN_VALID_TIMES 5

APP_TIMER_DEF(encoder_switch_timer);

static bool switch_frames[ENCODER_SWITCH_SCAN_VALID_TIMES];
static uint8_t switch_frame_idx;
static bool switch_state;

static void encoder_switch_timeout_handler(void* p_context)
{
    if(nrf_gpio_pin_read(ENCS) == 0){
        switch_frames[switch_frame_idx] = true;
    }
    else{
        switch_frames[switch_frame_idx] = false;
    }
    switch_frame_idx++;
    if(switch_frame_idx >= ENCODER_SWITCH_SCAN_VALID_TIMES){
        switch_frame_idx = 0;
    }
    bool flag = true;
    for(uint8_t idx=0; idx<ENCODER_SWITCH_SCAN_VALID_TIMES; idx++){
        if(switch_frames[idx] == switch_state){
            flag = false;
            break;
        }
    }
    if(flag){
        switch_state = !switch_state;
        matrix_extra_set(ENCODER_SWH, switch_state);
    }
}
#endif //EC11_WITH_SWITCH


const nrfx_qdec_config_t qdec_config = {
    .reportper = NRF_QDEC_REPORTPER_10,
    .sampleper = NRF_QDEC_SAMPLEPER_256us,
    .psela = ENCA,
    .pselb = ENCB,
    .pselled = NRF_QDEC_LED_NOT_CONNECTED,
    .dbfen = true,
    .sample_inten = true,
    .interrupt_priority = APP_IRQ_PRIORITY_MID,
};

enum rotate_dir{
    ROT_NEUTRAL,
    ROT_FORWARD,
    ROT_BACKWARD
};
static uint8_t pre_dir = ROT_NEUTRAL;
static uint8_t report_cnt = 1;

void decoder_event_handler(nrfx_qdec_event_t event)
{
    if(event.type == NRF_QDEC_EVENT_REPORTRDY){
        if(event.data.report.accdbl == 0){
            if(event.data.report.acc > 0){
                if(pre_dir == ROT_BACKWARD){
                    report_cnt = 1;
                }
                else{
                    report_cnt++;
                }
                pre_dir = ROT_FORWARD;
                if(report_cnt >= 4){
                    matrix_extra_add_oneshot(ENCODER_TN_POS);
                    report_cnt = 0;
                }
            }
            if(event.data.report.acc < 0){
                if(pre_dir == ROT_FORWARD){
                    report_cnt = 1;
                }
                else{
                    report_cnt++;
                }
                pre_dir = ROT_BACKWARD;
                if(report_cnt >= 4){
                    matrix_extra_add_oneshot(ENCODER_TN_NEG);
                    report_cnt = 0;
                }
            }
        }
    }
}


static void encoder_init(void)
{
    nrfx_qdec_init(&qdec_config, decoder_event_handler);
    nrf_gpio_cfg_input(qdec_config.psela, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(qdec_config.pselb, NRF_GPIO_PIN_PULLUP);
#ifdef EC11_WITH_SWITCH
    nrf_gpio_cfg_input(ENCS, NRF_GPIO_PIN_PULLUP);
    for(uint8_t idx=0; idx<ENCODER_SWITCH_SCAN_VALID_TIMES; idx++){
        switch_frames[idx] = false;
    }
    switch_frame_idx = 0;
    switch_state = false;
    app_timer_create(
        &encoder_switch_timer, 
        APP_TIMER_MODE_REPEATED, 
        encoder_switch_timeout_handler
    );
#endif
}

static void encoder_start(void)
{
    nrfx_qdec_enable();
    app_timer_start(encoder_switch_timer, APP_TIMER_TICKS(ENCODER_SWITCH_SCAN_INTERVAL), NULL);
}

static void encoder_deinit(void)
{
    nrfx_qdec_uninit();
    nrf_gpio_cfg_default(ENCA);
    nrf_gpio_cfg_default(ENCB);
#ifdef EC11_WITH_SWITCH
    app_timer_stop(encoder_switch_timer);
    nrf_gpio_cfg_default(ENCS);
#endif
}

static void encoder_wakeup_prepare(void)
{
    nrf_gpio_cfg_sense_input(ENCA, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(ENCB, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(ENCS, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
}

static void encoder_event_handler(kb_event_type_t event, void * p_arg)
{
    switch(event){
        case KB_EVT_INIT:
            encoder_init();
            NRF_LOG_INFO("Encoder Init");
            break;
        case KB_EVT_START:
            encoder_start();
            break;
        case KB_EVT_SLEEP:
            encoder_deinit();
            encoder_wakeup_prepare();
            break;
        default:
            break;
    }
}

KB_EVT_HANDLER(encoder_event_handler);

#endif //EC11_ENCODER

