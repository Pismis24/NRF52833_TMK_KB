#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app_error.h"
#include "ble.h"
#include "ble_hid_descriptor.h"
#include "ble_hids.h"

#include "nrf_queue.h"

#include "ble_svc_hid.h"

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

enum input_report_index {
    INPUT_REP_KBD_INDEX,
#ifdef MOUSEKEY_ENABLE
    INPUT_REP_MOUSE_INDEX,
#endif
#ifdef EXTRAKEY_ENABLE
    INPUT_REP_SYSTEM_INDEX,
    INPUT_REP_CONSUMER_INDEX,
#endif
    INPUT_REP_COUNT
};

enum output_report_index {
    OUTPUT_REP_KBD_INDEX,
    OUTPUT_REP_COUNT
};

#ifndef MOUSEKEY_ENABLE
#define INPUT_REP_MOUSE_INDEX INPUT_REP_INDEX_INVALID
#endif

#ifndef EXTRAKEY_ENABLE
#define INPUT_REP_SYSTEM_INDEX INPUT_REP_INDEX_INVALID
#define INPUT_REP_CONSUMER_INDEX INPUT_REP_INDEX_INVALID
#endif

/**
 * @brief HID Report Index Lookup table
 * 
 * Mapping the internal ID to HID report id
 * 
 */
uint8_t hid_report_map_table[] = {
    INPUT_REP_KBD_INDEX,
    INPUT_REP_MOUSE_INDEX,
    INPUT_REP_SYSTEM_INDEX,
    INPUT_REP_CONSUMER_INDEX
};

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

BLE_HIDS_DEF(m_hids, /**< Structure used to identify the HID service. */
    NRF_SDH_BLE_TOTAL_LINK_COUNT,
    INPUT_REPORT_LEN_KEYBOARD,
#ifdef MOUSEKEY_ENABLE
    INPUT_REPORT_LEN_MOUSE,
#endif
#ifdef EXTRAKEY_ENABLE
    INPUT_REPORT_LEN_SYSTEM,
    INPUT_REPORT_LEN_CONSUMER,
#endif
    OUTPUT_REPORT_LEN_KEYBOARD);

uint8_t keyboard_led_val_ble;
static bool m_in_boot_mode = false; /**< Current protocol mode. */

static void on_hids_evt(ble_hids_t* p_hids, ble_hids_evt_t* p_evt);

/**@brief Function for initializing HID Service.
 */
static void hids_init(ble_srv_error_handler_t err_handler)
{
    ret_code_t err_code;
    ble_hids_init_t hids_init_obj;

    static ble_hids_inp_rep_init_t input_report_array[INPUT_REP_COUNT];
    static ble_hids_outp_rep_init_t output_report_array[OUTPUT_REP_COUNT];

    memset((void*)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t) * INPUT_REP_COUNT);
    memset((void*)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t) * OUTPUT_REP_COUNT);

    // Initialize HID Service
    HID_REP_IN_SETUP(
        input_report_array[INPUT_REP_KBD_INDEX],
        INPUT_REPORT_LEN_KEYBOARD,
        INPUT_REP_KEYBOARD_ID);

    // keyboard led report
    HID_REP_OUT_SETUP(
        output_report_array[OUTPUT_REP_KBD_INDEX],
        OUTPUT_REPORT_LEN_KEYBOARD,
        OUTPUT_REP_KEYBOARD_ID);

#ifdef MOUSEKEY_ENABLE
    HID_REP_IN_SETUP(
        input_report_array[INPUT_REP_MOUSE_INDEX],
        INPUT_REPORT_LEN_MOUSE,
        REPORT_ID_MOUSE);
#endif
#ifdef EXTRAKEY_ENABLE
    // system input report
    HID_REP_IN_SETUP(
        input_report_array[INPUT_REP_SYSTEM_INDEX],
        INPUT_REPORT_LEN_SYSTEM,
        REPORT_ID_SYSTEM);
    // consumer input report
    HID_REP_IN_SETUP(
        input_report_array[INPUT_REP_CONSUMER_INDEX],
        INPUT_REPORT_LEN_CONSUMER,
        REPORT_ID_CONSUMER);
#endif

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler = on_hids_evt;
    hids_init_obj.error_handler = err_handler;
    hids_init_obj.is_kb = true;
    hids_init_obj.is_mouse = false;
    hids_init_obj.inp_rep_count = INPUT_REP_COUNT;
    hids_init_obj.p_inp_rep_array = input_report_array;
    hids_init_obj.outp_rep_count = OUTPUT_REP_COUNT;
    hids_init_obj.p_outp_rep_array = output_report_array;
    hids_init_obj.feature_rep_count = 0;
    hids_init_obj.p_feature_rep_array = NULL;
    hids_init_obj.rep_map.data_len = sizeof(hid_descriptor);
    hids_init_obj.rep_map.p_data = hid_descriptor;
    hids_init_obj.hid_information.bcd_hid = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;
    hids_init_obj.included_services_count = 0;
    hids_init_obj.p_included_services_array = NULL;

    hids_init_obj.rep_map.rd_sec = SEC_JUST_WORKS;
    hids_init_obj.hid_information.rd_sec = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_inp_rep_sec.cccd_wr = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_inp_rep_sec.rd = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_outp_rep_sec.rd = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_outp_rep_sec.wr = SEC_JUST_WORKS;

    hids_init_obj.protocol_mode_rd_sec = SEC_JUST_WORKS;
    hids_init_obj.protocol_mode_wr_sec = SEC_JUST_WORKS;
    hids_init_obj.ctrl_point_wr_sec = SEC_JUST_WORKS;

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}

