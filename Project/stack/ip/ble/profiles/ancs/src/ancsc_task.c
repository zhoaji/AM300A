/**
 ****************************************************************************************
 *
 * @file ancsc_task.c
 *
 * @brief ANCS Client Task implementation. V201130.1.0
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup ANCSCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "rwip_config.h"

#include "gap.h"
#include "ancsc.h"
#include "ancsc_task.h"
#include "ancsc_utils.h"
#include "gattc_task.h"
#include "app_sec.h"
#include "co_math.h"

#include "ke_mem.h"
#include "co_utils.h"
#include "co_debug.h"

// ancs pending uid buffer
#define ANCS_INVALID_UID 0xFFFFFFFF
static uint32_t pending_uid[ANCS_MAX_PENDING_NUM];
static uint32_t pending_timestamp;
static uint32_t last_request_uid;
static uint8_t fifo_fr = ANCS_MAX_PENDING_NUM, fifo_ra = ANCS_MAX_PENDING_NUM;
static void push_pending_uid(uint32_t uid)
{
    ancs_debug("%s: 0x%08X\n", __func__, uid);
    pending_uid[fifo_ra++] = uid;
    fifo_ra = fifo_ra % ANCS_MAX_PENDING_NUM;
    if(fifo_fr == fifo_ra)
    {
        fifo_fr = (fifo_fr+1) % ANCS_MAX_PENDING_NUM;
    }
}
static uint8_t pop_pending_uid(uint32_t *uid)
{
    if(fifo_fr == fifo_ra){ // buffer empty
        ancs_debug("%s: -1\n", __func__);
        return false;
    }
    *uid = pending_uid[fifo_fr++];
    fifo_fr = fifo_fr % ANCS_MAX_PENDING_NUM;
    ancs_debug("%s: 0x%08X\n", __func__, *uid);
    return true;
}
static void clear_pending_uid(void)
{
    ancs_debug("%s\n", __func__);
    fifo_fr = fifo_ra;
}

static void set_time_diff(void)
{
    pending_timestamp = CO_TIME_SYS2MS(co_time());
}

static uint32_t get_time_diff(void)
{
    uint32_t now = CO_TIME_SYS2MS(co_time());
    ancs_debug("%s: 0x%08X\n", __func__, now - pending_timestamp);
    return now > pending_timestamp ? now - pending_timestamp : pending_timestamp - now;
}

static void trigger_pending_req(ke_task_id_t const dest_id)
{
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
    uint8_t conidx = KE_IDX_GET(dest_id);
    uint8_t state = ke_state_get(dest_id);
    uint32_t uid = ANCS_INVALID_UID;
    while(pop_pending_uid(&uid) && uid == ANCS_INVALID_UID);
    if(uid == ANCS_INVALID_UID){ // no uid to request
        ancs_debug("%s: uid:0x%08X-0x%08X\n", __func__, uid, last_request_uid);
        return;
    }
    ancs_debug("%s: GET_NTF_ATTR:0x%08X\n", __func__, uid);
    uint16_t handle = ancsc_env->env[conidx]->ancs.chars[ANCS_CNTL_POINT_CHAR].val_hdl;
    uint8_t req[] = {
    ANCS_COMMAND_ID_GET_NTF_ATTR,
    (uid >> 0) & 0xFF,  (uid >> 8) & 0xFF,
    (uid >> 16) & 0xFF, (uid >> 24) & 0xFF,
    ANCS_NTF_ATTR_ID_APP_IDENTIFIER, ANCS_APPID_CAP & 0xFF,     (ANCS_APPID_CAP >> 8) & 0xFF,
    ANCS_NTF_ATTR_ID_TITLE,          ANCS_TITLE_CAP & 0xFF,     (ANCS_TITLE_CAP >> 8) & 0xFF,
    ANCS_NTF_ATTR_ID_SUBTITLE,       ANCS_SUBTITLE_CAP & 0xFF,  (ANCS_SUBTITLE_CAP >> 8) & 0xFF,
    ANCS_NTF_ATTR_ID_DATE,           ANCS_DATE_CAP & 0xFF,      (ANCS_DATE_CAP >> 8) & 0xFF,
    //ANCS_NTF_ATTR_ID_POS_ACT_LABEL,  ANCS_ACT_LABEL_CAP & 0xFF, (ANCS_ACT_LABEL_CAP) >> 8 & 0xFF,
    //ANCS_NTF_ATTR_ID_NEG_ACT_LABEL,  ANCS_ACT_LABEL_CAP & 0xFF, (ANCS_ACT_LABEL_CAP) >> 8 & 0xFF,
    ANCS_NTF_ATTR_ID_MESSAGE,        ANCS_MSG_CAP & 0xFF,       (ANCS_MSG_CAP >> 8) & 0xFF,
    };
    prf_gatt_write(&ancsc_env->prf_env, conidx, handle, req, sizeof(req), GATTC_WRITE);
    set_time_diff();
    last_request_uid = uid;
    ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_WAIT_DATA));
}

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

#define ATT_SVC_ANCS_UUID            (ATT_UUID_16(0x00D0))
#define ATT_SVC_ANCS_UUID128         {0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79}
#define ATT_CHAR_NTF_UUID            (ATT_UUID_16(0x1DBD))
#define ATT_CHAR_NTF_UUID128         {0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0x8C, 0xD9, 0x42, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F}
#define ATT_CHAR_CNTL_POINT_UUID     (ATT_UUID_16(0xD9D9))
#define ATT_CHAR_CNTL_POINT_UUID128  {0xD9, 0xD9, 0xAA, 0xFD, 0xE6, 0x9B, 0x21, 0x98, 0xA8, 0x49, 0xE1, 0x45, 0xF3, 0xD8, 0xD1, 0x69}
#define ATT_CHAR_DATA_SOURCE_UUID    (ATT_UUID_16(0x7BFB))
#define ATT_CHAR_DATA_SOURCE_UUID128 {0xFB, 0x7B, 0x7C, 0xCE, 0x6A, 0xB3, 0x44, 0xBE, 0xB5, 0x4B, 0xD6, 0x24, 0xE9, 0xC6, 0xEA, 0x22}

/// State machine used to retrieve ANCS characteristics information
const struct prf_char_def ancsc_ancs_char[ANCS_CHAR_MAX] =
{

    [ANCS_CNTL_POINT_CHAR]  = {ATT_CHAR_CNTL_POINT_UUID,  ATT_OPTIONAL,  ATT_CHAR_PROP_WR | ATT_CHAR_PROP_EXT_PROP},
    [ANCS_NTF_SOURCE_CHAR]  = {ATT_CHAR_NTF_UUID,         ATT_MANDATORY, ATT_CHAR_PROP_NTF},
    [ANCS_DATA_SOURCE_CHAR] = {ATT_CHAR_DATA_SOURCE_UUID, ATT_OPTIONAL,  ATT_CHAR_PROP_NTF},
};

/// State machine used to retrieve ANCS characteristic description information
const struct prf_char_desc_def ancsc_ancs_char_desc[ANCS_DESC_MAX] =
{
    [ANCS_DESC_NTF_SRC_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, ANCS_NTF_SOURCE_CHAR},
    [ANCS_DESC_DATA_SRC_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, ANCS_DATA_SOURCE_CHAR},
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ANCSC_ENABLE_REQ message.
 * The handler enables the Battery Service Client Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int ancsc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct ancsc_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Battery service Client Role Task Environment
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
    ASSERT_INFO(ancsc_env != NULL, dest_id, src_id);
    if((ANCS_GET_STATE(state) == ANCSC_ST_IDLE) && (ancsc_env->env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        ancsc_env->env[conidx] = (struct ancsc_cnx_env*) ke_malloc(sizeof(struct ancsc_cnx_env), KE_MEM_ATT_DB);
        memset(ancsc_env->env[conidx], 0, sizeof(struct ancsc_cnx_env));
        fifo_fr = fifo_ra = last_request_uid = 0; // init pending buffer

        //Config connection, start discovering
        if(param->con_type == PRF_CON_DISCOVERY)
        {
            //start discovering ANCS on peer
            uint8_t ancs_svc_uuid[] = ATT_SVC_ANCS_UUID128;
            prf_disc_svc_send_uuid128(&(ancsc_env->prf_env), conidx, ancs_svc_uuid);
            // Go to DISCOVERING state
            ke_state_set(dest_id, ANCSC_ST_DISCOVER);
        }
        //normal connection, get saved att details
        else
        {
            memcpy(&ancsc_env->env[conidx]->ancs, &param->ancs, sizeof(struct ancs_content));
            ke_state_set(dest_id, ANCSC_ST_REGISTER_GATTC);
            //send APP confirmation that can start normal connection to ANCS
            ancsc_enable_rsp_send(ancsc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }else if(ANCS_GET_STATE(state) != ANCSC_ST_FREE){
        ke_state_set(dest_id, ANCSC_ST_ERROR);
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if(status != GAP_ERR_NO_ERROR)
    {
        ancsc_enable_rsp_send(ancsc_env, conidx, status);
    }

    return KE_MSG_CONSUMED;
}


__STATIC int ancsc_encrypted_cmd_handler(ke_msg_id_t const msgid,
                                   struct ancsc_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    ke_state_set(dest_id, ANCS_SET_ST_ENCRYPTED(state));
    ancs_debug("ANCS change state to ENCRYPTED\n");
    
    uint8_t conidx = KE_IDX_GET(dest_id);
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
    if(ANCS_GET_STATE(state) >= ANCSC_ST_ENABLE_DATA && ANCS_GET_STATE(state) <= ANCSC_ST_PERFORM){
        uint16_t handle = ancsc_env->env[conidx]->ancs.descs[ANCS_DESC_DATA_SRC_CFG].desc_hdl;
        // Send GATT Write Request
        prf_gatt_write_ntf_ind(&ancsc_env->prf_env, conidx, handle, PRF_CLI_START_NTF);
        ancs_debug("ANCS Enable data source again\n");
        ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_ENABLE_DATA));
    }
    return KE_MSG_CONSUMED;
}

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
    if(ANCS_GET_STATE(state) == ANCSC_ST_DISCOVER)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);

        ASSERT_INFO(ancsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(ancsc_env->env[conidx] != NULL, dest_id, src_id);

        // Retrieve DIS characteristics
        prf_extract_svc_info(ind, ANCS_CHAR_MAX, &ancsc_ancs_char[0],
                &(ancsc_env->env[conidx]->ancs.chars[0]),
                ANCS_DESC_MAX, &ancsc_ancs_char_desc[0],
                &(ancsc_env->env[conidx]->ancs.descs[0]));

        //Even if we get multiple responses we only store 1 range
        ancsc_env->env[conidx]->ancs.svc.shdl = ind->start_hdl;
        ancsc_env->env[conidx]->ancs.svc.ehdl = ind->end_hdl;
        ancs_debug("ANCS service discovered\n");
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
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get the address of the environment
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);

    // sanity check
    if(ancsc_env->env[conidx] != NULL){
        if(param->operation == GATTC_SDP_DISC_SVC && ANCS_GET_STATE(state) == ANCSC_ST_DISCOVER){
            uint8_t status = param->status;
            if (param->status == ATT_ERR_NO_ERROR){
                // check characteristic validity
                status = prf_check_svc_char_validity(ANCS_CHAR_MAX, ancsc_env->env[conidx]->ancs.chars, ancsc_ancs_char);
                // check descriptor validity
                if(status == GAP_ERR_NO_ERROR){
                    status = prf_check_svc_char_desc_validity(ANCS_DESC_MAX,
                            ancsc_env->env[conidx]->ancs.descs, ancsc_ancs_char_desc,
                            ancsc_env->env[conidx]->ancs.chars);
                    ancs_debug("ANCS Register gattc\n");
#if 0
                    if(!ANCS_GET_ST_ENCRYPTED(state)){
                        app_sec_send_security_req(conidx);
                    }
#endif
                    ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_REGISTER_GATTC));
                }
            }else{
                status = PRF_ERR_STOP_DISC_CHAR_MISSING;
                ke_state_set(dest_id, ANCSC_ST_ERROR);
            }
            ancsc_enable_rsp_send(ancsc_env, conidx, status);
        }else if(param->operation == GATTC_REGISTER && ANCS_GET_STATE(state) == ANCSC_ST_REGISTER_GATTC){
            if(1){ // Failed if already registed, so always goto next state
                if(ANCS_GET_ST_ENCRYPTED(state)){
                    uint16_t handle = ancsc_env->env[conidx]->ancs.descs[ANCS_DESC_DATA_SRC_CFG].desc_hdl;
                    // Send GATT Write Request
                    prf_gatt_write_ntf_ind(&ancsc_env->prf_env, conidx, handle, PRF_CLI_START_NTF);
                    ancs_debug("ANCS Enable data source\n");
                    ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_ENABLE_DATA));
                }else{
                    msg_status = KE_MSG_SAVED;
                }
            }else{
                ke_state_set(dest_id, ANCSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && ANCS_GET_STATE(state) == ANCSC_ST_ENABLE_DATA){
            if(param->status == ATT_ERR_NO_ERROR){
                uint16_t handle = ancsc_env->env[conidx]->ancs.descs[ANCS_DESC_NTF_SRC_CFG].desc_hdl;
                // Send GATT Write Request
                prf_gatt_write_ntf_ind(&ancsc_env->prf_env, conidx, handle, PRF_CLI_START_NTF);
                ancs_debug("ANCS Enable control point\n");
                ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_ENABLE_NTF));
            }else if(param->status == ATT_ERR_INSUFF_ENC){
                // Do nothing.
            }else{
                ke_state_set(dest_id, ANCSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && ANCS_GET_STATE(state) == ANCSC_ST_ENABLE_NTF){
            if(param->status == ATT_ERR_NO_ERROR){
                ancs_debug("ANCS Waiting for GATT notification\n");
                ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_WAIT_NTF));
            }else if(param->status == ATT_ERR_INSUFF_ENC){
                // Do nothing.
            }else{
                ke_state_set(dest_id, ANCSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && ANCS_GET_STATE(state) == ANCSC_ST_WAIT_DATA){
        }else if(param->operation == GATTC_WRITE && ANCS_GET_STATE(state) == ANCSC_ST_PERFORM){
            ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_WAIT_NTF));
        }else{
            ancs_debug("ANCS Error op(0x%02x), state:(0x%02x)\n", param->operation, ANCS_GET_STATE(state));
        }
    }
    return msg_status;
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
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
    uint16_t handle_ntf = ancsc_env->env[conidx]->ancs.chars[ANCS_NTF_SOURCE_CHAR].val_hdl;
    uint16_t handle_data = ancsc_env->env[conidx]->ancs.chars[ANCS_DATA_SOURCE_CHAR].val_hdl;
    if(param->handle == handle_ntf){
        if(ANCS_GET_STATE(state) == ANCSC_ST_WAIT_NTF || ANCS_GET_STATE(state) == ANCSC_ST_WAIT_DATA){
            struct ancsc_notification_ind *ind = KE_MSG_ALLOC(ANCSC_NOTIFICATION_IND,
                                                    prf_dst_task_get(&(ancsc_env->prf_env) ,conidx),
                                                    dest_id,
                                                    ancsc_notification_ind);
            ancs_parse_ntf_ind(ind, param->value, param->length);
            if(ind->event_id == ANCS_EVENT_ID_NOTIFICATION_REMOVED){
                if(ind->notification_uid == last_request_uid){
                    ancs_debug("%s: pending msg removed:0x%08x\n", __func__, last_request_uid);
                    ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_WAIT_NTF));
                    trigger_pending_req(dest_id);
                }else{
                    // UID removed and not request
                }
            }
            ke_msg_send(ind);
        }else{
        }
    }else if(param->handle == handle_data){
        struct ancs_ntf_attr_rec* rec = ancs_parse_ntf_attr_ind(param->value, param->length);
        ancs_debug("%s: parse:%d(%d)\n", __func__, !!rec, rec?rec->app_id_len:0);
        if(rec){
            if(rec->app_id_len == 0){ // parse error
                ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_WAIT_NTF));
                return msg_status;
            }
            struct ancsc_ntf_attr_ind *ind = KE_MSG_ALLOC_DYN(ANCSC_NTF_ATTR_IND,
                                                        prf_dst_task_get(&(ancsc_env->prf_env), conidx),
                                                        dest_id,
                                                        ancsc_ntf_attr_ind,
                                                        rec->app_id_len + rec->title_len + rec->subtitle_len + rec->msg_len);
            memcpy(ind->date, rec->date, ANCS_DATE_CAP);
            ind->notif_uid = rec->notif_uid;
            ind->app_id = ind->data;
            ind->app_id_len = rec->app_id_len;
            memcpy(ind->app_id, rec->app_id, rec->app_id_len);
            ind->title = ind->app_id + ind->app_id_len;
            ind->title_len = rec->title_len;
            memcpy(ind->title, rec->title, rec->title_len);
            ind->subtitle = ind->title + ind->title_len;
            ind->subtitle_len = rec->subtitle_len;
            memcpy(ind->subtitle, rec->subtitle, rec->subtitle_len);
            ind->msg = ind->subtitle + ind->subtitle_len;
            ind->msg_len = rec->msg_len;
            memcpy(ind->msg, rec->msg, rec->msg_len);
            ke_msg_send(ind);
            ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_WAIT_NTF));
            trigger_pending_req(dest_id);
        }else{
        }
    }

    return msg_status;
}

__STATIC int ancsc_get_ntf_attr_cmd_handler(ke_msg_id_t const msgid,
                                   struct ancsc_get_ntf_attr_cmd const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
    ancs_debug("%s: 0x%08X\n", __func__, param->uid);
    if(ANCS_GET_STATE(state) == ANCSC_ST_WAIT_DATA){
        if(get_time_diff() > ANCS_GET_ATT_TOUT_MS){
            clear_pending_uid();
            state = ANCS_NEW_STATE(state, ANCSC_ST_WAIT_NTF);
            ke_state_set(dest_id, state);
        }
    }
    if(ANCS_GET_STATE(state) == ANCSC_ST_WAIT_NTF || ANCS_GET_STATE(state) == ANCSC_ST_WAIT_DATA){
        push_pending_uid(param->uid);
        if(ANCS_GET_STATE(state) == ANCSC_ST_WAIT_NTF){
            trigger_pending_req(dest_id);
        }
    }else if(ANCS_GET_STATE(state) == ANCSC_ST_PERFORM){
        msg_status = KE_MSG_SAVED;
    }else{
    }
    return msg_status;
}

__STATIC int ancsc_perform_act_cmd_handler(ke_msg_id_t const msgid,
                                   struct ancsc_perform_act_cmd const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
    if(ANCS_GET_STATE(state) == ANCSC_ST_WAIT_NTF){
        uint16_t handle = ancsc_env->env[conidx]->ancs.chars[ANCS_CNTL_POINT_CHAR].val_hdl;
        uint8_t req[] = {
            ANCS_COMMAND_ID_PERFORM_NTF_ACT,
            (param->uid >> 0) & 0xFF,  (param->uid >> 8) & 0xFF,
            (param->uid >> 16) & 0xFF, (param->uid >> 24) & 0xFF,
            param->act_id,
        };
        ke_state_set(dest_id, ANCS_NEW_STATE(state, ANCSC_ST_PERFORM));
        prf_gatt_write(&ancsc_env->prf_env, conidx, handle, req, sizeof(req), GATTC_WRITE);
    }else if(ANCS_GET_STATE(state) == ANCSC_ST_WAIT_DATA){
        msg_status = KE_MSG_SAVED;
    }
  return msg_status;
}

static int ancsc_mtu_changed_ind_handler(ke_msg_id_t const msgid, struct gattc_mtu_changed_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    ke_msg_forward(param, TASK_APP, src_id);
    return KE_MSG_NO_FREE;
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/// Default State handlers definition
KE_MSG_HANDLER_TAB(ancsc)
{
    {ANCSC_ENABLE_REQ,              (ke_msg_func_t)ancsc_enable_req_handler},
    {ANCSC_ENCRYPTED_CMD,           (ke_msg_func_t)ancsc_encrypted_cmd_handler},
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {ANCSC_GET_NTF_ATTR_CMD,        (ke_msg_func_t)ancsc_get_ntf_attr_cmd_handler},
    {ANCSC_PERFORM_ACT_CMD,         (ke_msg_func_t)ancsc_perform_act_cmd_handler},
    {GATTC_MTU_CHANGED_IND,         (ke_msg_func_t)ancsc_mtu_changed_ind_handler},
};

void ancsc_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);

    task_desc->msg_handler_tab = ancsc_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(ancsc_msg_handler_tab);
    task_desc->state           = ancsc_env->state;
    task_desc->idx_max         = ANCSC_IDX_MAX;
}


/// @} BASCTASK
