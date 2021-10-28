#include "config.h"

#include <stdint.h>
#include <stdbool.h>

#include "app_error.h"

#include "nrfx_saadc.h"

#include "app_timer.h"

#include "kb_adc.h"

APP_TIMER_DEF(voltage_meas_timer);

static uint8_t batt_percentage = 100;
static volatile bool saadc_is_ready = true;
static nrf_saadc_value_t saadc_samples[SAADC_CHANNEL_COUNT];
static nrfx_saadc_channel_t saadc_channels[SAADC_CHANNEL_COUNT] = {NRFX_SAADC_DEFAULT_CHANNEL_SE(VOLTAGE_SOURCE, 0)};

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
        //使用VDDH输入，输入值是VDDH电压的1/5
        //NRFX_SAADC_DEFAULT_CHANNEL_SE设置增益为1/6
        //参考电压0.6V
        //无论设置精度为多少，输出总为10位
        //1 / (1/6) * 0.6 / 1024 * 1000
        nrf_saadc_value_t voltage = meas_result * 5 * 3.5;
        batt_percentage = trans_voltage_to_percentage(voltage);
        saadc_is_ready = true;
    }
}

static void voltage_meas_timer_handler(void * p_context)
{
    if(saadc_is_ready)
    {
        ret_code_t err_code;
        err_code = nrfx_saadc_simple_mode_set(
            (1<<0),
            NRF_SAADC_RESOLUTION_12BIT,
            NRF_SAADC_OVERSAMPLE_DISABLED,
            saadc_event_handler
            );
        APP_ERROR_CHECK(err_code);
        err_code = nrfx_saadc_buffer_set(saadc_samples, SAADC_CHANNEL_COUNT);
        APP_ERROR_CHECK(err_code);
        err_code = nrfx_saadc_mode_trigger();
        APP_ERROR_CHECK(err_code);
        saadc_is_ready = false;
    }
}

void kb_adc_init(void)
{
    ret_code_t err_code;
    //初始化SAADC
    err_code = nrfx_saadc_init(NRFX_SAADC_CONFIG_IRQ_PRIORITY);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_saadc_channels_config(saadc_channels, SAADC_CHANNEL_COUNT);
    APP_ERROR_CHECK(err_code);
    //初始化Timer
    err_code = app_timer_create(&voltage_meas_timer,
                                APP_TIMER_MODE_REPEATED,
                                voltage_meas_timer_handler);
    APP_ERROR_CHECK(err_code);
    //启动Timer开启定时测量
    err_code = app_timer_start(voltage_meas_timer, APP_TIMER_TICKS(SAADC_SAMPLE_INTERVAL_MS), NULL);
    APP_ERROR_CHECK(err_code);
}

uint8_t kb_voltage_percentage(void)
{
    return batt_percentage;
}