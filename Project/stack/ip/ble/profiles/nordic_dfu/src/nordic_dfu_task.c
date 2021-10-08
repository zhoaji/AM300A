/**
 ****************************************************************************************
 *
 * @file nordic_dfu_task.c
 *
 * @brief Service Server Role Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup NORDIC_DFUTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#if BLE_APP_NORDIC_DFU

#include "app.h"
#include "gap.h"
#include "gattc_task.h"
#include "nordic_dfu.h"
#include "nordic_dfu_task.h"
#include "prf_utils.h"
#include "co_utils.h"
#include "mbr.h"
#include "sha256.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "pb.h"
#include "uECC.h"
#include "dfu-cc.pb.h"
#include "peripheral.h"
#include "ke_timer.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

enum
{
    DFU_FIRMWARE_TYPE_SOFTDEVICE    = 0x00, //image
    DFU_FIRMWARE_TYPE_APPLICATION   = 0x01, //reserve
    DFU_FIRMWARE_TYPE_BOOTLOADER    = 0x02, //patch
    DFU_FIRMWARE_TYPE_BL_SD         = 0x03, //patch + image
    DFU_FIRMWARE_TYPE_UNKNOWN       = 0xFF,
};

enum NORDIC_DFU_STATE
{
    NORDIC_DFU_STATE_IDLE,
    NORDIC_DFU_STATE_UPDATE,
    NORDIC_DFU_STATE_SUCCESS,
    NORDIC_DFU_STATE_FAILED,
    NORDIC_DFU_STATE_REBOOT,
};

typedef struct
{
    uint32_t cmd_obj_size;
    uint32_t cmd_obj_offset;
    uint32_t cmd_obj_crc;
    uint32_t data_obj_size;
    uint32_t data_obj_offset;
    uint32_t data_obj_crc;
} dfu_obj_info_t;

typedef struct
{
    uint16_t                  conn_handle;             /**< Handle of the current connection. BLE_CONN_HANDLE_INVALID if not in a connection. */
    uint16_t                  service_handle;          /**< Handle of Nordic DFU Service. */
    uint16_t                  control_point_handles;   /**< Handles related to the DFU control point characteristic. */
    uint16_t                  packet_handles;          /**< Handles related to the DFU packet characteristic. */
    uint16_t                  version_handles;          /**< Handles related to the DFU version characteristic. */
    uint8_t                   uuid_chat_type;               /**< UUID type for Nordic DFU characteristic Base UUID. */
    bool                      is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
} ble_nordic_dfu_t;

struct dfu_rsp_cal_checksum
{
    uint32_t offset;
    uint32_t crc32;
};

struct dfu_rsp_select
{
    uint32_t max_size;
    uint32_t offset;
    uint32_t crc32;
};

typedef struct
{
    uint8_t rsp_code;
    uint8_t opcode;
    uint8_t result;
    uint8_t data_len;
    union
    {
        uint8_t data[1];
        struct dfu_rsp_cal_checksum checksum_data;
        struct dfu_rsp_select select_data;
    };
} dfu_response_t;

/*********************************************************************
 * LOCAL VARIABLES
 */
static dfu_obj_info_t    m_dfu_obj_info;
static uint16_t          m_recv_pkg_prn;
static uint16_t          m_recv_pkg_type;           /**< Receiving package type @ref enum DFU_PKG_TYPE. */
static uint32_t          m_recv_pkg_cnt;            /**< Receiving package count, use for reply confirm CRC. */
static uint32_t          m_new_app_base_addr;       /**< new app location in flash. */
static uint32_t          m_new_app_size;
static uint32_t          m_new_patch_base_addr;     /**< new patch location in flash. */
static uint32_t          m_new_patch_size;
static uint32_t          m_new_cfg_base_addr;       /**< new config location in flash. */
static uint32_t          m_new_cfg_size;
__attribute__((aligned(16))) static uint8_t           m_cmd_data[DFU_COMMAND_OBJ_MAX_SIZE];
static dfu_packet_t      m_cmd_data_packet;
static SHA256_CTX        m_dfu_data_hash_ctx;
static uint8_t           m_dfu_data_hash[SHA256_BLOCK_SIZE];
static uint8_t           m_state;
static uint8_t*          m_init_packet_data_ptr   = 0;
static uint32_t          m_init_packet_data_len   = 0;
static uint8_t           *m_dfu_cache = m_cmd_data; // m_cmd_data also use for cache DFU data, assert()
#if DFU_BUF_SIZE > DFU_COMMAND_OBJ_MAX_SIZE
#error "sizeof(m_dfu_cache) MUST NOT bigger then sizeof(m_cmd_data) if they share the memory."
#endif
static uint32_t          m_dfu_cache_len;
static uint32_t          m_dfu_cache_offset;
static uint32_t          m_dfu_cache_crc;


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static  uint16_t calc_dfu_crc16(uint32_t in_addr, uint32_t in_len);
static  uint32_t calc_dfu_crc32(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc);
static uint8_t dfu_update_cmd_init(uint32_t data_obj_size)
{
    if(data_obj_size <= DFU_COMMAND_OBJ_MAX_SIZE)
    {
        memset(m_cmd_data, 0, data_obj_size);
        return DFU_SUCCESS;
    }
    else
    {
        return DFU_INSUFFICIENT_RESOURCES;
    }
}

