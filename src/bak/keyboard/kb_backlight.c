#include "config.h"

#ifdef BACKLIGHT_ENABLE

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"

#include "nrfx_pwm.h"
#include "nrf_gpio.h"

#include "backlight.h"

#include "kb_backlight.h"

#include "kb_evt.h"

static nrfx_pwm_t m_pwm0 = NRFX_PWM_INSTANCE(BACKLIGHT_PWM_INSTANCE);

static const nrfx_pwm_config_t pwm_config = 
{
    .output_pins = 
    {
        BACKLIGHT_PIN,                  //预先设定为输出PWM的GPIO
        NRFX_PWM_PIN_NOT_USED, 
        NRFX_PWM_PIN_NOT_USED,
        NRFX_PWM_PIN_NOT_USED  
    },
    .irq_priority = APP_IRQ_PRIORITY_LOWEST, //最低优先级
    .base_clock   = NRF_PWM_CLK_1MHz,       //PWM时钟频率设置为1MHz pwm周期为时钟频率除以计数最大值
    .count_mode   = NRF_PWM_MODE_UP,        //向上计数模式
    .top_value    = PWM_MAX_COUNT,          //计数最大值
    .load_mode    = NRF_PWM_LOAD_COMMON,    //通用装载模式
    .step_mode    = NRF_PWM_STEP_AUTO       //序列中的周期自动推进
};

static nrf_pwm_values_common_t duty_values[BACKLIGHT_LEVELS + 1];

static void backlight_pwm_init(void)
{
    //Setting duty values
    nrf_pwm_values_common_t step = (PWM_MAX_COUNT - PWM_TOP_LUM) / BACKLIGHT_LEVELS;
    for(uint8_t i = 0; i <= BACKLIGHT_LEVELS; i++)
    {
        duty_values[i] = PWM_MAX_COUNT - step * i;
    }
    ret_code_t err_code;
    err_code = nrfx_pwm_init(&m_pwm0, &pwm_config, NULL);
    APP_ERROR_CHECK(err_code);
    //active pulldowm res
    nrf_gpio_cfg(
        BACKLIGHT_PIN,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_PULLDOWN,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
    APP_ERROR_CHECK(err_code);
}




static void pwm_backlight_set(uint8_t backlight_level)
{
    nrfx_pwm_stop(&m_pwm0, true);
    if(backlight_level > 0){
        nrf_pwm_sequence_t const pwm_seq = {
            .values.p_common = &duty_values[backlight_level],
            .length = 1,
            .repeats = 0,
            .end_delay = 0,
        };
        nrfx_pwm_simple_playback(&m_pwm0, &pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
    }
}

void backlight_set(uint8_t level)
{
    pwm_backlight_set(level);
    NRF_LOG_INFO("backlight level %d", level);
}

void backlight_event_handler(enum kb_event event)
{
    switch(event)
    {
        case KB_EVT_INIT:
            backlight_pwm_init();
            break;
        case KB_EVT_SLEEP:
            nrfx_pwm_uninit(&m_pwm0);
            nrf_gpio_cfg_default(BACKLIGHT_PIN);
            break;
        default:
            break;
    }
}

KB_EVENT_HANDLER(backlight_event_handler);

#endif