/**
 ****************************************************************************************
 *
 * @file simple_server_task.c
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
 * @addtogroup SIMPLE_SERVERTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#include "gap.h"
#include "gattc_task.h"

#include "simple_server.h"
#include "simple_server_task.h"

#include "prf_utils.h"

#include "co_utils.h"

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SIMPLE_SERVER_ENABLE_REQ message.
 * The handler enables the 'Profile' Server Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int simple_server_enable_req_handler(ke_msg_id_t const msgid,
                                   struct simple_server_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);

    // check state of the task
    if(state == SIMPLE_SERVER_IDLE)
    {
        struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct simple_server_enable_rsp* rsp = KE_MSG_ALLOC(SIMPLE_SERVER_ENABLE_RSP, src_id,
                dest_id, simple_server_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            ke_msg_send(rsp);
        }
        else
        {
            simple_server_env->ntf_cfg[param->conidx] = param->ntf_cfg;
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
__STATIC int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint8_t att_idx = 0;
    // retrieve handle information
    uint8_t status = simple_server_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(att_idx == SIMPLE_SERVER_IDX_DEMO_NTF_CFG)
        {
            // CCC attribute length = 2
            cfm->length = sizeof(uint16_t);
        }
        else if(att_idx == SIMPLE_SERVER_IDX_DEMO_VAL1 || att_idx == SIMPLE_SERVER_IDX_DEMO_VAL2)
        {
            cfm->length = SIMPLE_SERVER_MAX_CHAC_LEN;
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
#include "main.h"
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = simple_server_get_att_idx(param->handle, &att_idx);
    log_debug("%s handle:%d(idx:%d).\n", __func__, param->handle, att_idx);
	
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
			
        struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);

        // Only update configuration if value for stop or notification enable
        if (att_idx == SIMPLE_SERVER_IDX_DEMO_NTF_CFG)
        {
//					printf("dest_id = %d\r\n",dest_id);
					
            // Extract value before check
            uint16_t ntf_cfg = co_read16p(&param->value[0]);
            if((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF) || (ntf_cfg == PRF_CLI_START_IND))
            {
                // Conserve information in environment
                simple_server_env->ntf_cfg[conidx] = ntf_cfg;
            }
            // Inform APP of configuration change
            struct simple_server_demo_ntf_cfg_ind * ind = KE_MSG_ALLOC(SIMPLE_SERVER_NTF_CFG_IND,
                    prf_dst_task_get(&(simple_server_env->prf_env), conidx), dest_id,
                    simple_server_demo_ntf_cfg_ind);
            ind->conidx = conidx;
            ind->ntf_cfg = simple_server_env->ntf_cfg[conidx];

            ke_msg_send(ind);
        }
        else if (att_idx == SIMPLE_SERVER_IDX_DEMO_VAL1 || att_idx == SIMPLE_SERVER_IDX_DEMO_VAL2)
        {
            log_debug("Offset:%2d. ", param->offset);   
            log_debug_array_ex("write data", param->value, param->length); 
					
						for(int i = 0; i < param->length; i++) QUEUE_WRITE(BLE_Rx, param->value[i]);  // 20210525
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
__STATIC int gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = simple_server_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);
    log_debug("%s handle:%d(idx:%d).\n", __func__, param->handle, att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == SIMPLE_SERVER_IDX_DEMO_NTF_CFG)
        {
            length = sizeof(uint16_t);
        }
        else if (att_idx == SIMPLE_SERVER_IDX_DEMO_VAL1 || att_idx == SIMPLE_SERVER_IDX_DEMO_VAL2)
        {
            length = 50;
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
        if (att_idx == SIMPLE_SERVER_IDX_DEMO_NTF_CFG)
        {
            cfm->value[0] = simple_server_env->ntf_cfg[conidx]; //Characteristic value
        }
        else if (att_idx == SIMPLE_SERVER_IDX_DEMO_VAL2)
        {
          for(int i=0;i<length;i++)
					{
						cfm->value[i] = i; //Characteristic value     20210519
          }
        }
    }

    ke_msg_send(cfm);

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
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    log_debug("%s msgid=0x%04x, operation=%d, status=%d\n", __func__, msgid, param->operation, param->status);
    return (KE_MSG_CONSUMED);
}

__STATIC int simple_server_send_ntf_cmd_handler(ke_msg_id_t const msgid,  struct simple_server_send_ntf_cmd const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);
   
//		printf("8888888\r\n");
		if(!(simple_server_env->ntf_cfg[conidx] == PRF_CLI_START_NTF || simple_server_env->ntf_cfg[conidx] == PRF_CLI_START_IND)){
        return (KE_MSG_CONSUMED);
    }
	
    // Attribute handle
    uint16_t att_handle = simple_server_get_att_handle(SIMPLE_SERVER_IDX_DEMO_VAL1);
    // Send the indication
    struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            TASK_GATTC, dest_id, gattc_send_evt_cmd, param->length);
    // Fill in the parameter structure
    if(simple_server_env->ntf_cfg[conidx] == PRF_CLI_START_NTF){
        req->operation = GATTC_NOTIFY;
    }else{
        req->operation = GATTC_INDICATE;
    }
    req->handle    = att_handle;
    req->length    = param->length;
    memcpy(req->value, &param->value, req->length);
    // Send the event
    ke_msg_send(req);

    return (KE_MSG_CONSUMED);
}

__STATIC int simple_server_default_handler(ke_msg_id_t const msgid,  void const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    log_debug("%s msgid=0x%04x\n", __func__, msgid);
    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(simple_server)
{
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t) simple_server_default_handler},
    {SIMPLE_SERVER_ENABLE_REQ,      (ke_msg_func_t) simple_server_enable_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {SIMPLE_SERVER_SEND_NTF_CMD,    (ke_msg_func_t) simple_server_send_ntf_cmd_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
};

void simple_server_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct simple_server_env_tag *simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);

    task_desc->msg_handler_tab = simple_server_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(simple_server_msg_handler_tab);
    task_desc->state           = simple_server_env->state;
    task_desc->idx_max         = SIMPLE_SERVER_IDX_MAX;
}


void enable_notification(uint8_t enable)
{

	uint8_t conidx = 0; 

	struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);

//	uint16_t ntf_cfg = PRF_CLI_START_NTF; 
	if(enable) simple_server_env->ntf_cfg[conidx] = PRF_CLI_START_NTF;
	else simple_server_env->ntf_cfg[conidx] = PRF_CLI_STOP_NTFIND;

	// Inform APP of configuration change
	struct simple_server_demo_ntf_cfg_ind * ind = KE_MSG_ALLOC(SIMPLE_SERVER_NTF_CFG_IND,
					prf_dst_task_get(&(simple_server_env->prf_env), conidx), 9/*dest_id*/,
					simple_server_demo_ntf_cfg_ind);
	ind->conidx = conidx;
	ind->ntf_cfg = simple_server_env->ntf_cfg[conidx];

	ke_msg_send(ind);
}


/// @} SIMPLE_SERVERTASK