static void dfu_update_cmd_data(uint8_t *data, uint16_t len, uint32_t offset)
{
    memcpy(&m_cmd_data[offset], data, len);
}

static void pb_decoding_callback(pb_istream_t *str, uint32_t tag, pb_wire_type_t wire_type, void *iter)
{
    pb_field_iter_t* p_iter = (pb_field_iter_t *) iter;
    // match the beginning of the init command
    if (p_iter->pos->ptr == &dfu_init_command_fields[0])
    {
        uint8_t  * ptr  = (uint8_t *)str->state;
        uint32_t   size = str->bytes_left;
        // remove tag byte
        ptr++;
        size--;
        // store the info in init_packet_data
        m_init_packet_data_ptr = ptr;
        m_init_packet_data_len = size;
    }
}

static uint8_t dfu_update_cmd_finish(void)
{
    extern const uint8_t dfu_public_key[64];
    pb_istream_t pb_stream;
    pb_stream = pb_istream_from_buffer(m_cmd_data, m_dfu_obj_info.cmd_obj_size);
    pb_stream.decoding_callback = pb_decoding_callback;
    if (pb_decode(&pb_stream, dfu_packet_fields, &m_cmd_data_packet))
    {
        SHA256_CTX ctx;
        uint8_t hash[32];
        if(m_cmd_data_packet.has_signed_command && m_cmd_data_packet.signed_command.command.has_init && \
                m_cmd_data_packet.signed_command.command.init.has_hash && \
                m_cmd_data_packet.signed_command.command.init.hash.hash_type == DFU_HASH_TYPE_SHA256 && \
                m_cmd_data_packet.signed_command.signature_type == DFU_SIGNATURE_TYPE_ECDSA_P256_SHA256)
        {
            sha256_init(&ctx);
            sha256_update(&ctx, (BYTE*)m_init_packet_data_ptr, m_init_packet_data_len);
            sha256_final(&ctx, hash);
            int res = uECC_verify(dfu_public_key, hash, SHA256_BLOCK_SIZE, m_cmd_data_packet.signed_command.signature.bytes, uECC_secp256r1());
            dfu_debug("uECC_verify, res = %d\n", res);
            if(res)
            {
                if(m_cmd_data_packet.signed_command.command.init.has_app_size)
                {
                    m_new_cfg_size = m_cmd_data_packet.signed_command.command.init.app_size;
                }
                if(m_cmd_data_packet.signed_command.command.init.has_bl_size)
                {
                    m_new_patch_size = m_cmd_data_packet.signed_command.command.init.bl_size;
                }
                if(m_cmd_data_packet.signed_command.command.init.has_sd_size)
                {
                    m_new_app_size = m_cmd_data_packet.signed_command.command.init.sd_size;
                }
                if(!((m_new_cfg_size > 0 && !(m_new_app_size > 0 || m_new_patch_size > 0)) ||
                        (!(m_new_cfg_size > 0) && (m_new_app_size > 0 || m_new_patch_size > 0))) ||
                        m_new_app_size > MAX_APP_SIZE || m_new_patch_size > MAX_PATCH_SIZE || m_new_cfg_size > MAX_CFG_SIZE
                  )
                {
                    dfu_debug("New app size:%d\nNew patch size:%d\nNew config size:%d\n", m_new_app_size, m_new_patch_size, m_new_cfg_size);
                    return DFU_INVALID_PARAMETER;
                }
                return DFU_SUCCESS;
            }
        }
        else
        {
            return DFU_UNSUPPORTED_TYPE;
        }
    }
    return DFU_INVALID_OBJECT;
}