static uint32_t send_key(ble_hids_t* p_hids,
    uint8_t index,
    uint8_t* pattern,
    uint8_t len)
{
    ret_code_t err_code = NRF_SUCCESS;
    if (m_in_boot_mode) {
        if (index == 0) {
            err_code = ble_hids_boot_kb_inp_rep_send(p_hids,
                len,
                pattern,
                m_conn_handle);
        }
    } else {
        err_code = ble_hids_inp_rep_send(p_hids,
            index,
            len,
            pattern,
            m_conn_handle);
    }
    return err_code;
}

static uint32_t buffer_enqueue(ble_hids_t * p_hids,
    uint8_t report_index,
    uint8_t * p_key_pattern,
    uint16_t pattern_len)
{
    buffer_entry_t element;
    ret_code_t err_code = NRF_SUCCESS;
    //写入
    element.p_instance = p_hids;
    element.p_data = p_key_pattern;
    element.index = report_index;
    element.data_len = pattern_len;
    err_code = nrf_queue_push(&hid_buffer_queue, &element);
    return err_code;
}

static uint32_t buffer_dequeue(bool tx_flag)
{
    buffer_entry_t element;
    ret_code_t err_code = NRF_SUCCESS;
    //判断缓冲是否为空
    if(nrf_queue_is_empty(&hid_buffer_queue)){
        err_code = NRF_ERROR_NOT_FOUND;
    }
    else{
        bool remove_element = true;
        //读取缓冲队列
        err_code = nrf_queue_peek(&hid_buffer_queue, &element);
        //发送
        if(tx_flag) {
            err_code = send_key(element.p_instance,
                element.index,
                element.p_data,
                element.data_len);
            if(err_code == NRF_ERROR_RESOURCES){
                remove_element = false;
            }
        }
        if(remove_element){
            nrf_queue_pop(&hid_buffer_queue, &element);
        }
    }
    return err_code;
}

void keys_send(uint8_t report_id, uint8_t key_pattern_len, uint8_t* p_key_pattern)
{
    ret_code_t err_code;
    if(report_id >= sizeof(hid_report_map_table)){ return; }
    uint8_t report_index = hid_report_map_table[report_id];
    if(report_index == INPUT_REP_INDEX_INVALID) { return; }
    err_code = send_key(&m_hids, report_index, p_key_pattern, key_pattern_len);
    if (err_code == NRF_ERROR_RESOURCES) {
        // Buffer enqueue routine return value is not intentionally checked.
        // Rationale: Its better to have a a few keys missing than have a system
        // reset. Recommendation is to work out most optimal value for
        // MAX_BUFFER_ENTRIES to minimize chances of buffer queue full condition
        UNUSED_VARIABLE(buffer_enqueue(&m_hids, report_index, p_key_pattern, key_pattern_len));
    }

    if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES) && (err_code != NRF_ERROR_BUSY) && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING) && (err_code != NRF_ERROR_FORBIDDEN)) {
        APP_ERROR_HANDLER(err_code);
    }
}

/**@brief Function for handling the HID Report Characteristic Write event.
 *
 * @param[in]   p_evt   HID service event.
 */
static void on_hid_rep_char_write(ble_hids_evt_t* p_evt)
{
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT) {
        ret_code_t err_code;
        uint8_t report_val;
        uint8_t report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REP_KBD_INDEX) {
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
}

/**@brief Function for handling HID events.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service structure.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t* p_hids, ble_hids_evt_t* p_evt)
{
    switch (p_evt->evt_type) {
    case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
        m_in_boot_mode = true;
        break;

    case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
        m_in_boot_mode = false;
        break;

    case BLE_HIDS_EVT_REP_CHAR_WRITE:
        on_hid_rep_char_write(p_evt);
        break;

    case BLE_HIDS_EVT_NOTIF_ENABLED:
        break;

    default:
        // No implementation needed.
        break;
    }
}

static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void hid_init(void)
{
    hids_init(service_error_handler);
}

bool hid_queue_empty(void)
{
    return nrf_queue_is_empty(&hid_buffer_queue);
}

void send_next_hid_buffer_key(void)
{
    buffer_dequeue(true);
}

void clear_buffer(void)
{
    nrf_queue_reset(&hid_buffer_queue);
}