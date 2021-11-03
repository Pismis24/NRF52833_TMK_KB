#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"
#include "ble.h"
#include "ble_hid_descriptor.h"
#include "ble_hids.h"

#include "nrf_queue.h"

#include "ble_svc_hid.h"

#define OUTPUT_REPORT_INDEX                 0                                          /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN               1                                          /**< Maximum length of Output Report. */
#define INPUT_REPORT_KEYS_INDEX             0                                          /**< Index of Input Report. */
#define INPUT_REP_REF_ID                    0                                          /**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_REF_ID                   0                                          /**< Id of reference to Keyboard Output Report. */
#define FEATURE_REP_REF_ID                  0                                          /**< ID of reference to Keyboard Feature Report. */
#define FEATURE_REPORT_MAX_LEN              2                                          /**< Maximum length of Feature Report. */
#define FEATURE_REPORT_INDEX                0                                          /**< Index of Feature Report. */

#define MAX_BUFFER_ENTRIES                  5                                          /**< Number of elements that can be enqueued */

#define INPUT_REPORT_KEYS_MAX_LEN           8                                          /**< Maximum length of the Input Report characteristic. */
#define MAX_KEYS_IN_ONE_REPORT              (INPUT_REPORT_KEYS_MAX_LEN - SCAN_CODE_POS)/**< Maximum number of key presses that can be sent in one Input Report. */


#define MODIFIER_KEY_POS                    0                                          /**< Position of the modifier byte in the Input Report. */
#define SCAN_CODE_POS                       2                                          /**< The start position of the key scan code in a HID Report. */

#define MAX_BUFFER_ENTRIES 5 /**< Number of elements that can be enqueued */
#define BASE_USB_HID_SPEC_VERSION 0x0101 /**< Version number of base USB HID Specification implemented by this application. */

#define INPUT_REP_KEYBOARD_ID 0x7f /**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_KEYBOARD_ID 0x7f /**< Id of reference to Keyboard Output Report. */

#define INPUT_REPORT_LEN_KEYBOARD 8 /**< Maximum length of the Input Report characteristic. */
#define OUTPUT_REPORT_LEN_KEYBOARD 1 /**< Maximum length of Output Report. */
#define INPUT_REPORT_LEN_MOUSE 5
#define INPUT_REPORT_LEN_SYSTEM 2
#define INPUT_REPORT_LEN_CONSUMER 2

#define INPUT_REP_INDEX_INVALID 0xFF /** Invalid index **/

#ifndef MOUSEKEY_ENABLE
#define INPUT_REP_MOUSE_INDEX INPUT_REP_INDEX_INVALID
#endif

#ifndef EXTRAKEY_ENABLE
#define INPUT_REP_SYSTEM_INDEX INPUT_REP_INDEX_INVALID
#define INPUT_REP_CONSUMER_INDEX INPUT_REP_INDEX_INVALID
#endif


//data structure of key buffer
typedef struct hid_key_buffer {
    uint8_t index;
    uint8_t data_len;
    uint8_t* p_data;
    ble_hids_t * p_instance;
} buffer_entry_t;

//using queue as buffer
//使用overflow模式，当buffer内装满后自动丢弃最早进入的
NRF_QUEUE_DEF(buffer_entry_t, hid_buffer_queue, MAX_BUFFER_ENTRIES, NRF_QUEUE_MODE_OVERFLOW);

BLE_HIDS_DEF(m_hids,                                                /**< Structure used to identify the HID service. */
             NRF_SDH_BLE_TOTAL_LINK_COUNT,
             INPUT_REPORT_KEYS_MAX_LEN,
             OUTPUT_REPORT_MAX_LEN,
             FEATURE_REPORT_MAX_LEN);

uint8_t keyboard_led_val_ble;
static bool m_in_boot_mode = false; /**< Current protocol mode. */

static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt);

/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing HID Service.
 */
/**@brief Function for initializing HID Service.
 */