static uint8_t dfu_update_app_init(uint32_t data_obj_size)
{
    uint32_t part_addr;
    sfs_enable();
    if(m_new_app_size > 0)
    {
        int res = mbr_read_part(PART_TYPE_APP, &part_addr, NULL, NULL);
        if(res>=0)
        {
            if(part_addr >= NEW_APP_BASE_ADDR1 && part_addr < NEW_APP_BASE_ADDR1 + MAX_APP_SIZE)
            {
                m_new_app_base_addr = NEW_APP_BASE_ADDR2;
            }
            else
            {
                m_new_app_base_addr = NEW_APP_BASE_ADDR1;
            }
            dfu_debug("New app address in flash is 0x%08x, size:0x%08x(%d)\n",
                      m_new_app_base_addr, m_new_app_size, m_new_app_size);
            sfs_erase(m_new_app_base_addr, m_new_app_size);
        }
        else
        {
            return DFU_INSUFFICIENT_RESOURCES;
        }
    }
    if(m_new_patch_size > 0)
    {
        part_addr = PART_TYPE_PATCH;
        int res = mbr_read_part(PART_TYPE_PATCH, &part_addr, NULL, NULL);
        if(res == 0 || res == -ENOENT)
        {
            if(part_addr >= NEW_PATCH_BASE_ADDR1 && part_addr < NEW_PATCH_BASE_ADDR1 + MAX_PATCH_SIZE)
            {
                m_new_patch_base_addr = NEW_PATCH_BASE_ADDR2;
            }
            else
            {
                m_new_patch_base_addr = NEW_PATCH_BASE_ADDR1;
            }
            dfu_debug("New patch address in flash is 0x%08x, size:0x%08x(%d)\n",
                      m_new_patch_base_addr, m_new_patch_size, m_new_patch_size);
            sfs_erase(m_new_patch_base_addr, m_new_patch_size);
        }
        else
        {
            return DFU_INSUFFICIENT_RESOURCES;
        }
    }
    if(m_new_cfg_size > 0)
    {
        part_addr = PART_TYPE_CFG;
        int res = mbr_read_part(PART_TYPE_CFG, &part_addr, NULL, NULL);
        if(res == 0 || res == -ENOENT)
        {
            if(part_addr >= NEW_CFG_BASE_ADDR1 && part_addr < NEW_CFG_BASE_ADDR1 + MAX_CFG_SIZE)
            {
                m_new_cfg_base_addr = NEW_CFG_BASE_ADDR2;
            }
            else
            {
                m_new_cfg_base_addr = NEW_CFG_BASE_ADDR1;
            }
            dfu_debug("New config address in flash is 0x%08x, size:0x%08x(%d)\n",
                      m_new_cfg_base_addr, m_new_cfg_size, m_new_cfg_size);
            sfs_erase(m_new_cfg_base_addr, m_new_cfg_size);
        }
        else
        {
            return DFU_INSUFFICIENT_RESOURCES;
        }
    }
    sha256_init(&m_dfu_data_hash_ctx);
    m_dfu_cache_offset = m_dfu_cache_len = m_dfu_cache_crc = 0;
    return DFU_SUCCESS;
}

static void dfu_update_app_data(uint8_t *data, uint16_t len, uint32_t offset)
{
    co_assert(offset == m_dfu_cache_offset + m_dfu_cache_len);
    while(len)
    {
        uint32_t cache_len = MIN(DFU_BUF_SIZE-m_dfu_cache_len, len);
        if(m_new_app_size > 0 && m_new_patch_size > 0 && offset < m_new_app_size)
        {
            cache_len = MIN(cache_len, m_new_app_size - offset);
        }
        memcpy(&m_dfu_cache[m_dfu_cache_len], data, cache_len);
        m_dfu_cache_len += cache_len;
        data += cache_len;
        len -= cache_len;
        offset += cache_len;

        if(m_new_cfg_size > 0)   // update config data
        {
            if(m_dfu_cache_len == DFU_BUF_SIZE ||
                    m_dfu_cache_offset + m_dfu_cache_len == m_new_cfg_size)
            {
                uint32_t write_addr = m_new_cfg_base_addr + m_dfu_cache_offset;
                sfs_enable();
                sfs_write(write_addr, m_dfu_cache, m_dfu_cache_len); //update config in flash
                m_dfu_cache_offset += m_dfu_cache_len;
                m_dfu_cache_crc = calc_dfu_crc32(m_dfu_cache, m_dfu_cache_len, &m_dfu_cache_crc);
                sha256_update(&m_dfu_data_hash_ctx, m_dfu_cache, m_dfu_cache_len);
                m_dfu_cache_len = 0;
            }
        }
        else     // update app & patch data
        {
            co_assert(m_new_app_size > 0 || m_new_patch_size > 0);
            if(m_dfu_cache_offset < m_new_app_size)
            {
                if(m_dfu_cache_len == DFU_BUF_SIZE ||
                        m_dfu_cache_offset + m_dfu_cache_len == m_new_app_size)
                {
                    uint32_t write_addr = m_new_app_base_addr + m_dfu_cache_offset;
                    sfs_enable();
                    sfs_write(write_addr, m_dfu_cache, m_dfu_cache_len); //update app in flash
                    m_dfu_cache_offset += m_dfu_cache_len;
                    m_dfu_cache_crc = calc_dfu_crc32(m_dfu_cache, m_dfu_cache_len, &m_dfu_cache_crc);
                    sha256_update(&m_dfu_data_hash_ctx, m_dfu_cache, m_dfu_cache_len);
                    m_dfu_cache_len = 0;
                }

            }
            else
            {
                if(m_dfu_cache_len == DFU_BUF_SIZE ||
                        (m_dfu_cache_offset - m_new_app_size) + m_dfu_cache_len == m_new_patch_size)
                {
                    uint32_t write_addr = m_new_patch_base_addr + (m_dfu_cache_offset - m_new_app_size);
                    sfs_enable();
                    sfs_write(write_addr, m_dfu_cache, m_dfu_cache_len); //update patch in flash
                    m_dfu_cache_offset += m_dfu_cache_len;
                    m_dfu_cache_crc = calc_dfu_crc32(m_dfu_cache, m_dfu_cache_len, &m_dfu_cache_crc);
                    sha256_update(&m_dfu_data_hash_ctx, m_dfu_cache, m_dfu_cache_len);
                    m_dfu_cache_len = 0;
                }
            }
        }
    }
}

