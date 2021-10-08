/**
 ****************************************************************************************
 *
 * @file onmicro_dfu_task.c
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
 * @addtogroup ONMICRO_DFUTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#if BLE_APP_ONMICRO_DFU

#include "app.h"
#include "gap.h"
#include "gattc_task.h"
#include "dfu.h"
#include "dfu_task.h"
#include "prf_utils.h"
#include "co_utils.h"
#include "mbr.h"
#include "peripheral.h"
#include "ke_timer.h"
#include "onmicro_dfu.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
 
static void dfu_response(dfu_response_t *dfu_rsp_data, ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
    struct gattc_send_evt_cmd* notify = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                        dest_id, src_id,
                                        gattc_send_evt_cmd, dfu_rsp_data->length);
    notify->operation   = GATTC_NOTIFY;
    notify->handle      = onmicro_dfu_get_att_handle(ONMICRO_DFU_IDX_CTRL_VAL);
    notify->length      = dfu_rsp_data->length;
    memcpy(notify->value, (uint8_t*)&dfu_rsp_data->rsp_code, dfu_rsp_data->length);
    ke_msg_send(notify);
}

static void dfu_response_version(dfu_version_t *dfu_version, ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
    struct gattc_send_evt_cmd* notify = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                        dest_id, src_id,
                                        gattc_send_evt_cmd, sizeof(struct dfu_version_data));
    notify->operation   = GATTC_NOTIFY;
    notify->handle      = onmicro_dfu_get_att_handle(ONMICRO_DFU_IDX_VERSION_VAL);
    notify->length      = sizeof(struct dfu_version_data);
    memcpy(notify->value, (uint8_t*)&dfu_version->version_data, sizeof(struct dfu_version_data));
    ke_msg_send(notify);
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
    struct gattc_att_info_cfm * cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);;
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
    uint8_t status = onmicro_dfu_get_att_idx(param->handle, &att_idx);
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
        uint8_t *data = req->value;
        uint16_t len = req->length;
        dfu_response_t dfu_rsp_data = {0};
        if(att_idx == ONMICRO_DFU_IDX_CTRL_VAL){
            dfu_write_cmd(data, len, &dfu_rsp_data);
            dfu_response(&dfu_rsp_data, dest_id, src_id);
        }else if(att_idx == ONMICRO_DFU_IDX_PKG_VAL){
            dfu_write_data(data, len, &dfu_rsp_data);
            if(dfu_rsp_data.length != DFU_RESP_SIZE_NO_DATA){
                dfu_response(&dfu_rsp_data, dest_id, src_id);
            }
        }else if(att_idx == ONMICRO_DFU_IDX_VERSION_VAL){
            dfu_version_t version;
            uint32_t cmd = 0xFFFFFFFF;
            if(len == 4){
                cmd = (data[0]<<24)+(data[1]<<16)+(data[2]<<8)+(data[3]);
            }else if(len == 1){
                cmd = data[0];
            }
            dfu_write_version_char(cmd, &version);
            dfu_response_version(&version, dest_id, src_id);
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
    uint8_t status = onmicro_dfu_get_att_idx(param->handle, &att_idx);
    //dfu_debug("%s handle:%d(idx:%d).\n", __func__, param->handle, att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct gattc_read_cfm *cfm;
        if(att_idx == ONMICRO_DFU_IDX_VERSION_VAL)
        {
            dfu_version_t version;
            dfu_read_version_char(&version);
            cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, version.desc_data.rsp_length);
            cfm->handle = param->handle;
            cfm->length = version.desc_data.rsp_length;
            memcpy(cfm->value, version.desc_data.app_describe, cfm->length);
            ke_msg_send(cfm);
        }
        else if(att_idx == ONMICRO_DFU_IDX_CTRL_DESC)
        {
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
    return (KE_MSG_CONSUMED);
}

__STATIC int onmicro_dfu_default_handler(ke_msg_id_t const msgid,  void const *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}


__STATIC int gattc_mtu_changed_handler(ke_msg_id_t const msgid,  void const *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}
__STATIC int onmicro_dfu_end_timer_handler(ke_msg_id_t const msgid, void const *param,
                                            ke_task_id_t const dest_id, ke_task_id_t const src_id);

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(onmicro_dfu)
{
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t) onmicro_dfu_default_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
    {GATTC_MTU_CHANGED_IND,         (ke_msg_func_t) gattc_mtu_changed_handler},
    {ONMICRO_DFU_END_TIMER,         (ke_msg_func_t) onmicro_dfu_end_timer_handler},
};

void onmicro_dfu_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct onmicro_dfu_env_tag *onmicro_dfu_env = PRF_ENV_GET(ONMICRO_DFU, onmicro_dfu);

    task_desc->msg_handler_tab = onmicro_dfu_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(onmicro_dfu_msg_handler_tab);
    task_desc->state           = onmicro_dfu_env->state;
    task_desc->idx_max         = ONMICRO_DFU_IDX_MAX;
}

extern void app_onmicro_dfu_update_start_ind_handler(uint8_t status, void *p);
extern void app_onmicro_dfu_update_prog_ind_handler(uint8_t status, void *p);
extern void app_onmicro_dfu_update_end_ind_handler(uint8_t status, void *p);

static void dfu_begin_ind_handler(uint8_t status, void *p)
{
    app_onmicro_dfu_update_start_ind_handler(status, p);
}

static void dfu_prog_ind_handler(uint8_t status, void *p)
{
    app_onmicro_dfu_update_prog_ind_handler(status, p);
}

static uint8_t end_status;
static uint16_t ctrl_flag;
__STATIC int onmicro_dfu_end_timer_handler(ke_msg_id_t const msgid, void const *param,
                                            ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    app_onmicro_dfu_update_end_ind_handler(end_status, &ctrl_flag);
    return (KE_MSG_CONSUMED);
}
static void dfu_end_ind_handler(uint8_t status, void *p)
{
    end_status = status;
    ctrl_flag = *(uint16_t*)p;
    ke_timer_set(ONMICRO_DFU_END_TIMER, prf_get_task_from_id(TASK_ID_ONMICRO_DFU), 100);
}

const dfu_cb_itf_t dfu_cb_itf = {
    dfu_begin_ind_handler,
    dfu_prog_ind_handler,
    dfu_end_ind_handler,
};
#endif //BLE_APP_ONMICRO_DFU

/// @} ONMICRO_DFUTASK