static void hids_init(void)
{
    ret_code_t                    err_code;
    ble_hids_init_t               hids_init_obj;
    ble_hids_inp_rep_init_t     * p_input_report;
    ble_hids_outp_rep_init_t    * p_output_report;
    ble_hids_feature_rep_init_t * p_feature_report;
    uint8_t                       hid_info_flags;

    static ble_hids_inp_rep_init_t     input_report_array[1];
    static ble_hids_outp_rep_init_t    output_report_array[1];
    static ble_hids_feature_rep_init_t feature_report_array[1];
    static uint8_t                     report_map_data[] =
    {
        0x05, 0x01,       // Usage Page (Generic Desktop)
        0x09, 0x06,       // Usage (Keyboard)
        0xA1, 0x01,       // Collection (Application)
        0x05, 0x07,       // Usage Page (Key Codes)
        0x19, 0xe0,       // Usage Minimum (224)
        0x29, 0xe7,       // Usage Maximum (231)
        0x15, 0x00,       // Logical Minimum (0)
        0x25, 0x01,       // Logical Maximum (1)
        0x75, 0x01,       // Report Size (1)
        0x95, 0x08,       // Report Count (8)
        0x81, 0x02,       // Input (Data, Variable, Absolute)

        0x95, 0x01,       // Report Count (1)
        0x75, 0x08,       // Report Size (8)
        0x81, 0x01,       // Input (Constant) reserved byte(1)

        0x95, 0x05,       // Report Count (5)
        0x75, 0x01,       // Report Size (1)
        0x05, 0x08,       // Usage Page (Page# for LEDs)
        0x19, 0x01,       // Usage Minimum (1)
        0x29, 0x05,       // Usage Maximum (5)
        0x91, 0x02,       // Output (Data, Variable, Absolute), Led report
        0x95, 0x01,       // Report Count (1)
        0x75, 0x03,       // Report Size (3)
        0x91, 0x01,       // Output (Data, Variable, Absolute), Led report padding

        0x95, 0x06,       // Report Count (6)
        0x75, 0x08,       // Report Size (8)
        0x15, 0x00,       // Logical Minimum (0)
        0x25, 0x65,       // Logical Maximum (101)
        0x05, 0x07,       // Usage Page (Key codes)
        0x19, 0x00,       // Usage Minimum (0)
        0x29, 0x65,       // Usage Maximum (101)
        0x81, 0x00,       // Input (Data, Array) Key array(6 bytes)

        0x09, 0x05,       // Usage (Vendor Defined)
        0x15, 0x00,       // Logical Minimum (0)
        0x26, 0xFF, 0x00, // Logical Maximum (255)
        0x75, 0x08,       // Report Size (8 bit)
        0x95, 0x02,       // Report Count (2)
        0xB1, 0x02,       // Feature (Data, Variable, Absolute)

        0xC0              // End Collection (Application)
    };

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));
    memset((void *)feature_report_array, 0, sizeof(ble_hids_feature_rep_init_t));

    // Initialize HID Service
    p_input_report                      = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    p_input_report->max_len             = INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = INPUT_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
    p_input_report->sec.wr      = SEC_JUST_WORKS;
    p_input_report->sec.rd      = SEC_JUST_WORKS;

    p_output_report                      = &output_report_array[OUTPUT_REPORT_INDEX];
    p_output_report->max_len             = OUTPUT_REPORT_MAX_LEN;
    p_output_report->rep_ref.report_id   = OUTPUT_REP_REF_ID;
    p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    p_output_report->sec.wr = SEC_JUST_WORKS;
    p_output_report->sec.rd = SEC_JUST_WORKS;

    p_feature_report                      = &feature_report_array[FEATURE_REPORT_INDEX];
    p_feature_report->max_len             = FEATURE_REPORT_MAX_LEN;
    p_feature_report->rep_ref.report_id   = FEATURE_REP_REF_ID;
    p_feature_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_FEATURE;

    p_feature_report->sec.rd              = SEC_JUST_WORKS;
    p_feature_report->sec.wr              = SEC_JUST_WORKS;

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler                    = on_hids_evt;
    hids_init_obj.error_handler                  = service_error_handler;
    hids_init_obj.is_kb                          = true;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = 1;
    hids_init_obj.p_inp_rep_array                = input_report_array;
    hids_init_obj.outp_rep_count                 = 1;
    hids_init_obj.p_outp_rep_array               = output_report_array;
    hids_init_obj.feature_rep_count              = 1;
    hids_init_obj.p_feature_rep_array            = feature_report_array;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags          = hid_info_flags;
    hids_init_obj.included_services_count        = 0;
    hids_init_obj.p_included_services_array      = NULL;

    hids_init_obj.rep_map.rd_sec         = SEC_JUST_WORKS;
    hids_init_obj.hid_information.rd_sec = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_inp_rep_sec.cccd_wr = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_inp_rep_sec.rd      = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_outp_rep_sec.rd = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_outp_rep_sec.wr = SEC_JUST_WORKS;

    hids_init_obj.protocol_mode_rd_sec = SEC_JUST_WORKS;
    hids_init_obj.protocol_mode_wr_sec = SEC_JUST_WORKS;
    hids_init_obj.ctrl_point_wr_sec    = SEC_JUST_WORKS;

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}