static uint8_t dfu_update_app_finish(void)
{
    dfu_debug("DFU finished.\n");
    m_dfu_cache_crc = 0;
    if(m_new_cfg_size > 0)   // update config data
    {
        for(int i=0; i<m_new_cfg_size; i+=DFU_BUF_SIZE)
        {
            int len = MIN(DFU_BUF_SIZE, m_new_cfg_size-i);
            memset(m_dfu_cache, 0x5F, len);
            sfs_enable();
            sfs_read((m_new_cfg_base_addr+i), (void *)m_dfu_cache, len);
            m_dfu_cache_crc = calc_dfu_crc32(m_dfu_cache, len, &m_dfu_cache_crc);
        }
    }
    else
    {
        if(m_new_app_size > 0)   // update app data
        {
            for(int i=0; i<m_new_app_size; i+=DFU_BUF_SIZE)
            {
                int len = MIN(DFU_BUF_SIZE, m_new_app_size-i);
                memset(m_dfu_cache, 0x5F, len);
                sfs_enable();
                sfs_read((m_new_app_base_addr+i), (void *)m_dfu_cache, len);
                m_dfu_cache_crc = calc_dfu_crc32(m_dfu_cache, len, &m_dfu_cache_crc);
            }
        }
        if(m_new_patch_size > 0)   // update app data
        {
            for(int i=0; i<m_new_patch_size; i+=DFU_BUF_SIZE)
            {
                int len = MIN(DFU_BUF_SIZE, m_new_patch_size-i);
                memset(m_dfu_cache, 0x5F, len);
                sfs_enable();
                sfs_read((m_new_patch_base_addr+i), (void *)m_dfu_cache, len);
                m_dfu_cache_crc = calc_dfu_crc32(m_dfu_cache, len, &m_dfu_cache_crc);
            }
        }
    }
    if(m_dfu_obj_info.data_obj_crc != m_dfu_cache_crc)
    {
        dfu_debug("DFU app data CRC NOT matched!!\n");
        return DFU_INVALID_OBJECT;
    }
    sha256_final(&m_dfu_data_hash_ctx, m_dfu_data_hash);
    if(memcmp(m_dfu_data_hash, m_cmd_data_packet.signed_command.command.init.hash.hash.bytes, SHA256_BLOCK_SIZE))
    {
        dfu_debug("DFU app data hash NOT matched!!\n");
        return DFU_INVALID_OBJECT;
    }
    else
    {
        uint16_t crc16;
        if(m_new_patch_size > 0)
        {
            //Enable patch image
            dfu_debug("Update patch MBR 0x%08x %d.\n",m_new_patch_base_addr, m_new_patch_size);
            crc16 = calc_dfu_crc16(m_new_patch_base_addr, m_new_patch_size);
            if(mbr_write_part(PART_TYPE_PATCH, m_new_patch_base_addr, m_new_patch_size, crc16) < 0)
            {
                return DFU_INSUFFICIENT_RESOURCES;
            }
        }
        if(m_new_app_size > 0)
        {
            //Enable application image
            int err = mbr_validate_app(m_new_app_base_addr, m_new_app_size);
            if(err >= 0)
            {
                dfu_debug("App validate successful. Update app MBR 0x%08x %d\n",m_new_app_base_addr, m_new_app_size);
                uint16_t crc16 = calc_dfu_crc16(m_new_app_base_addr, m_new_app_size);
                if(mbr_write_part(PART_TYPE_APP, m_new_app_base_addr, m_new_app_size, crc16) < 0)
                {
                    return DFU_INSUFFICIENT_RESOURCES;
                }
            }
            else
            {
                dfu_debug("mbr_validate_app read app addr&size failed\n");
                return DFU_INVALID_OBJECT;
            }
        }
        if(m_new_cfg_size > 0)
        {
            //Enable config image
            dfu_debug("Update config MBR 0x%08x %d.\n",m_new_cfg_base_addr, m_new_cfg_size);
            crc16 = calc_dfu_crc16(m_new_cfg_base_addr, m_new_cfg_size);
            if(mbr_write_part(PART_TYPE_CFG, m_new_cfg_base_addr, m_new_cfg_size, crc16) < 0)
            {
                return DFU_INSUFFICIENT_RESOURCES;
            }
        }
        m_state = NORDIC_DFU_STATE_SUCCESS;
        return DFU_SUCCESS;
    }
}


