/**
 ****************************************************************************************
 *
 * @file atvvs_task.c
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
 * @addtogroup ATVVSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_ATVV_SERVER)
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"
#include "co.h"
#include "atvvs.h"
#include "atvvs_task.h"

/// atvv server error id
enum atvv_server_audio_type
{
    ATVV_SERVER_AUDIO_TYPE_FIRST      = 0,
    ATVV_SERVER_AUDIO_TYPE_CONTI      = 1,
    ATVV_SERVER_AUDIO_TYPE_LAST       = 2,
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

__STATIC void atvv_server_send_error(uint16_t operation, uint8_t status, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct atvv_server_error_evt *evt;
    //struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    //uint8_t conidx = KE_IDX_GET(src_id);
    
    evt = KE_MSG_ALLOC(ATVV_SERVER_ERROR,
            src_id,//prf_dst_task_get(&(atvv_server_env->prf_env), conidx),
            dest_id,
            atvv_server_error_evt);
    evt->operation = operation;
    evt->status = status;

    ke_msg_send(evt);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ATVV_SERVER_ENABLE_REQ message.
 * The handler enables the 'Profile' Server Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int atvv_server_enable_req_handler(ke_msg_id_t const msgid,
        struct atvv_server_enable_req const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);

    if(param->en) {
        // check state of the task
        if(state == ATVVS_IDLE)
        {
            // Check provided values
            if((param->conidx > BLE_CONNECTION_MAX)
                    || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
            {
                // an error occurs, trigg it.
                struct atvv_server_enable_rsp* rsp = KE_MSG_ALLOC(ATVV_SERVER_ENABLE_RSP, src_id,
                        dest_id, atvv_server_enable_rsp);
                rsp->conidx = param->conidx;
                rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
                ke_msg_send(rsp);
            }
            else
            {
                atvv_server_env->ntf_cfg[param->conidx][0] = param->ntf_cfg;
                atvv_server_env->ntf_cfg[param->conidx][1] = param->ntf_cfg;
            }
        }
    }
    else
    {
        ke_state_set(dest_id, ATVVS_IDLE);
        co_fifo_reset(&(atvv_server_env->send_fifo[param->conidx]));
        atvv_server_env->sent_num = 0;
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
__STATIC int atvv_server_gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint8_t att_idx = 0;
    // retrieve handle information
    uint8_t status = atvv_server_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(att_idx == ATVVS_IDX_READ_NTF_CFG ||att_idx == ATVVS_IDX_CONTROL_NTF_CFG)
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
__STATIC int atvv_server_gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = atvv_server_get_att_idx(param->handle, &att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);

        // Only update configuration if value for stop or notification enable
        if (att_idx == ATVVS_IDX_READ_NTF_CFG || att_idx == ATVVS_IDX_CONTROL_NTF_CFG )
        {
            // Extract value before check
            uint16_t ntf_cfg = co_read16p(&param->value[0]);
            if((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF) || (ntf_cfg == PRF_CLI_START_IND))
            {
                // Conserve information in environment
                if(att_idx == ATVVS_IDX_READ_NTF_CFG)
                {
                    atvv_server_env->ntf_cfg[conidx][0] = ntf_cfg;
                }
                else
                {
                    atvv_server_env->ntf_cfg[conidx][1] = ntf_cfg;
                }
            }
            // Inform APP of configuration change
            struct atvv_server_ntf_cfg_ind * ind = KE_MSG_ALLOC(ATVV_SERVER_NTF_CFG_IND,
                    prf_dst_task_get(&(atvv_server_env->prf_env), conidx), KE_BUILD_ID(dest_id, conidx),
                    atvv_server_ntf_cfg_ind);
            ind->conidx = conidx;
            ind->ntf_cfg = ntf_cfg;
            if(att_idx == ATVVS_IDX_READ_NTF_CFG)
            {
                ind->type = 0;
            }
            else
            {
                ind->type = 1;
            }

            ke_msg_send(ind);
            //atvv_server_send_error(ATVV_SERVER_SEND_NTF_CMD, (uint8_t)ATVV_SERVER_ERROR_NO_CONN, dest_id, prf_dst_task_get(&(atvv_server_env->prf_env), conidx));
        }
        else if (att_idx == ATVVS_IDX_WRITE_VAL)
        {
            //log_debug("Offset:%2d. ", param->offset);
            //log_debug_array_ex("write data", param->value, param->length);
            struct atvv_server_write_ind * ind = KE_MSG_ALLOC_DYN(ATVV_SERVER_WRITE_IND,
                    prf_dst_task_get(&(atvv_server_env->prf_env), conidx), dest_id,
                    atvv_server_write_ind, param->length);
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
__STATIC int atvv_server_gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = atvv_server_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    //log_debug("%s handle:%d(idx:%d).\n", __func__, param->handle, att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == ATVVS_IDX_READ_NTF_CFG || att_idx == ATVVS_IDX_CONTROL_NTF_CFG)
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
        if (att_idx == ATVVS_IDX_READ_NTF_CFG)
        {
            cfm->value[0] = atvv_server_env->ntf_cfg[conidx][0]; //Characteristic value
        }
        else if (att_idx == ATVVS_IDX_CONTROL_NTF_CFG)
        {
            cfm->value[0] = atvv_server_env->ntf_cfg[conidx][1]; //Characteristic value
        }
    }

    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

__STATIC int atvv_server_send_data_to_gattc(ke_task_id_t const dest_id, uint8_t conidx)
{
    uint16_t att_handle = atvv_server_get_att_handle(ATVVS_IDX_READ_VAL);
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    
    uint8_t len;
    uint8_t pdata[GAP_MAX_LE_MTU];
    uint8_t head[2];
    
    co_fifo_out(&(atvv_server_env->send_fifo[conidx]), &head[0], 2);
    len = co_fifo_out(&(atvv_server_env->send_fifo[conidx]), pdata, head[1]);
    if(head[0] == ATVV_SERVER_AUDIO_TYPE_FIRST)
    {
        atvv_server_env->send_seq = (uint16_t)((pdata[0]<<8) | pdata[1]);
    }
    if (len >0) {
        struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                                        KE_BUILD_ID(TASK_GATTC, conidx), 
                                                        prf_get_task_from_id(KE_BUILD_ID(TASK_ID_ATVVS, conidx)), 
                                                        gattc_send_evt_cmd, len);

        if(atvv_server_env->ntf_cfg[conidx][0] == PRF_CLI_START_NTF){
            req->operation = GATTC_NOTIFY;
        }else{
            req->operation = GATTC_INDICATE;
        }
        
        req->handle    = att_handle;
        req->length    = len;
        memcpy(req->value, pdata, req->length);
        // Send the event
        ke_msg_send(req);
        ke_state_set(dest_id, ATVVS_BUSY);
        atvv_server_env->sent_num++;
    }
    else {
        ke_state_set(dest_id, ATVVS_CONNECTED);
    }
    
    //send audio sync packet
    if(head[0] == ATVV_SERVER_AUDIO_TYPE_LAST)
    {
        uint16_t temp = atvv_server_env->send_seq + 1;
        uint16_t hdl = atvv_server_get_att_handle(ATVVS_IDX_CONTROL_VAL);
        uint8_t buf[3] = {0x0A};
        buf[1] = temp >> 8;
        buf[2] = temp & 0xFF;
        struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                                    KE_BUILD_ID(TASK_GATTC, conidx), 
                                                    prf_get_task_from_id(KE_BUILD_ID(TASK_ID_ATVVS, conidx)), 
                                                    gattc_send_evt_cmd, 3);
        req->operation = GATTC_NOTIFY;
        req->handle    = hdl;
        req->length    = 3;
        memcpy(req->value, buf, 3);
        // Send the event
        ke_msg_send(req);
        ke_state_set(dest_id, ATVVS_BUSY);
        atvv_server_env->sent_num++;
    }
    //log_debug("%x\r\n", atvv_server_env->sent_num);
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
__STATIC int atvv_server_gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);

    struct atvv_server_buffer_empty_evt *evt;
    uint8_t state = ke_state_get(dest_id);
    if (state == ATVVS_IDLE || state > ATVVS_BUSY) {
        return (KE_MSG_CONSUMED);
    }
    //log_debug("%x,%x, %x,%x,\r\n", param->status, param->operation, atvv_server_env->sent_num, l2cm_get_nb_buffer_available() );
    if(param->status != GAP_ERR_NO_ERROR || param->operation != GATTC_NOTIFY)
    {
        return (KE_MSG_CONSUMED);
    }
    else if(atvv_server_env->sent_num > 0) {
        atvv_server_env->sent_num--;
    }
    
    if(atvv_server_env->sent_num >=1)
    {
        return (KE_MSG_CONSUMED);
    }
    
    if(!(atvv_server_env->ntf_cfg[conidx][0] == PRF_CLI_START_NTF || atvv_server_env->ntf_cfg[conidx][0] == PRF_CLI_START_IND
        || atvv_server_env->ntf_cfg[conidx][1] == PRF_CLI_START_NTF || atvv_server_env->ntf_cfg[conidx][1] == PRF_CLI_START_IND)){
        return (KE_MSG_CONSUMED);
    }
    //log_debug("send done(%d)\n", conidx);
    if (co_fifo_is_empty(&(atvv_server_env->send_fifo[conidx])))
    {
        // send fifo empty
        ke_state_set(dest_id, ATVVS_CONNECTED);

        evt = KE_MSG_ALLOC(ATVV_SERVER_BUFFER_EMPTY,
                prf_dst_task_get(&(atvv_server_env->prf_env), conidx),
                dest_id,
                atvv_server_buffer_empty_evt);
        evt->operation = param->operation;
        evt->status = param->status;

        ke_msg_send(evt);

        return (KE_MSG_CONSUMED);

    }
    switch (param->operation)
    {
        case (GATTC_NOTIFY):
            {
                atvv_server_send_data_to_gattc(dest_id, conidx);
            } 
            break;
        default:
            ke_state_set(dest_id, ATVVS_CONNECTED);
            break;
    }
    
    return (KE_MSG_CONSUMED);
}

__STATIC int atvv_server_send_ntf_cmd_handler(ke_msg_id_t const msgid,  struct atvv_server_send_ntf_cmd const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    uint8_t state = ke_state_get(dest_id);
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);

    if (state == ATVVS_IDLE || state > ATVVS_BUSY) {
        atvv_server_send_error(ATVV_SERVER_SEND_NTF_CMD, (uint8_t)ATVV_SERVER_ERROR_NO_CONN, dest_id, src_id);
        return (KE_MSG_CONSUMED);
    }

    if(!(atvv_server_env->ntf_cfg[conidx][0] == PRF_CLI_START_NTF || atvv_server_env->ntf_cfg[conidx][0] == PRF_CLI_START_IND)){
        atvv_server_send_error(ATVV_SERVER_SEND_NTF_CMD, (uint8_t)ATVV_SERVER_ERROR_NO_NOTI, dest_id, src_id);
        return (KE_MSG_CONSUMED);
    }
    if(co_fifo_avail(&(atvv_server_env->send_fifo[conidx]))+14< param->length)  
    {
        return (KE_MSG_CONSUMED);
    }
    if(co_fifo_is_full(&(atvv_server_env->send_fifo[conidx]))){
        // send app full message
        //atvv_server_send_error(ATVV_SERVER_SEND_NTF_CMD, (uint8_t)ATVV_SERVER_ERROR_NO_BUFF, dest_id, src_id);
        struct atvv_server_buffer_full_evt * evt = KE_MSG_ALLOC(ATVV_SERVER_BUFFER_FULL,
                prf_dst_task_get(&(atvv_server_env->prf_env), conidx),
                dest_id,
                atvv_server_buffer_full_evt);

        evt->operation = (uint8_t)ATVV_SERVER_SEND_NTF_CMD;
        evt->status = (uint8_t)GAP_ERR_INSUFF_RESOURCES;

        ke_msg_send(evt);
        return (KE_MSG_CONSUMED);
    }
    // audio data len = 134;
    uint8_t head[2];
    uint8_t temp_len = 0;
    
    while(temp_len<param->length)
    {
        if(temp_len == 0)
        {
            head[0] = ATVV_SERVER_AUDIO_TYPE_FIRST;
            head[1] = 20;
        }
        else if(temp_len+20 >= param->length)
        {
            head[0] = ATVV_SERVER_AUDIO_TYPE_LAST;
            head[1] = param->length-temp_len;
        }
        else
        {
            head[0] = ATVV_SERVER_AUDIO_TYPE_CONTI;
            head[1] = 20;
        }
        
        co_fifo_in(&(atvv_server_env->send_fifo[conidx]), (uint8_t*)&head[0], 2);
        co_fifo_in(&(atvv_server_env->send_fifo[conidx]), (uint8_t*)param->value+temp_len, head[1]);
        temp_len += head[1];
    }
    
    if (state == ATVVS_BUSY) {
        return (KE_MSG_CONSUMED);
    }

    return atvv_server_send_data_to_gattc(dest_id, conidx);
}

__STATIC int atvv_server_send_ctl_cmd_handler(ke_msg_id_t const msgid,  struct atvv_server_send_ctl_cmd const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	  uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(src_id);
    uint16_t att_handle = atvv_server_get_att_handle(ATVVS_IDX_CONTROL_VAL);
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);

    uint16_t len = atvv_server_env->perfect_once_tx_length[conidx];
    len = co_min(len,param->length);
    //printf("len=%d,default_len=%d\n", len, default_len);

    
    if (state == ATVVS_IDLE) {
        atvv_server_send_error(ATVV_SERVER_SEND_CTL_CMD, (uint8_t)ATVV_SERVER_ERROR_NO_CONN, dest_id, src_id);
        return (KE_MSG_CONSUMED);
    }

    if(!(atvv_server_env->ntf_cfg[conidx][1] == PRF_CLI_START_NTF || atvv_server_env->ntf_cfg[conidx][1] == PRF_CLI_START_IND)){
        atvv_server_send_error(ATVV_SERVER_SEND_CTL_CMD, (uint8_t)ATVV_SERVER_ERROR_NO_NOTI, dest_id, src_id);
        return (KE_MSG_CONSUMED);
    }
    
    if (len >0) {
        struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                                        KE_BUILD_ID(TASK_GATTC, conidx), 
                                                        prf_get_task_from_id(KE_BUILD_ID(TASK_ID_ATVVS, conidx)), 
                gattc_send_evt_cmd, len);
        if(atvv_server_env->ntf_cfg[conidx][1] == PRF_CLI_START_NTF){
            req->operation = GATTC_NOTIFY;
        }else{
            req->operation = GATTC_INDICATE;
        }

        req->handle    = att_handle;
        req->length    = len;
        memcpy(req->value, &param->value[0], req->length);
        // Send the event
        ke_msg_send(req);
        //ke_state_set(dest_id, ATVVS_BUSY);
    }
    else {
        //ke_state_set(dest_id, ATVVS_CONNECTED);
    }

    return (KE_MSG_CONSUMED);
}
static int atvv_server_gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid, struct gattc_mtu_changed_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    //log_debug("mtu=%d\n", param->mtu);
    uint8_t conidx = KE_IDX_GET(src_id);
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    atvv_server_env->mtu[conidx] = param->mtu;
    return KE_MSG_CONSUMED;
}

__STATIC int atvv_server_default_handler(ke_msg_id_t const msgid,  void const *param,
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
KE_MSG_HANDLER_TAB(atvvs)
{
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t) atvv_server_default_handler},
    {ATVV_SERVER_ENABLE_REQ,        (ke_msg_func_t) atvv_server_enable_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) atvv_server_gattc_att_info_req_ind_handler},
    {ATVV_SERVER_SEND_NTF_CMD,      (ke_msg_func_t) atvv_server_send_ntf_cmd_handler},
    {ATVV_SERVER_SEND_CTL_CMD,       (ke_msg_func_t) atvv_server_send_ctl_cmd_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) atvv_server_gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) atvv_server_gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) atvv_server_gattc_cmp_evt_handler},
    {GATTC_MTU_CHANGED_IND,         (ke_msg_func_t) atvv_server_gattc_mtu_changed_ind_handler},
};

void atvv_server_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct atvvs_env_tag *atvvs_env = PRF_ENV_GET(ATVVS, atvvs);

    task_desc->msg_handler_tab = atvvs_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(atvvs_msg_handler_tab);
    task_desc->state           = atvvs_env->state;
    task_desc->idx_max         = ATVVS_IDX_MAX;
}

uint16_t atvv_server_send_data(uint8_t conidx, uint8_t* pdata, uint16_t len)
{
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);

    if (atvv_server_env->state[conidx] == ATVVS_IDLE || atvv_server_env->state[conidx] > ATVVS_BUSY) {
        return 0;
    }

    if(!(atvv_server_env->ntf_cfg[conidx][0] == PRF_CLI_START_NTF || atvv_server_env->ntf_cfg[conidx][0] == PRF_CLI_START_IND)){
        return 0;
    }
    if(co_fifo_is_full(&(atvv_server_env->send_fifo[conidx]))){
        return 0;
    }
    if(co_fifo_avail(&(atvv_server_env->send_fifo[conidx]))+14< len)  
    {
        return 0;
    }
    // audio data len = 134;
    uint8_t head[2];
    uint8_t temp_len = 0;
    
    while(temp_len<len)
    {
        if(temp_len == 0)
        {
            head[0] = ATVV_SERVER_AUDIO_TYPE_FIRST;
            head[1] = 20;
        }
        else if(temp_len+20 >= len)
        {
            head[0] = ATVV_SERVER_AUDIO_TYPE_LAST;
            head[1] = len-temp_len;
        }
        else
        {
            head[0] = ATVV_SERVER_AUDIO_TYPE_CONTI;
            head[1] = 20;
        }
        
        co_fifo_in(&(atvv_server_env->send_fifo[conidx]), (uint8_t*)&head[0], 2);
        co_fifo_in(&(atvv_server_env->send_fifo[conidx]), pdata+temp_len, head[1]);
        temp_len += head[1];
    }

    if (atvv_server_env->state[conidx] == ATVVS_BUSY) {
        return len;
    }

    atvv_server_send_data_to_gattc(KE_BUILD_ID(prf_get_task_from_id(TASK_ID_ATVVS), conidx), conidx);
    return len;
}

uint8_t atvv_server_check_fifo_full(uint8_t conidx)
{
    struct atvvs_env_tag *atvvs_env = PRF_ENV_GET(ATVVS, atvvs);

    return co_fifo_is_full(&(atvvs_env->send_fifo[conidx]));
}

uint32_t atvv_server_get_free_size(uint8_t conidx)
{
    struct atvvs_env_tag *atvvs_env = PRF_ENV_GET(ATVVS, atvvs);
    return co_fifo_avail(&(atvvs_env->send_fifo[conidx]));
}
#endif /* #if (BLE_ATVV_SERVER) */

/// @} ATVVSTASK