static uint32_t send_key(ble_hids_t* p_hids,
    uint8_t index,
    uint8_t* pattern,
    uint8_t len)
{
    ret_code_t err_code = NRF_SUCCESS;
    if (!m_in_boot_mode){
        if(index != 0){ return NRF_SUCCESS; }
        err_code = ble_hids_inp_rep_send(p_hids,
            INPUT_REPORT_KEYS_INDEX,
            len,
            pattern,
            m_conn_handle);
        }
    else{
        err_code = ble_hids_boot_kb_inp_rep_send(p_hids,
            len,
            pattern,
            m_conn_handle);
        }
    return err_code;
}

void send_next_buffer(void)
{
    buffer_entry_t entry;
    ret_code_t err_code = NRF_SUCCESS;
    //判断缓冲是否为空
    if(nrf_queue_is_empty(&hid_buffer_queue)){
        return;
    }
    else{
        nrf_queue_peek(&hid_buffer_queue, &entry);
        err_code = send_key(entry.p_instance, entry.index, entry.p_data, entry.data_len);
        if(err_code == NRF_SUCCESS){
            nrf_queue_pop(&hid_buffer_queue, &entry);
            NRF_LOG_INFO("key queue out");
        }
        else if(
            (err_code != NRF_SUCCESS) 
            && (err_code != NRF_ERROR_INVALID_STATE) 
            && (err_code != NRF_ERROR_RESOURCES) 
            && (err_code != NRF_ERROR_BUSY) 
            && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING) 
            && (err_code != NRF_ERROR_FORBIDDEN)
        ){
            APP_ERROR_HANDLER(err_code);
        }
    }
}

void keys_send(uint8_t report_id, uint8_t key_pattern_len, uint8_t* p_key_pattern)
{
    ret_code_t err_code;
    uint8_t report_index;
    bool queue_flag = false;
    if(!nrf_queue_is_empty(&hid_buffer_queue)){
        send_next_buffer();
        queue_flag = true;
    }
    else{
        if (report_id != 0) {return;}
        
        report_index = 0;

        err_code = send_key(&m_hids, report_index, p_key_pattern, key_pattern_len);
        if(err_code == NRF_ERROR_RESOURCES){
            queue_flag = true;
        }
        else if(
            (err_code != NRF_SUCCESS) 
            && (err_code != NRF_ERROR_INVALID_STATE) 
            && (err_code != NRF_ERROR_BUSY) 
            && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING) 
            && (err_code != NRF_ERROR_FORBIDDEN)
        ){
            APP_ERROR_HANDLER(err_code);
        }
    }
    if(queue_flag){
        buffer_entry_t const entry = {
            .p_instance = &m_hids,
            .index = report_index,
            .p_data = p_key_pattern,
            .data_len = key_pattern_len,
        };
        nrf_queue_push(&hid_buffer_queue, &entry);
        NRF_LOG_INFO("key in queue");
    }
    
}

/**@brief Function for handling the HID Report Characteristic Write event.
 *
 * @param[in]   p_evt   HID service event.
 */
static void on_hid_rep_char_write(ble_hids_evt_t* p_evt)
{
    NRF_LOG_INFO("report type: %d, report_index: %d", p_evt->params.char_write.char_id.rep_type, p_evt->params.char_write.char_id.rep_index);
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT) {
        ret_code_t err_code;
        uint8_t report_val;
        uint8_t report_index = p_evt->params.char_write.char_id.rep_index;

            err_code = ble_hids_outp_rep_get(&m_hids,
                report_index,
                OUTPUT_REPORT_LEN_KEYBOARD,
                0,
                m_conn_handle,
                &report_val);

            if (err_code == NRF_SUCCESS) {
                keyboard_led_val_ble = report_val;
            }
        
    }
}

/**@brief Function for handling HID events.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service structure.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt)
{
    switch (p_evt->evt_type) {
    case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
        m_in_boot_mode = true;
        break;

    case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
        m_in_boot_mode = false;
        break;

    case BLE_HIDS_EVT_REP_CHAR_WRITE:
        NRF_LOG_INFO("REP CHAR WRITE");
        on_hid_rep_char_write(p_evt);
        break;

    case BLE_HIDS_EVT_NOTIF_ENABLED:
        break;

    default:
        // No implementation needed.
        break;
    }
    NRF_LOG_INFO("HID EVENT");
}

void hid_init(void)
{
    nrf_queue_reset(&hid_buffer_queue);
    hids_init();
}

void clear_buffer(void)
{
    nrf_queue_reset(&hid_buffer_queue);
}