static  uint32_t calc_dfu_crc32(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc)
{
    uint32_t crc;
    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++)
    {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--) crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
    }
    return ~crc;
}

static  uint16_t calc_dfu_crc16(uint32_t in_addr, uint32_t in_len)
{
    uint32_t buffer[256/4];
    uint16_t crc16 = 0;
    uint32_t read_size = sizeof(buffer);
    uint32_t addr = in_addr;
    uint32_t len;

    sfs_enable();

    for(len=0; len<in_len; len+=read_size, addr+=read_size)
    {
        if(read_size > in_len - len)
            read_size = in_len - len;
        sfs_read( addr, (void *)buffer, read_size); //alignment for dual/quad read
        crc16 = co_crc16_ccitt(crc16, buffer, read_size);
    }

    return crc16;
}

static void nordic_dfu_update_prog_indicate(ke_task_id_t const src_id, uint32_t value_now, uint32_t value_max)
{
    // Send NORDIC_DFU_UPDATE_PROG_IND
    
    uint16_t task_app = KE_BUILD_ID(TASK_APP, KE_IDX_GET(src_id));
    struct nordic_dfu_update_prog_ind *ind = KE_MSG_ALLOC(
                NORDIC_DFU_UPDATE_PROG_IND, task_app, src_id, nordic_dfu_update_prog_ind);
    ind->value_now = value_now;
    ind->value_max = value_max;
    ke_msg_send(ind);
}

static void nordic_dfu_update_end_indicate(ke_task_id_t const src_id, uint8_t status)
{
    uint16_t task_app = KE_BUILD_ID(TASK_APP, KE_IDX_GET(src_id));
    struct nordic_dfu_update_end_ind *ind = KE_MSG_ALLOC(
            NORDIC_DFU_UPDATE_END_IND, task_app, src_id, nordic_dfu_update_end_ind);

    ind->status = status;
    ind->new_app_base_addr = m_new_app_base_addr;
    ind->new_app_size = m_new_app_size;
    ind->new_patch_base_addr = m_new_patch_base_addr;
    ind->new_patch_size = m_new_patch_size;
    ind->new_cfg_base_addr = m_new_cfg_base_addr;
    ind->new_cfg_size = m_new_cfg_size;
    ke_msg_send(ind);
}

