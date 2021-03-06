#include "config.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "fds.h"

#include "kb_storage.h"

#define DIRTY_RECOREDS_THRS 200

static bool volatile m_fds_initialized = false;
static bool volatile m_fds_gc_complete = true;
//写入缓存
static store_data_t data_to_write;
static bool volatile data_to_write_flag = false;

static void storage_evt_handler(fds_evt_t const * p_evt)
{
    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == NRF_SUCCESS)
            {
                m_fds_initialized = true;

            }
            break;
        case FDS_EVT_GC:
        if (p_evt->result == NRF_SUCCESS)
            {
                m_fds_gc_complete = true;

            }
            break;
        default:
            break;
    }
}

static void wait_until_fds_ready(void)
{
    while(!m_fds_initialized)
    {
        (void) sd_app_evt_wait();
    }
}

static void wait_until_fds_gc_done(void)
{
    while(!m_fds_gc_complete){
        (void) sd_app_evt_wait();
    }
}

//初始化存储
void storage_init(void)
{
    ret_code_t err_code;
    err_code = fds_register(storage_evt_handler);
    APP_ERROR_CHECK(err_code);
    err_code = fds_init();
    APP_ERROR_CHECK(err_code);
    wait_until_fds_ready();
    //判断当前有无数据，没有的话写个空的
    store_data_t temp_data;
    if(!storage_read(&temp_data)){
        memset(&temp_data, 0x00, sizeof(temp_data));
        storage_write(temp_data);
    }
    fds_stat_t fds_current_stat;
    err_code = fds_stat(&fds_current_stat);
    NRF_LOG_INFO("Dirty records %d", fds_current_stat.dirty_records);
    if(fds_current_stat.dirty_records >= DIRTY_RECOREDS_THRS){
        NRF_LOG_INFO("Dirty records exceeds thres, start GC");
        m_fds_gc_complete = false;
        fds_gc();
        wait_until_fds_gc_done();
    }
    NRF_LOG_INFO("Storage init");
}

//将data写入存储
void storage_write(store_data_t data)
{
    ret_code_t err_code;
    data_to_write = data;
    // 待写入or更新的数据
    fds_record_t const new_record =
    {
        .file_id           = STORE_FILE,
        .key               = STORE_KEY,
        .data.p_data       = &data_to_write,
        /* The length of a record is always expressed in 4-byte units (words). */
        .data.length_words = (sizeof(data_to_write) + 3) / sizeof(uint32_t),
    };
    // 寻找并定位记录
    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};
    err_code = fds_record_find(STORE_FILE, STORE_KEY, &desc, &tok);
    if(err_code == NRF_SUCCESS){
        // 存在记录，因此使用更新
        err_code = fds_record_update(&desc, &new_record);
        APP_ERROR_CHECK(err_code);
    }
    else{
        // 原本没记录，使用写入
        err_code = fds_record_write(&desc, &new_record);
        APP_ERROR_CHECK(err_code);
    }

}

//将data读出存储
bool storage_read(store_data_t * data)
{
    ret_code_t err_code;
    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};
    //查找记录
    err_code = fds_record_find(STORE_FILE, STORE_KEY, &desc, &tok);
    if(err_code == NRF_SUCCESS){
        //找到后读出
        fds_flash_record_t flash_record = {0};
        err_code = fds_record_open(&desc, &flash_record);
        APP_ERROR_CHECK(err_code);
        memcpy(data, flash_record.p_data, sizeof(store_data_t));
        err_code = fds_record_close(&desc);
        APP_ERROR_CHECK(err_code);
        return true;
    }
    else{
    //找不到返回false
        return false;
    }
}

//删除存储
void storage_del(void)
{
    ret_code_t err_code;
    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};
    err_code = fds_record_find(STORE_FILE, STORE_KEY, &desc, &tok);
    if(err_code == NRF_SUCCESS){
        err_code = fds_record_delete(&desc);
        APP_ERROR_CHECK(err_code);
    }
    else{
        APP_ERROR_CHECK(err_code);
    }
}