//C basic headers
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

//Settings
#define DEVICE_NAME                         "nRF52833_Keyboard"
#define DEVICE_APPEARANCE                   BLE_APPEARANCE_GENERIC_HID

#define APP_BLE_OBSERVER_PRIO               3                                          /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG                1                                          /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(10, UNIT_1_25_MS)           /**< Minimum connection interval (10 ms) */
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(50, UNIT_1_25_MS)            /**< Maximum connection interval (50 ms). */
#define SLAVE_LATENCY                       3                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(1000, UNIT_10_MS)             /**< Connection supervisory timeout (1000 ms). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY      APP_TIMER_TICKS(5000)                      /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(20000)                     /**< Time between each call to sd_ble_gap_conn_param_update after the first call (20 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        5                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_ADV_FAST_INTERVAL               0x0028                                     /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL               0x0320                                     /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 0.5 seconds). */
#define APP_ADV_FAST_DURATION               6000                                       /**< The advertising duration of fast advertising in units of 10 milliseconds. */
#define APP_ADV_SLOW_DURATION               18000                                      /**< The advertising duration of slow advertising in units of 10 milliseconds. */

#define SEC_PARAM_BOND                      1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      0                                          /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS                  0                                          /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                           0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


void ble_init(void);
uint16_t m_conn_handle_state(void);
void advertising_start(bool erase_bonds);