static void dfu_response(dfu_response_t *dfu_rsp_data, ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
#define DFU_RSP_VALUE_OFFSET  3
#define DFU_RSP_VALUE_MAX_LEN 16

    uint16_t rsp_len = dfu_rsp_data->data_len + DFU_RSP_VALUE_OFFSET;
    uint8_t rsp_data[DFU_RSP_VALUE_MAX_LEN] =
    {
        dfu_rsp_data->rsp_code,
        dfu_rsp_data->opcode,
        dfu_rsp_data->result
    };
    memcpy(&rsp_data[DFU_RSP_VALUE_OFFSET], dfu_rsp_data->data, dfu_rsp_data->data_len);
    struct gattc_send_evt_cmd* notify = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                        dest_id, src_id,
                                        gattc_send_evt_cmd, rsp_len);
    notify->operation   = GATTC_NOTIFY;
    notify->handle      = nordic_dfu_get_att_handle(NORDIC_DFU_IDX_CTRL_VAL);
    notify->length      = rsp_len;
    memcpy(notify->value, rsp_data, rsp_len);
    ke_msg_send(notify);
    dfu_debug_array_ex("DFU Response", rsp_data, rsp_len);

    if(dfu_rsp_data->result != DFU_SUCCESS)
    {
        m_state = NORDIC_DFU_STATE_FAILED;
        nordic_dfu_update_end_indicate(src_id, dfu_rsp_data->result);
    }
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref NORDIC_DFU_ENABLE_REQ message.
 * The handler enables the 'Profile' Server Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int nordic_dfu_enable_req_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_att_info_cfm * cfm;
    cfm->handle = param->handle;
    cfm->length = 0;
    cfm->status = ATT_ERR_WRITE_NOT_PERMITTED;
    ke_msg_send(cfm);
    return KE_MSG_CONSUMED;
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_WRITE_REQ_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    //uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = nordic_dfu_get_att_idx(param->handle, &att_idx);
    //dfu_debug("%s handle:%d(idx:%d).\n", __func__, param->handle, att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        //Send write response
        cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
        cfm->handle = param->handle;
        cfm->status = status;
        ke_msg_send(cfm);

        struct gattc_write_req_ind *req = (struct gattc_write_req_ind*) param;
        uint8_t opcode = req->value[0];
        uint8_t *data = req->value;
        uint16_t len = req->length;
        dfu_response_t dfu_rsp_data = {DFU_CTRL_RESPONSE, opcode, DFU_SUCCESS, 0};
        switch(att_idx)
        {
            case NORDIC_DFU_IDX_CTRL_VAL:
            {
                switch(opcode)
                {
                    case DFU_CTRL_CREATE:
                        dfu_debug_array_ex("DFU CREATE", data, len);
                        if(DFU_CTRL_CREATE_REQ_LEN == len)
                        {
                            uint8_t type = data[1];
                            uint32_t size = co_read32p(&data[2]);
                            if(type == DFU_PKG_TYPE_COMMAND_OBJ)
                            {
                                m_dfu_obj_info.cmd_obj_size = size;
                                m_dfu_obj_info.cmd_obj_offset = 0;
                                m_dfu_obj_info.cmd_obj_crc = 0;
                                dfu_rsp_data.result = dfu_update_cmd_init(size);
                            }
                            else if(type == DFU_PKG_TYPE_DATA_OBJ)
                            {
                                m_dfu_obj_info.data_obj_size = size;
                                m_dfu_obj_info.data_obj_offset = 0;
                                m_dfu_obj_info.data_obj_crc = 0;
                                dfu_rsp_data.result = dfu_update_app_init(size);
                            }
                            else
                            {
                                dfu_rsp_data.result = DFU_UNSUPPORTED_TYPE;
                            }
                        }
                        else
                        {
                            dfu_rsp_data.result = DFU_INVALID_PARAMETER;
                        }
                        m_recv_pkg_cnt = 0;
                        dfu_response(&dfu_rsp_data, dest_id, src_id);
                        break;
                    case DFU_CTRL_SET_PRN:
                        dfu_debug_array_ex("DFU SET PRN ", data, len);
                        if(DFU_CTRL_SET_PRN_REQ_LEN == len)
                        {
                            m_recv_pkg_prn = data[1] + (data[2] << 8);
                        }
                        else
                        {
                            dfu_rsp_data.result = DFU_INVALID_PARAMETER;
                        }
                        m_recv_pkg_cnt = 0;
                        dfu_response(&dfu_rsp_data, dest_id, src_id);
                        break;
                    case DFU_CTRL_CAL_CHECKSUM:
                        dfu_debug_array_ex("DFU CHECKSUM", data, len);
                        if(DFU_CTRL_CAL_CHECKSUM_REQ_LEN == len)
                        {
                            if(m_recv_pkg_type == DFU_PKG_TYPE_COMMAND_OBJ)
                            {
                                dfu_rsp_data.checksum_data.crc32 = m_dfu_obj_info.cmd_obj_crc;
                                dfu_rsp_data.checksum_data.offset = m_dfu_obj_info.cmd_obj_offset;
                            }
                            else if(m_recv_pkg_type == DFU_PKG_TYPE_DATA_OBJ)
                            {
                                dfu_rsp_data.checksum_data.crc32 = m_dfu_obj_info.data_obj_crc;
                                dfu_rsp_data.checksum_data.offset = m_dfu_obj_info.data_obj_offset;
                            }
                            else
                            {
                                dfu_rsp_data.result = DFU_UNSUPPORTED_TYPE;
                            }
                            dfu_rsp_data.data_len = sizeof(struct dfu_rsp_cal_checksum);
                        }
                        else
                        {
                            dfu_rsp_data.result = DFU_INVALID_PARAMETER;
                        }
                        dfu_response(&dfu_rsp_data, dest_id, src_id);
                        break;
                    case DFU_CTRL_EXECTUE:
                        dfu_debug_array_ex("DFU EXECTUE ", data, len);
                        if(DFU_CTRL_EXECTUE_REQ_LEN == len)
                        {
                            if(m_recv_pkg_type == DFU_PKG_TYPE_COMMAND_OBJ)
                            {
                                dfu_debug("DFU All Data Size:%d/%d\n", m_dfu_obj_info.cmd_obj_offset, m_dfu_obj_info.cmd_obj_size);
                                dfu_rsp_data.result = dfu_update_cmd_finish();
                                dfu_response(&dfu_rsp_data, dest_id, src_id);
                                break;
                            }
                            else if(m_recv_pkg_type == DFU_PKG_TYPE_DATA_OBJ)
                            {
                                dfu_debug("DFU All Data Size:%d/%d\n", m_dfu_obj_info.data_obj_offset, m_dfu_obj_info.data_obj_size);
                                dfu_rsp_data.result = dfu_update_app_finish();
                                dfu_response(&dfu_rsp_data, dest_id, src_id);
                                break;
                            }
                        }
                        else
                        {
                            dfu_rsp_data.result = DFU_INVALID_PARAMETER;
                        }
                        dfu_response(&dfu_rsp_data, dest_id, src_id);
                        break;
                    case DFU_CTRL_SELECT:
                        dfu_debug_array_ex("DFU SELECT  ", data, len);
                        m_dfu_obj_info.cmd_obj_crc = 0; // retransmission not valid due to flash write cache
                        m_dfu_obj_info.data_obj_crc = 0;
                        if(DFU_CTRL_SELECT_REQ_LEN == len)
                        {
                            m_recv_pkg_type = data[1];
                            if(m_recv_pkg_type == DFU_PKG_TYPE_COMMAND_OBJ)
                            {
                                uint16_t task_app = KE_BUILD_ID(TASK_APP, KE_IDX_GET(src_id));
                                ke_msg_send_basic(NORDIC_DFU_UPDATE_START_IND, task_app, dest_id);
                                dfu_rsp_data.data_len = sizeof(struct dfu_rsp_select);
                                dfu_rsp_data.select_data.crc32 = m_dfu_obj_info.cmd_obj_crc;
                                dfu_rsp_data.select_data.max_size = DFU_COMMAND_OBJ_MAX_SIZE;
                                dfu_rsp_data.select_data.offset = m_dfu_obj_info.cmd_obj_offset;
                            }
                            else if(m_recv_pkg_type == DFU_PKG_TYPE_DATA_OBJ)
                            {
                                m_dfu_obj_info.data_obj_offset = m_dfu_cache_offset;
                                m_dfu_obj_info.data_obj_crc = m_dfu_cache_crc;
                                dfu_rsp_data.data_len = sizeof(struct dfu_rsp_select);
                                dfu_rsp_data.select_data.crc32 = m_dfu_obj_info.data_obj_crc;
                                dfu_rsp_data.select_data.max_size = DFU_DATA_OBJ_MAX_SIZE;
                                dfu_rsp_data.select_data.offset = m_dfu_obj_info.data_obj_offset;
                                m_dfu_cache_len = 0;
                                m_state = NORDIC_DFU_STATE_UPDATE;
                            }
                            else
                            {
                                dfu_rsp_data.result = DFU_UNSUPPORTED_TYPE;
                            }
                        }
                        else
                        {
                            dfu_rsp_data.result = DFU_INVALID_PARAMETER;
                        }
                        dfu_response(&dfu_rsp_data, dest_id, src_id);
                        break;
                    default:
                        dfu_debug("Opcode 0x%02x not support\n", opcode);
                        dfu_rsp_data.result = DFU_OPCODE_NOT_SUPPORT;
                        dfu_response(&dfu_rsp_data, dest_id, src_id);
                        break;
                }
            }
            break;
            case NORDIC_DFU_IDX_PKG_VAL:
            {
                uint32_t *crc32, *offset;
                if(m_recv_pkg_type == DFU_PKG_TYPE_COMMAND_OBJ)
                {
                    crc32 = &m_dfu_obj_info.cmd_obj_crc;
                    offset = &m_dfu_obj_info.cmd_obj_offset;
                    dfu_update_cmd_data(data, len, *offset);
                }
                else if(m_recv_pkg_type == DFU_PKG_TYPE_DATA_OBJ)
                {
                    crc32 = &m_dfu_obj_info.data_obj_crc;
                    offset = &m_dfu_obj_info.data_obj_offset;
                    dfu_update_app_data(data, len, *offset);
                    nordic_dfu_update_prog_indicate(dest_id, len + *offset, m_new_app_size + m_new_cfg_size);
                }
                else
                {
                    break;
                }
                (*crc32) = calc_dfu_crc32(data, len, crc32);
                (*offset) += len;
                m_recv_pkg_cnt++;
                //dfu_debug("PKG:%d\n", *offset);
                if(m_recv_pkg_prn != 0 && !(m_recv_pkg_cnt % m_recv_pkg_prn))
                {
                    dfu_response_t dfu_rsp_data =
                    {
                        DFU_CTRL_RESPONSE,
                        DFU_CTRL_CAL_CHECKSUM,
                        DFU_SUCCESS,
                        sizeof(struct dfu_rsp_cal_checksum),
                        .checksum_data.crc32 = *crc32,
                        .checksum_data.offset = *offset,
                    };
                    dfu_response(&dfu_rsp_data, dest_id, src_id);
                }
            }
            break;
            case NORDIC_DFU_IDX_VERSION_VAL:
            {
                dfu_debug_array_ex("NORDIC_DFU_IDX_VERSION_VAL", req->value, req->length);
            }
            break;
            case NORDIC_DFU_IDX_CTRL_DESC:
            {
            }
            break;
        }
    }
    return KE_MSG_CONSUMED;
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_READ_REQ_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t att_idx = 0;
    // retrieve handle information
    uint8_t status = nordic_dfu_get_att_idx(param->handle, &att_idx);
    //dfu_debug("%s handle:%d(idx:%d).\n", __func__, param->handle, att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct gattc_read_cfm *cfm;
        if(att_idx == NORDIC_DFU_IDX_VERSION_VAL)
        {
            extern const uint8_t dfu_version[DFU_VER_SIZE];
            cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, DFU_VER_SIZE)
                  cfm->handle = param->handle;
            cfm->length = DFU_VER_SIZE;
            memcpy(cfm->value, dfu_version, DFU_VER_SIZE);
            ke_msg_send(cfm);
        }
        else if(att_idx == NORDIC_DFU_IDX_CTRL_DESC)
        {
            extern const uint8_t dfu_version[DFU_VER_SIZE];
            cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, 2)
                  cfm->handle = param->handle;
            cfm->length = 2;
            cfm->value[0] = 0x01; // Notification is always enabled
            ke_msg_send(cfm);
        }
        else
        {
            cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, 0)
                  cfm->handle = param->handle;
            cfm->length = 0;
            ke_msg_send(cfm);
        }
    }
    return KE_MSG_CONSUMED;
}
/**
 ****************************************************************************************
 * @brief Handles @ref GATTC_CMP_EVT for GATTC_NOTIFY message meaning that Measurement
 * notification has been correctly sent to peer device (but not confirmed by peer device).
 * *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                   ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    //uint8_t conidx = KE_IDX_GET(src_id);
    //dfu_debug("%s msgid=0x%04x, operation=%d, status=%d\n", __func__, msgid, param->operation, param->status);
    if(m_state == NORDIC_DFU_STATE_SUCCESS && ((struct gattc_cmp_evt*)param)->operation == GATTC_NOTIFY)
    {
        ke_timer_set(NORDIC_DFU_END_TIMER, dest_id, 5); // Delay 50ms then send DFU end event.
    }
    return (KE_MSG_CONSUMED);
}

__STATIC int nordic_dfu_end_timer_handler(ke_msg_id_t const msgid,
                                               void const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
    if(m_state == NORDIC_DFU_STATE_SUCCESS)
    {
        nordic_dfu_update_end_indicate(dest_id, DFU_SUCCESS);
        m_state = NORDIC_DFU_STATE_IDLE;
        m_dfu_obj_info.cmd_obj_size = 0;
        m_dfu_obj_info.cmd_obj_offset = 0;
        m_dfu_obj_info.cmd_obj_crc = 0;
        m_dfu_obj_info.data_obj_size = 0;
        m_dfu_obj_info.data_obj_offset = 0;
        m_dfu_obj_info.data_obj_crc = 0;
    }
    return (KE_MSG_CONSUMED);
}

__STATIC int nordic_dfu_default_handler(ke_msg_id_t const msgid,  void const *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    dfu_debug("%s msgid=0x%04x\n", __func__, msgid);
    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(nordic_dfu)
{
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t) nordic_dfu_default_handler},
    {NORDIC_DFU_ENABLE_REQ,         (ke_msg_func_t) nordic_dfu_enable_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
    {NORDIC_DFU_END_TIMER,          (ke_msg_func_t) nordic_dfu_end_timer_handler},
};

void nordic_dfu_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct nordic_dfu_env_tag *nordic_dfu_env = PRF_ENV_GET(NORDIC_DFU, nordic_dfu);

    task_desc->msg_handler_tab = nordic_dfu_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(nordic_dfu_msg_handler_tab);
    task_desc->state           = nordic_dfu_env->state;
    task_desc->idx_max         = NORDIC_DFU_IDX_MAX;
}

#endif //BLE_APP_NORDIC_DFU

/// @} NORDIC_DFUTASK
