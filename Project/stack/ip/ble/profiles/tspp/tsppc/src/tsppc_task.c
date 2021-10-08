/**
 ****************************************************************************************
 *
 * @file tsppc_task.c
 *
 * @brief Transport Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup TSPPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "tspp_common.h"

#if (BLE_TSPP_COLLECTOR)
#include "co_utils.h"
#include "gap.h"
#include "attm.h"
#include "tsppc_task.h"
#include "tsppc.h"
#include "gattc_task.h"

#include "ke_mem.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// State machine used to retrieve transport service characteristics information
const struct prf_char_def tsppc_char[TSPPC_CHAR_MAX] =
{
    /// notify
    [TSPPC_CHAR_NOTIFY]              = {ATT_CHAR_TSPP_UPLOAD,
                                        ATT_MANDATORY,
                                        ATT_CHAR_PROP_NTF},
    /// Body Sensor Location
    [TSPPC_CHAR_WRITE]               = {ATT_CHAR_TSPP_REV1,
                                        ATT_MANDATORY,
                                        ATT_CHAR_PROP_WR},
    /// Heart Rate Control Point
    [TSPPC_CHAR_WRITE_NO_RESPONSE]   = {ATT_CHAR_TSPP_REV2,
                                        ATT_MANDATORY,
                                        ATT_CHAR_PROP_WR_NO_RESP},
};

/// State machine used to retrieve transport service characteristic description information
const struct prf_char_desc_def tsppc_char_desc[TSPPC_DESC_MAX] =
{
    /// Heart Rate Measurement client config
    [TSPPC_DESC_CLI_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, TSPPC_CHAR_NOTIFY},
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_SDP_SVC_IND_HANDLER message.
 * The handler stores the found service details for service discovery.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                                             struct gattc_sdp_svc_ind const *ind,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);

    if(state == TSPPC_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(src_id);

        struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);

        ASSERT_INFO(tsppc_env != NULL, dest_id, src_id);
        ASSERT_INFO(tsppc_env->env[conidx] != NULL, dest_id, src_id);

        if(tsppc_env->env[conidx]->nb_svc == 0)
        {
            // Retrieve characteristics and descriptors
            prf_extract_svc_info(ind, TSPPC_CHAR_MAX, &tsppc_char[0],  &tsppc_env->env[conidx]->tspps.chars[0],
                                      TSPPC_DESC_MAX, &tsppc_char_desc[0], &tsppc_env->env[conidx]->tspps.descs[0]);

            //Even if we get multiple responses we only store 1 range
            tsppc_env->env[conidx]->tspps.svc.shdl = ind->start_hdl;
            tsppc_env->env[conidx]->tspps.svc.ehdl = ind->end_hdl;
        }

        tsppc_env->env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_CMP_EVT message.
 * This generic event is received for different requests, so need to keep track.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gattc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);
    // Status
    uint8_t status;

    if (tsppc_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(src_id);
        uint8_t state = ke_state_get(dest_id);

        if(state == TSPPC_DISCOVERING)
        {
            status = param->status;

            if ((status == ATT_ERR_ATTRIBUTE_NOT_FOUND)||
                (status == ATT_ERR_NO_ERROR))
            {
                // Discovery
                // check characteristic validity
                if(tsppc_env->env[conidx]->nb_svc == 1)
                {
                    status = prf_check_svc_char_validity(TSPPC_CHAR_MAX,
                            tsppc_env->env[conidx]->tspps.chars,
                            tsppc_char);
                }
                // too much services
                else if (tsppc_env->env[conidx]->nb_svc > 1)
                {
                    status = PRF_ERR_MULTIPLE_SVC;
                }
                // no services found
                else
                {
                    status = PRF_ERR_STOP_DISC_CHAR_MISSING;
                }

                // check descriptor validity
                if (status == GAP_ERR_NO_ERROR)
                {
                    status = prf_check_svc_char_desc_validity(TSPPC_DESC_MAX,
                            tsppc_env->env[conidx]->tspps.descs,
                            tsppc_char_desc,
                            tsppc_env->env[conidx]->tspps.chars);
                }
            }

            tsppc_enable_rsp_send(tsppc_env, conidx, status);
        }

        else if(state == TSPPC_BUSY)
        {
            switch(param->operation)
            {
                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    uint16_t rsp_msg_id = TSPPC_CFG_INDNTF_RSP;
                    if(tsppc_env->env[conidx]->last_char_code != TSPPC_DESC_CLI_CFG)
                    {
                        rsp_msg_id = TSPPC_WR_CHAR_RSP;
                    }

                    struct tsppc_cfg_indntf_rsp *rsp = KE_MSG_ALLOC(rsp_msg_id,
                            prf_dst_task_get(&(tsppc_env->prf_env), conidx), dest_id,
                            tsppc_cfg_indntf_rsp);
                    rsp->status    = param->status;
                    // Send the message
                    ke_msg_send(rsp);
                }
                break;

                case GATTC_READ:
                {
                    if(param->status != GAP_ERR_NO_ERROR)
                    {
                        // an error occurs while reading peer device attribute
                        prf_client_att_info_rsp(
                                (prf_env_t*) tsppc_env->env[conidx],
                                conidx,
                                TSPPC_RD_CHAR_RSP,
                                param->status,
                                NULL);
                    }
                }
                break;

                default: break;
            }

            ke_state_set(prf_src_task_get(&tsppc_env->prf_env, conidx), TSPPC_IDLE);
        }
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_READ_IND message.
 * Generic event received after every simple read command sent to peer server.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_ind_handler(ke_msg_id_t const msgid,
                                    struct gattc_read_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(src_id);

    prf_client_att_info_rsp(&tsppc_env->prf_env, conidx, TSPPC_RD_CHAR_RSP,
            GAP_ERR_NO_ERROR, param);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_EVENT_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                        struct gattc_event_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    // Get the address of the environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);

    if(tsppc_env != NULL)
    {
        if((param->handle == tsppc_env->env[conidx]->tspps.chars[TSPPC_CHAR_NOTIFY].val_hdl) &&
                (param->type == GATTC_NOTIFY))
        {
            //build a TSPPC_NOTIFY_IND message.
            struct tsppc_notify_ind * ind = KE_MSG_ALLOC_DYN(
                    TSPPC_NOTIFY_IND,
                    prf_dst_task_get(&(tsppc_env->prf_env), conidx),
                    prf_src_task_get(&(tsppc_env->prf_env), conidx),
                    tsppc_notify_ind,param->length);

            ind->length = param->length;
            memcpy(ind->value, param->value, param->length);

            ke_msg_send(ind);
        }
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TSPPC_RD_DATETIME_REQ message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tsppc_rd_char_req_handler(ke_msg_id_t const msgid,
                                        struct tsppc_rd_char_req const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    uint8_t msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(src_id);

    if(state == TSPPC_IDLE)
    {
        ASSERT_INFO(tsppc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(tsppc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;

            if(((param->char_code & TSPPC_DESC_MASK) == TSPPC_DESC_MASK) &&
               ((param->char_code & ~TSPPC_DESC_MASK) < TSPPC_DESC_MAX))
            {
                search_hdl = tsppc_env->env[conidx]->tspps.descs[param->char_code & ~TSPPC_DESC_MASK].desc_hdl;
            }
            else if (param->char_code < TSPPC_CHAR_MAX)
            {
                search_hdl = tsppc_env->env[conidx]->tspps.chars[param->char_code].val_hdl;
            }

            //check if handle is viable
            if (search_hdl != ATT_INVALID_SEARCH_HANDLE)
            {
                // Store the command
                tsppc_env->env[conidx]->last_char_code = param->char_code;
                // Send the read request
                prf_read_char_send(&(tsppc_env->prf_env), conidx,
                        tsppc_env->env[conidx]->tspps.svc.shdl,
                        tsppc_env->env[conidx]->tspps.svc.ehdl,
                        search_hdl);

                // Go to the Busy state
                ke_state_set(dest_id, TSPPC_BUSY);

                status = ATT_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == TSPPC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        prf_client_att_info_rsp(&tsppc_env->prf_env, conidx, TSPPC_RD_CHAR_RSP,
                status, NULL);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TSPPC_CFG_INDNTF_REQ message.
 * It allows configuration of the peer ind/ntf/stop characteristic for a specified characteristic.
 * Will return an error code if that cfg char does not exist.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tsppc_cfg_indntf_req_handler(ke_msg_id_t const msgid,
                                       struct tsppc_cfg_indntf_req const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(src_id);
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    // Message status
    uint8_t msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);

    if(state == TSPPC_IDLE)
    {
        if(tsppc_env->env[conidx] != NULL)
        {

            // Status
            status = PRF_ERR_INVALID_PARAM;
            // Check if parameter is OK
            if((param->cfg_val == PRF_CLI_STOP_NTFIND) ||
                    (param->cfg_val == PRF_CLI_START_NTF))
            {
                // Status
                status = PRF_ERR_INEXISTENT_HDL;
                // Get handle of the client configuration
                uint16_t cfg_hdl =
                        tsppc_env->env[conidx]->tspps.descs[TSPPC_DESC_CLI_CFG].desc_hdl;
                //check if the handle value exists
                if (cfg_hdl != ATT_INVALID_SEARCH_HANDLE)
                {
                    tsppc_env->env[conidx]->last_char_code = TSPPC_DESC_CLI_CFG;

                    status = GAP_ERR_NO_ERROR;
                    // Go to the Busy state
                    ke_state_set(dest_id, TSPPC_BUSY);
                    // Send GATT Write Request
                    prf_gatt_write_ntf_ind(&tsppc_env->prf_env, conidx, cfg_hdl, param->cfg_val);
                }
            }
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }
    else if (state == TSPPC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        struct tsppc_cfg_indntf_rsp *rsp = KE_MSG_ALLOC(TSPPC_CFG_INDNTF_RSP, src_id, dest_id, tsppc_cfg_indntf_rsp);
        rsp->status    = status;
        // Send the message
        ke_msg_send(rsp);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TSPPC_ENABLE_REQ message.
 * The handler enables the Heart Rate Profile Collector Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tsppc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct tsppc_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(src_id);

    uint8_t state = ke_state_get(dest_id);
    // Client Role Task Environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);;

    ASSERT_INFO(tsppc_env != NULL, dest_id, src_id);
    // Config connection, start discovering
    if((state == TSPPC_IDLE) && (tsppc_env->env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        tsppc_env->env[conidx] = (struct tsppc_cnx_env*) ke_malloc(sizeof(struct tsppc_cnx_env),KE_MEM_ATT_DB);
        memset(tsppc_env->env[conidx], 0, sizeof(struct tsppc_cnx_env));

        //Config connection, start discovering
        if(param->con_type == PRF_CON_DISCOVERY)
        {
            //start discovering
            prf_disc_svc_send(&(tsppc_env->prf_env), conidx,  ATT_SERVICE_TSPP);

            tsppc_env->env[conidx]->last_uuid_req = ATT_SERVICE_TSPP;

            // Go to DISCOVERING state
            ke_state_set(dest_id, TSPPC_DISCOVERING);
        }
        //normal connection, get saved att details
        else
        {
            tsppc_env->env[conidx]->tspps = param->tspps;
            //send APP confirmation that can start normal connection to TH
            tsppc_enable_rsp_send(tsppc_env, conidx, GAP_ERR_NO_ERROR);
        }

        status = GAP_ERR_NO_ERROR;
    }

    // send an error if request fails
    if(status != GAP_ERR_NO_ERROR)
    {
        tsppc_enable_rsp_send(tsppc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
****************************************************************************************
* @brief Handles reception of the @ref TSPPC_WR_CHAR_REQ message.
* Check if the handle exists in profile(already discovered) and send request, otherwise
* error to APP.
* @param[in] msgid Id of the message received (probably unused).
* @param[in] param Pointer to the parameters of the message.
* @param[in] dest_id ID of the receiving task instance (probably unused).
* @param[in] src_id ID of the sending task instance.
* @return If the message was consumed or not.
****************************************************************************************
*/
__STATIC int tsppc_wr_char_req_handler(ke_msg_id_t const msgid,
                                       struct tsppc_wr_char_req const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(src_id);
    uint8_t status = GAP_ERR_NO_ERROR;

    // Message status
    uint8_t msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);

    if(state == TSPPC_IDLE)
    {
        //this is mandatory readable if it is included in the peer's DB
        if(tsppc_env->env[conidx] != NULL)
        {
            //this is mandatory readable if it is included in the peer's DB
            if (tsppc_env->env[conidx]->tspps.chars[TSPPC_CHAR_WRITE].char_hdl != ATT_INVALID_SEARCH_HANDLE)
            {
                if ((tsppc_env->env[conidx]->tspps.chars[TSPPC_CHAR_WRITE].prop & ATT_CHAR_PROP_WR) == ATT_CHAR_PROP_WR)
                {
                    tsppc_env->env[conidx]->last_char_code = TSPPC_CHAR_WRITE;

                    // Send GATT Write Request
                    prf_gatt_write(&tsppc_env->prf_env, conidx, tsppc_env->env[conidx]->tspps.chars[TSPPC_CHAR_WRITE].val_hdl,
                            (uint8_t *)&param->value, param->length, GATTC_WRITE);
                    // Go to the Busy state
                    ke_state_set(dest_id, TSPPC_BUSY);
                }
                //write not allowed, so no point in continuing
                else
                {
                    status = PRF_ERR_NOT_WRITABLE;
                }
            }
            //send app error indication for inexistent handle for this characteristic
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }
    else if (state == TSPPC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if(status != GAP_ERR_NO_ERROR)
    {
        struct tsppc_wr_char_rsp *rsp = KE_MSG_ALLOC(TSPPC_CHAR_WRITE, src_id, dest_id, tsppc_wr_char_rsp);
        rsp->status    = status;
        // Send the message
        ke_msg_send(rsp);
    }

    return (msg_status);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(tsppc)
{
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_READ_IND,                (ke_msg_func_t)gattc_read_ind_handler},
    {TSPPC_ENABLE_REQ,              (ke_msg_func_t)tsppc_enable_req_handler},
    {TSPPC_CFG_INDNTF_REQ,          (ke_msg_func_t)tsppc_cfg_indntf_req_handler},
    {TSPPC_RD_CHAR_REQ,             (ke_msg_func_t)tsppc_rd_char_req_handler},
    {TSPPC_WR_CHAR_REQ,             (ke_msg_func_t)tsppc_wr_char_req_handler},
};

void tsppc_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct tsppc_env_tag *tsppc_env = PRF_ENV_GET(TSPPC, tsppc);

    task_desc->msg_handler_tab = tsppc_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(tsppc_msg_handler_tab);
    task_desc->state           = tsppc_env->state;
    task_desc->idx_max         = TSPPC_IDX_MAX;
}

#endif /* (BLE_TSPP_COLLECTOR) */
/// @} TSPPCTASK
