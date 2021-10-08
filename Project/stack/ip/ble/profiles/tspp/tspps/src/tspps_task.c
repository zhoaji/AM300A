/**
 ****************************************************************************************
 *
 * @file tspps_task.c
 *
 * @brief transport Task Implementation.
 *
 * Copyright (C) Huntersun 2018-2028
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup TSPPSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_TSPP_SERVER)
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"
#include "co.h"
#include "tspps.h"
#include "tspps_task.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

__STATIC void tspp_server_send_error(uint16_t operation, uint8_t status, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct tspp_server_error_evt *evt;
    //struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);
    //uint8_t conidx = KE_IDX_GET(src_id);
    
    evt = KE_MSG_ALLOC(TSPP_SERVER_ERROR,
            src_id,//prf_dst_task_get(&(tspp_server_env->prf_env), conidx),
            dest_id,
            tspp_server_error_evt);
    evt->operation = operation;
    evt->status = status;

    ke_msg_send(evt);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TSPP_SERVER_ENABLE_REQ message.
 * The handler enables the 'Profile' Server Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tspp_server_enable_req_handler(ke_msg_id_t const msgid,
        struct tspp_server_enable_req const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);

    // check state of the task
    if(state == TSPPS_IDLE)
    {
        struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
                || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct tspp_server_enable_rsp* rsp = KE_MSG_ALLOC(TSPP_SERVER_ENABLE_RSP, src_id,
                    dest_id, tspp_server_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            ke_msg_send(rsp);
        }
        else
        {
            tspp_server_env->ntf_cfg[param->conidx] = param->ntf_cfg;
        }
    }

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
__STATIC int tspp_server_gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint8_t att_idx = 0;
    // retrieve handle information
    uint8_t status = tspp_server_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(att_idx == TSPPS_IDX_UPLOAD_NTF_CFG)
        {
            // CCC attribute length = 2
            cfm->length = sizeof(uint16_t);
        }
        // not expected request
        else
        {
            cfm->length = 0;
            status = ATT_ERR_WRITE_NOT_PERMITTED;
        }
    }

    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
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
__STATIC int tspp_server_gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = tspp_server_get_att_idx(param->handle, &att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);

        // Only update configuration if value for stop or notification enable
        if (att_idx == TSPPS_IDX_UPLOAD_NTF_CFG)
        {
            // Extract value before check
            uint16_t ntf_cfg = co_read16p(&param->value[0]);
            if((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF) || (ntf_cfg == PRF_CLI_START_IND))
            {
                // Conserve information in environment
                tspp_server_env->ntf_cfg[conidx] = ntf_cfg;
            }
            // Inform APP of configuration change
            struct tspp_server_ntf_cfg_ind * ind = KE_MSG_ALLOC(TSPP_SERVER_NTF_CFG_IND,
                    prf_dst_task_get(&(tspp_server_env->prf_env), conidx), KE_BUILD_ID(dest_id, conidx),
                    tspp_server_ntf_cfg_ind);
            ind->conidx = conidx;
            ind->ntf_cfg = tspp_server_env->ntf_cfg[conidx];

            ke_msg_send(ind);
            //tspp_server_send_error(TSPP_SERVER_SEND_NTF_CMD, (uint8_t)TSPP_SERVER_ERROR_NO_CONN, dest_id, prf_dst_task_get(&(tspp_server_env->prf_env), conidx));
        }
        else if (att_idx == TSPPS_IDX_REV1_VAL || att_idx == TSPPS_IDX_REV2_VAL)
        {
            //log_debug("Offset:%2d. ", param->offset);
            //log_debug_array_ex("write data", param->value, param->length);
            struct tspp_server_write_ind * ind = KE_MSG_ALLOC_DYN(TSPP_SERVER_WRITE_IND,
                    prf_dst_task_get(&(tspp_server_env->prf_env), conidx), dest_id,
                    tspp_server_write_ind, param->length);
            ind->conidx = conidx;
            ind->handle = param->handle;
            ind->offset = param->offset;
            ind->length = param->length;
            memcpy(ind->value, param->value,param->length);

            ke_msg_send(ind);
        }
        else
        {
            status = PRF_APP_ERROR;
        }

    }

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    cfm->handle = param->handle;
    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
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
__STATIC int tspp_server_gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = tspp_server_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);
    //log_debug("%s handle:%d(idx:%d).\n", __func__, param->handle, att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == TSPPS_IDX_UPLOAD_NTF_CFG)
        {
            length = sizeof(uint16_t);
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }

    //Send write response
    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
    cfm->handle = param->handle;
    cfm->status = status;
    cfm->length = length;

    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == TSPPS_IDX_UPLOAD_NTF_CFG)
        {
            cfm->value[0] = tspp_server_env->ntf_cfg[conidx]; //Characteristic value
        }
    }

    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

__STATIC int tspp_server_send_data_to_gattc(ke_task_id_t const dest_id, uint8_t conidx)
{
    uint16_t att_handle = tspp_server_get_att_handle(TSPPS_IDX_UPLOAD_VAL);
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);

    uint16_t default_len,len;
    uint8_t pdata[GAP_MAX_LE_MTU];
    #if (BLE_TSPP_SEND_TYPE == 1)
    default_len = tspp_server_env->perfect_once_tx_length[conidx];
    len = co_fifo_out(&(tspp_server_env->send_fifo[conidx]), pdata, 20);
    #elif (BLE_TSPP_SEND_TYPE == 2)
    len = co_fifo_out(&(tspp_server_env->send_fifo[conidx]), (uint8_t*)&default_len, 2);
    if(len <=0) {
        ke_state_set(dest_id, TSPPS_CONNECTED);
        return (KE_MSG_CONSUMED);
    }
    default_len = co_min(default_len, tspp_server_env->perfect_once_tx_length[conidx]);
    len = co_fifo_out(&(tspp_server_env->send_fifo[conidx]), pdata, default_len);
    #else // BLE_TSPP_SEND_TYPE == 3
    default_len = tspp_server_env->perfect_once_tx_length[conidx];
    len = co_fifo_out(&(tspp_server_env->send_fifo[conidx]), pdata, default_len);
    #endif
    //printf("len=%d,default_len=%d\n", len, default_len);
    if (len >0) {
        struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                                        KE_BUILD_ID(TASK_GATTC, conidx), 
                                                        prf_get_task_from_id(KE_BUILD_ID(TASK_ID_TSPPS, conidx)), 
                gattc_send_evt_cmd, len);
        if(tspp_server_env->ntf_cfg[conidx] == PRF_CLI_START_NTF){
            req->operation = GATTC_NOTIFY;
        }else{
            req->operation = GATTC_INDICATE;
        }

        req->handle    = att_handle;
        req->length    = len;
        memcpy(req->value, pdata, req->length);
        // Send the event
        ke_msg_send(req);
        ke_state_set(dest_id, TSPPS_BUSY);
    }
    else {
        ke_state_set(dest_id, TSPPS_CONNECTED);
    }

    // Send the indication
    
    return (KE_MSG_CONSUMED);

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
__STATIC int tspp_server_gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);

    struct tspp_server_buffer_empty_evt *evt;
    uint8_t state = ke_state_get(dest_id);
    if (state == TSPPS_IDLE || state > TSPPS_BUSY) {
        return (KE_MSG_CONSUMED);
    }
    if(!(tspp_server_env->ntf_cfg[conidx] == PRF_CLI_START_NTF || tspp_server_env->ntf_cfg[conidx] == PRF_CLI_START_IND)){
        return (KE_MSG_CONSUMED);
    }
    //log_debug("send done(%d)\n", conidx);
    if (co_fifo_is_empty(&(tspp_server_env->send_fifo[conidx])))
    {
        // send fifo empty
        ke_state_set(dest_id, TSPPS_CONNECTED);

        evt = KE_MSG_ALLOC(TSPP_SERVER_BUFFER_EMPTY,
                prf_dst_task_get(&(tspp_server_env->prf_env), conidx),
                dest_id,
                tspp_server_buffer_empty_evt);
        evt->operation = param->operation;
        evt->status = param->status;

        ke_msg_send(evt);

        return (KE_MSG_CONSUMED);

    }
    switch (param->operation)
    {
        case (GATTC_NOTIFY):
            {
                tspp_server_send_data_to_gattc(dest_id, conidx);
            } 
            break;
        default:
            ke_state_set(dest_id, TSPPS_CONNECTED);
            break;
    }
    
    return (KE_MSG_CONSUMED);
}

__STATIC int tspp_server_send_ntf_cmd_handler(ke_msg_id_t const msgid,  struct tspp_server_send_ntf_cmd const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    uint8_t state = ke_state_get(dest_id);
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);

    if (state == TSPPS_IDLE || state > TSPPS_BUSY) {
        tspp_server_send_error(TSPP_SERVER_SEND_NTF_CMD, (uint8_t)TSPP_SERVER_ERROR_NO_CONN, dest_id, src_id);
        return (KE_MSG_CONSUMED);
    }

    if(!(tspp_server_env->ntf_cfg[conidx] == PRF_CLI_START_NTF || tspp_server_env->ntf_cfg[conidx] == PRF_CLI_START_IND)){
        tspp_server_send_error(TSPP_SERVER_SEND_NTF_CMD, (uint8_t)TSPP_SERVER_ERROR_NO_NOTI, dest_id, src_id);
        return (KE_MSG_CONSUMED);
    }
    if(co_fifo_is_full(&(tspp_server_env->send_fifo[conidx])) 
      || co_fifo_avail(&(tspp_server_env->send_fifo[conidx]))< (param->length+2)){
        // send app full message
        tspp_server_send_error(TSPP_SERVER_SEND_NTF_CMD, (uint8_t)TSPP_SERVER_ERROR_NO_BUFF, dest_id, src_id);
        struct tspp_server_buffer_full_evt * evt = KE_MSG_ALLOC(TSPP_SERVER_BUFFER_FULL,
                prf_dst_task_get(&(tspp_server_env->prf_env), conidx),
                dest_id,
                tspp_server_buffer_full_evt);

        evt->operation = (uint8_t)TSPP_SERVER_SEND_NTF_CMD;
        evt->status = (uint8_t)GAP_ERR_INSUFF_RESOURCES;

        ke_msg_send(evt);
        return (KE_MSG_CONSUMED);
    }
    #if (BLE_TSPP_SEND_TYPE == 2)
    co_fifo_in(&(tspp_server_env->send_fifo[conidx]), (uint8_t*)&param->length, 2);
    #endif
    co_fifo_in(&(tspp_server_env->send_fifo[conidx]), (uint8_t*)param->value, param->length);
    
    if (state == TSPPS_BUSY) {
        return (KE_MSG_CONSUMED);
    }

    return tspp_server_send_data_to_gattc(dest_id, conidx);
}

static int tspp_server_gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid, struct gattc_mtu_changed_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    //log_debug("mtu=%d\n", param->mtu);
    uint8_t conidx = KE_IDX_GET(src_id);
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);
    tspp_server_env->mtu[conidx] = param->mtu;
    return KE_MSG_CONSUMED;
}

__STATIC int tspp_server_default_handler(ke_msg_id_t const msgid,  void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    //log_debug("%s msgid=0x%04x\n", __func__, msgid);
    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(tspps)
{
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t) tspp_server_default_handler},
    {TSPP_SERVER_ENABLE_REQ,        (ke_msg_func_t) tspp_server_enable_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) tspp_server_gattc_att_info_req_ind_handler},
    {TSPP_SERVER_SEND_NTF_CMD,      (ke_msg_func_t) tspp_server_send_ntf_cmd_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) tspp_server_gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) tspp_server_gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) tspp_server_gattc_cmp_evt_handler},
    {GATTC_MTU_CHANGED_IND,         (ke_msg_func_t) tspp_server_gattc_mtu_changed_ind_handler},
};

void tspp_server_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct tspps_env_tag *tspps_env = PRF_ENV_GET(TSPPS, tspps);

    task_desc->msg_handler_tab = tspps_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(tspps_msg_handler_tab);
    task_desc->state           = tspps_env->state;
    task_desc->idx_max         = TSPPS_IDX_MAX;
}

uint8_t tspp_server_check_fifo_full(uint8_t conidx)
{
    struct tspps_env_tag *tspps_env = PRF_ENV_GET(TSPPS, tspps);

    return co_fifo_is_full(&(tspps_env->send_fifo[conidx]));
}

uint32_t tspp_server_get_free_size(uint8_t conidx)
{
    struct tspps_env_tag *tspps_env = PRF_ENV_GET(TSPPS, tspps);
    return co_fifo_avail(&(tspps_env->send_fifo[conidx]));
}
#endif /* #if (BLE_TSPP_SERVER) */

/// @} TSPPSTASK

