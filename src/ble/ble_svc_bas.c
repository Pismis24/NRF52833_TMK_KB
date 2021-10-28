//C basic headers
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
//nRF LOG headers
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//Error handler
#include "app_error.h"
#include "ble_err.h"
//nRF Timer headers
#include "app_timer.h"
//nRF BLE Bas
#include "ble_bas.h"
//config
#include "config.h"
//Self Header
#include "ble_svc_bas.h"
//batt
#include "kb_adc.h"

#define BATTERY_REPORT_INTERVAL APP_TIMER_TICKS(10000) 

BLE_BAS_DEF(m_bas); /**< Structure used to identify the battery service. */
APP_TIMER_DEF(bas_upd_timer);

static void bas_upd_handler(void * p_context)
{
    ret_code_t err_code;
    uint8_t battery_level = kb_batt_percentage();
    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY) && (err_code != NRF_ERROR_RESOURCES) && (err_code != NRF_ERROR_FORBIDDEN) && (err_code != NRF_ERROR_INVALID_STATE) && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
        APP_ERROR_HANDLER(err_code);
    }
}

/*brief Function for initializing Battery Service 
  and update battery 
 */
void bas_init(void)
{
    ret_code_t err_code;
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler = NULL;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref = NULL;
    bas_init_obj.initial_batt_level = 100;

    bas_init_obj.bl_rd_sec = SEC_JUST_WORKS;
    bas_init_obj.bl_cccd_wr_sec = SEC_JUST_WORKS;
    bas_init_obj.bl_report_rd_sec = SEC_JUST_WORKS;

    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&bas_upd_timer,
                                APP_TIMER_MODE_REPEATED,
                                bas_upd_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(bas_upd_timer, BATTERY_REPORT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
