#include "config.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrfx_saadc.h"

#include "app_timer.h"

#include "kb_adc.h"
#include "kb_evt.h"

#define SAADC_CHANNEL_COUNT 1
#define SAADC_SAMPLE_INTERVAL_MS 1000
#define SAADC_SAMPLE_PS_INTERVAL_MS 10000
#define BATTERY_BUFFER_SIZE 5

APP_TIMER_DEF(voltage_meas_timer);

static uint8_t batt_percentage = 100;
static nrf_saadc_value_t saadc_samples[SAADC_CHANNEL_COUNT];
static nrfx_saadc_channel_t saadc_channels[SAADC_CHANNEL_COUNT] = {NRFX_SAADC_DEFAULT_CHANNEL_SE(VOLTAGE_SOURCE, 0)};
static uint32_t sample_time_ticks;


struct measure_buffer {
    uint8_t buffer[BATTERY_BUFFER_SIZE];
    uint8_t pointer;
    uint8_t valid_num;
} saadc_buffer;

static void buffer_init(void)
{
    memset(saadc_buffer.buffer, 0, BATTERY_BUFFER_SIZE*sizeof(uint8_t));
    saadc_buffer.pointer = 0;
    saadc_buffer.valid_num = 0;
}

static void buffer_in(uint8_t percentage)
{
    saadc_buffer.buffer[saadc_buffer.pointer] = percentage;
    saadc_buffer.pointer++;
    saadc_buffer.pointer%=BATTERY_BUFFER_SIZE;
    if(saadc_buffer.valid_num < BATTERY_BUFFER_SIZE){
        saadc_buffer.valid_num++;
    }
}

static uint8_t buffer_result(void)
{
    uint16_t sum = 0;
    for(uint8_t idx=0; idx<saadc_buffer.valid_num; idx++){
        sum+=saadc_buffer.buffer[idx];
    }
    uint8_t result = sum/saadc_buffer.valid_num;
    return result;
}

static uint8_t trans_voltage_to_percentage(nrf_saadc_value_t voltage)
{
    uint8_t percentage = 255;
    if(voltage > 4100) {
        percentage = 100;
    }
    else if(voltage >= 3335) {
        percentage = 15 + (voltage - 3335) / 9;
    }
    else if(voltage >= 2900) {
        percentage = (voltage - 2900) / 29;
    }
    else if (voltage < 2900) {
        percentage = 0;
    }
    return percentage;
}

static void saadc_event_handler(nrfx_saadc_evt_t const * p_event)
{
    if (p_event->type == NRFX_SAADC_EVT_DONE)
    {
        nrf_saadc_value_t meas_result = p_event->data.done.p_buffer[0];
        //实际电压大约是adc值的4.4倍
        nrf_saadc_value_t voltage = meas_result * 4.4;
        buffer_in(trans_voltage_to_percentage(voltage));
        batt_percentage = buffer_result();
        //NRF_LOG_INFO("meas %d, vol: %d, per: %d",meas_result, voltage, batt_percentage);
        //完成测量后再启动下一次定时更新电量的定时器
        ret_code_t err_code;
        err_code = app_timer_start(voltage_meas_timer, sample_time_ticks, NULL);
        APP_ERROR_CHECK(err_code);
    }
}

static void voltage_meas_timer_handler(void * p_context)
{
    ret_code_t err_code;
    err_code = nrfx_saadc_simple_mode_set((1<<0),
        NRF_SAADC_RESOLUTION_12BIT,
        NRF_SAADC_OVERSAMPLE_DISABLED,
        saadc_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_saadc_buffer_set(saadc_samples, SAADC_CHANNEL_COUNT);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_saadc_mode_trigger();
    APP_ERROR_CHECK(err_code);
}

static void kb_adc_init(void)
{
    ret_code_t err_code;
    //初始化SAADC
    err_code = nrfx_saadc_init(NRFX_SAADC_CONFIG_IRQ_PRIORITY);
    APP_ERROR_CHECK(err_code);
    saadc_channels[0].channel_config.burst = SAADC_CH_CONFIG_BURST_Enabled;
    saadc_channels[0].channel_config.acq_time  = NRF_SAADC_ACQTIME_20US;
    err_code = nrfx_saadc_channels_config(saadc_channels, SAADC_CHANNEL_COUNT);
    APP_ERROR_CHECK(err_code);
    //初始化Timer
    err_code = app_timer_create(&voltage_meas_timer,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                voltage_meas_timer_handler);
    APP_ERROR_CHECK(err_code);
    //初始化sample timer ticks
    sample_time_ticks = APP_TIMER_TICKS(SAADC_SAMPLE_INTERVAL_MS);
}

uint8_t kb_batt_percentage(void)
{
    return batt_percentage;
}

static void kb_event_adc_handler(kb_event_type_t event, void * p_arg)
{
    ret_code_t err_code;
    uint8_t param = (uint32_t)p_arg;
    switch(event)
    {
        case KB_EVT_INIT:
            kb_adc_init();
            NRF_LOG_INFO("Keyboard adc init");
        case KB_EVT_START:
            //启动Timer开启定时测量
            err_code = app_timer_start(voltage_meas_timer, APP_TIMER_TICKS(SAADC_SAMPLE_INTERVAL_MS), NULL);
            APP_ERROR_CHECK(err_code);
            NRF_LOG_INFO("Keyboard adc start");
            break;
        case KB_EVT_POWERSAVE:
            switch(param){
                case KB_POWERSAVE_ENTER:
                    sample_time_ticks = APP_TIMER_TICKS(SAADC_SAMPLE_PS_INTERVAL_MS);
                    break;
                case KB_POWERSAVE_EXIT:
                    sample_time_ticks = APP_TIMER_TICKS(SAADC_SAMPLE_INTERVAL_MS);
                    break;
            }
        break;
        default:
            break;
    }
}

KB_EVT_HANDLER(kb_event_adc_handler);