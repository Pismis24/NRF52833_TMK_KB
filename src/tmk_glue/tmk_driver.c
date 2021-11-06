//Std Headers
#include <stdint.h>
#include <string.h>
//nRF LOG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//tmk headers
#include "host_driver.h"
#include "host.h"
//nrf
#include "nrfx_timer.h"
//config
#include "config.h"
//protocol
#include "ble_protocol.h"
//event
#include "kb_evt.h"

void keyboard_start(void)
{

    host_set_driver(&ble_driver);
    //启动timer1
    const nrfx_timer_t kbd_task_timer = NRFX_TIMER_INSTANCE(KBD_TASK_TIMER);
    nrfx_timer_enable(&kbd_task_timer);
    NRF_LOG_INFO("keyboard start");
}