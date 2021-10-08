/**
 ****************************************************************************************
 *
 * @file amsc_task.c
 *
 * @brief Battery Service Client Task implementation.
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup AMSCTASK
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
#include "amsc.h"
#include "amsc_task.h"
#include "gattc_task.h"
#include "co_math.h"

#include "ke_mem.h"
#include "co_utils.h"
#include "co_debug.h"


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
#define ATT_SVC_AMS_UUID                (ATT_UUID_16(0xF8DC))
#define ATT_SVC_AMS_UUID128             {0xDC,0xF8,0x55,0xAD,0x02,0xC5,0xF4,0x8E,0x3A,0x43,0x36,0x0F,0x2B,0x50,0xD3,0x89}
#define ATT_CHAR_REMOTE_COMMAND_UUID    (ATT_UUID_16(0x51C2))
#define ATT_CHAR_REMOTE_COMMAND_UUID128 {0xC2,0x51,0xCA,0xF7,0x56,0x0E,0xDF,0xB8,0x8A,0x4A,0xB1,0x57,0xD8,0x81,0x3C,0x9B}
#define ATT_CHAR_ENTITY_UPDATE_UUID     (ATT_UUID_16(0xC102))
#define ATT_CHAR_ENTITY_UPDATE_UUID128  {0x02,0xC1,0x96,0xBA,0x92,0xBB,0x0C,0x9A,0x1F,0x41,0x8D,0x80,0xCE,0xAB,0x7C,0x2F}
#define ATT_CHAR_ENTITY_ATTR_UUID       (ATT_UUID_16(0xD5D7))
#define ATT_CHAR_ENTITY_ATTR_UUID128    {0xD7,0xD5,0xBB,0x70,0xA8,0xA3,0xAB,0xA6,0xD8,0x46,0xAB,0x23,0x8C,0xF3,0xB2,0xC6}


/// State machine used to retrieve AMS characteristics information
const struct prf_char_def amsc_ams_char[AMS_CHAR_MAX] =
{

    [AMS_REMOTE_COMMAND_CHAR] = {ATT_CHAR_REMOTE_COMMAND_UUID,  ATT_MANDATORY, ATT_CHAR_PROP_WR | ATT_CHAR_PROP_NTF | ATT_CHAR_PROP_EXT_PROP},
    [AMS_ENTITY_UPDATE_CHAR]  = {ATT_CHAR_ENTITY_UPDATE_UUID, ATT_OPTIONAL,  ATT_CHAR_PROP_WR | ATT_CHAR_PROP_NTF | ATT_CHAR_PROP_EXT_PROP},
    [AMS_ENTITY_ATTR_CHAR]    = {ATT_CHAR_ENTITY_ATTR_UUID,    ATT_OPTIONAL,  ATT_CHAR_PROP_WR | ATT_CHAR_PROP_RD | ATT_CHAR_PROP_EXT_PROP},
};

/// State machine used to retrieve AMS characteristic description information
const struct prf_char_desc_def amsc_ams_char_desc[AMS_DESC_MAX] =
{
    [AMS_DESC_REMOTE_COMMAND_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, AMS_REMOTE_COMMAND_CHAR},
    [AMS_DESC_ENTITY_UPDATE_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, ATT_OPTIONAL, AMS_ENTITY_UPDATE_CHAR},
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref AMSC_ENABLE_REQ message.
 * The handler enables the Battery Service Client Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int amsc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct amsc_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Client Role Task Environment
    struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);
    ASSERT_INFO(amsc_env != NULL, dest_id, src_id);
    ams_debug("AMS Enabled\n");
    if((AMS_GET_STATE(state) == AMSC_ST_IDLE) && (amsc_env->env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        amsc_env->env[conidx] = (struct amsc_cnx_env*) ke_malloc(sizeof(struct amsc_cnx_env), KE_MEM_ATT_DB);
        memset(amsc_env->env[conidx], 0, sizeof(struct amsc_cnx_env));

        //Config connection, start discovering
        if(param->con_type == PRF_CON_DISCOVERY)
        {
            //start discovering AMS on peer
            uint8_t ams_svc_uuid[] = ATT_SVC_AMS_UUID128;
            prf_disc_svc_send_uuid128(&(amsc_env->prf_env), conidx, ams_svc_uuid);
            // Go to DISCOVERING state
            ke_state_set(dest_id, AMSC_ST_DISCOVER);
        }
        //normal connection, get saved att details
        else
        {
            memcpy(&amsc_env->env[conidx]->ams, &param->ams, sizeof(struct ams_content));
            ke_state_set(dest_id, AMSC_ST_REGISTER_GATTC);
            //send APP confirmation that can start normal connection to AMS
            amsc_enable_rsp_send(amsc_env, conidx, GAP_ERR_NO_ERROR);
        }
        amsc_env->env[conidx]->ams_subscribe = param->ams_subscribe;
    }else if(AMS_GET_STATE(state) != AMSC_ST_FREE){
        ke_state_set(dest_id, AMSC_ST_ERROR);
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if(status != GAP_ERR_NO_ERROR)
    {
        amsc_enable_rsp_send(amsc_env, conidx, status);
    }

    return KE_MSG_CONSUMED;
}

__STATIC int amsc_encrypted_cmd_handler(ke_msg_id_t const msgid,
                                   struct amsc_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    ke_state_set(dest_id, AMS_SET_ST_ENCRYPTED(state));
    ams_debug("AMS change state to ENCRYPTED\n");
    return KE_MSG_CONSUMED;
}

__STATIC int amsc_rem_ctrl_cmd_handler(ke_msg_id_t const msgid,
                                   struct amsc_rem_ctrl_cmd const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get the address of the environment
    struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);
    // sanity check
    if(amsc_env->env[conidx] != NULL){
        if(param->value < AMS_CMD_Reserved){
            if(AMS_GET_ST_ENCRYPTED(state)){
                uint16_t handle = amsc_env->env[conidx]->ams.chars[AMS_REMOTE_COMMAND_CHAR].val_hdl;
                // Send GATT Write Request
                ams_debug("AMS remote ctrl: %d\n", value);
                prf_gatt_write(&amsc_env->prf_env, conidx, handle, (uint8_t*)&param->value, 1, GATTC_WRITE);
            }
        }
    }
    return msg_status;
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
    ams_debug("AMS service discovered(0x%04X-0x%04X), state=0x%02X\n", ind->start_hdl, ind->end_hdl, state);
    if(AMS_GET_STATE(state) == AMSC_ST_DISCOVER)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);

        ASSERT_INFO(amsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(amsc_env->env[conidx] != NULL, dest_id, src_id);

        // Retrieve DIS characteristics
        prf_extract_svc_info(ind, AMS_CHAR_MAX, &amsc_ams_char[0],
                &(amsc_env->env[conidx]->ams.chars[0]),
                AMS_DESC_MAX, &amsc_ams_char_desc[0],
                &(amsc_env->env[conidx]->ams.descs[0]));

        //Even if we get multiple responses we only store 1 range
        amsc_env->env[conidx]->ams.svc.shdl = ind->start_hdl;
        amsc_env->env[conidx]->ams.svc.ehdl = ind->end_hdl;
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
    struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);
    //ams_debug("gattc_cmp_evt_handler %d %d %d\n", param->operation, param->status, state);
    // sanity check
    if(amsc_env->env[conidx] != NULL){
        if(param->operation == GATTC_SDP_DISC_SVC && AMS_GET_STATE(state) == AMSC_ST_DISCOVER){
            uint8_t status = param->status;
            if (param->status == ATT_ERR_NO_ERROR){
                // check characteristic validity
                status = prf_check_svc_char_validity(AMS_CHAR_MAX, amsc_env->env[conidx]->ams.chars, amsc_ams_char);
                // check descriptor validity
                if(status == GAP_ERR_NO_ERROR){
                    status = prf_check_svc_char_desc_validity(AMS_DESC_MAX,
                            amsc_env->env[conidx]->ams.descs, amsc_ams_char_desc,
                            amsc_env->env[conidx]->ams.chars);
                    ams_debug("AMS Register gattc\n");
                    ke_state_set(dest_id, AMS_NEW_STATE(state, AMSC_ST_REGISTER_GATTC));
                }
            }else{;
                status = PRF_ERR_STOP_DISC_CHAR_MISSING;
                ke_state_set(dest_id, AMSC_ST_ERROR);
            }
            amsc_enable_rsp_send(amsc_env, conidx, status);
        }else if(param->operation == GATTC_REGISTER && AMS_GET_STATE(state) == AMSC_ST_REGISTER_GATTC){
            if(param->status == ATT_ERR_NO_ERROR){
                if(AMS_GET_ST_ENCRYPTED(state)){
                    uint16_t handle = amsc_env->env[conidx]->ams.descs[AMS_DESC_REMOTE_COMMAND_CFG].desc_hdl;
                    // Send GATT Write Request
                    prf_gatt_write_ntf_ind(&amsc_env->prf_env, conidx, handle, PRF_CLI_START_NTF);
                    ams_debug("AMS Enable remote control\n");
                    ke_state_set(dest_id, AMS_NEW_STATE(state, AMSC_ST_ENABLE_REM_CMD));
                }else{
                    msg_status = KE_MSG_SAVED;
                }
            }else{
                ke_state_set(dest_id, AMSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && AMS_GET_STATE(state) == AMSC_ST_ENABLE_REM_CMD){
            if(param->status == ATT_ERR_NO_ERROR){
                uint16_t handle = amsc_env->env[conidx]->ams.descs[AMS_DESC_ENTITY_UPDATE_CFG].desc_hdl;
                // Send GATT Write Request
                prf_gatt_write_ntf_ind(&amsc_env->prf_env, conidx, handle, PRF_CLI_START_NTF);
                ams_debug("AMS Enable entity update\n");
                ke_state_set(dest_id, AMS_NEW_STATE(state, AMSC_ST_ENABLE_ENTITY_UPDATE));
            }else{
                ke_state_set(dest_id, AMSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && AMS_GET_STATE(state) == AMSC_ST_ENABLE_ENTITY_UPDATE){
            if(param->status == ATT_ERR_NO_ERROR){
                uint8_t update[8] = {AMS_EntityIDPlayer}, len = 1;
                uint16_t subscribe = amsc_env->env[conidx]->ams_subscribe;
                if(subscribe & (1<<PlayerAttributeIDName)){ update[len++] = AMS_PlayerAttributeIDName-PlayerAttributeIDName; }
                if(subscribe & (1<<PlayerAttributeIDPlaybackInfo)){ update[len++] = AMS_PlayerAttributeIDPlaybackInfo-PlayerAttributeIDName; }
                if(subscribe & (1<<PlayerAttributeIDVolume)){ update[len++] = AMS_PlayerAttributeIDVolume-PlayerAttributeIDName; }
                if(len > 1)
                {
                    uint16_t handle = amsc_env->env[conidx]->ams.chars[AMS_ENTITY_UPDATE_CHAR].val_hdl;
                    // Send GATT Write Request
                    ams_debug("AMS set entity update: Player\n");
                    prf_gatt_write(&amsc_env->prf_env, conidx, handle, update, len, GATTC_WRITE);
                }
                else
                {
                    ke_msg_forward(param, src_id, dest_id);
                    msg_status = KE_MSG_SAVED;
                }
                ke_state_set(dest_id, AMS_NEW_STATE(state, AMSC_ST_SET_PLAYER_UPDATE));
            }else{
                ke_state_set(dest_id, AMSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && AMS_GET_STATE(state) == AMSC_ST_SET_PLAYER_UPDATE){
            if(param->status == ATT_ERR_NO_ERROR){
                uint8_t update[8] = {AMS_EntityIDQueue}, len = 1;
                uint16_t subscribe = amsc_env->env[conidx]->ams_subscribe;
                if(subscribe & (1<<QueueAttributeIDIndex)){ update[len++] = AMS_QueueAttributeIDIndex-AMS_QueueAttributeIDIndex; }
                if(subscribe & (1<<QueueAttributeIDCount)){ update[len++] = AMS_QueueAttributeIDCount-AMS_QueueAttributeIDIndex; }
                if(subscribe & (1<<QueueAttributeIDShuffleMode)){ update[len++] = AMS_QueueAttributeIDShuffleMode-AMS_QueueAttributeIDIndex; }
                if(subscribe & (1<<QueueAttributeIDRepeatMode)){ update[len++] = AMS_QueueAttributeIDRepeatMode-AMS_QueueAttributeIDIndex; }
                if(len > 1)
                {
                    uint16_t handle = amsc_env->env[conidx]->ams.chars[AMS_ENTITY_UPDATE_CHAR].val_hdl;
                    // Send GATT Write Request
                    ams_debug("AMS set entity update: Queue\n");
                    prf_gatt_write(&amsc_env->prf_env, conidx, handle, update, len, GATTC_WRITE);
                }
                else
                {
                    ke_msg_forward(param, src_id, dest_id);
                    msg_status = KE_MSG_SAVED;
                }
                ke_state_set(dest_id, AMS_NEW_STATE(state, AMSC_ST_SET_QUEUE_UPDATE));
            }else{
                ke_state_set(dest_id, AMSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && AMS_GET_STATE(state) == AMSC_ST_SET_QUEUE_UPDATE){
            if(param->status == ATT_ERR_NO_ERROR){
                uint8_t update[8] = {AMS_EntityIDTrack}, len = 1;
                uint16_t subscribe = amsc_env->env[conidx]->ams_subscribe;
                if(subscribe & (1<<TrackAttributeIDArtist)){ update[len++] = AMS_TrackAttributeIDArtist-AMS_TrackAttributeIDArtist; }
                if(subscribe & (1<<TrackAttributeIDAlbum)){ update[len++] = AMS_TrackAttributeIDAlbum-AMS_TrackAttributeIDArtist; }
                if(subscribe & (1<<TrackAttributeIDTitle)){ update[len++] = AMS_TrackAttributeIDTitle-AMS_TrackAttributeIDArtist; }
                if(subscribe & (1<<TrackAttributeIDDuration)){ update[len++] = AMS_TrackAttributeIDDuration-AMS_TrackAttributeIDArtist; }
                if(len > 1)
                {
                    uint16_t handle = amsc_env->env[conidx]->ams.chars[AMS_ENTITY_UPDATE_CHAR].val_hdl;
                    // Send GATT Write Request
                    ams_debug("AMS set entity update: Track\n");
                    prf_gatt_write(&amsc_env->prf_env, conidx, handle, update, len, GATTC_WRITE);
                }
                else
                {
                    ke_msg_forward(param, src_id, dest_id);
                    msg_status = KE_MSG_SAVED;
                }
                ke_state_set(dest_id, AMS_NEW_STATE(state, AMSC_ST_SET_TRACK_UPDATE));
            }else{
                ke_state_set(dest_id, AMSC_ST_ERROR);
            }
        }else if(param->operation == GATTC_WRITE && AMS_GET_STATE(state) == AMSC_ST_SET_TRACK_UPDATE){
            if(param->status == ATT_ERR_NO_ERROR){
                ams_debug("AMS Waiting for GATT notification\n");
                ke_state_set(dest_id, AMS_NEW_STATE(state, AMSC_ST_WAIT_NTF));
            }else{
                ke_state_set(dest_id, AMSC_ST_ERROR);
            }
        }else{
            ams_debug("AMS Error op(0x%02x), state:(0x%02x)\n", param->operation, AMS_GET_STATE(state));
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
    struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);
    uint16_t handle_remote_ctrl = amsc_env->env[conidx]->ams.chars[AMS_REMOTE_COMMAND_CHAR].val_hdl;
    uint16_t handle_entity_update = amsc_env->env[conidx]->ams.chars[AMS_ENTITY_UPDATE_CHAR].val_hdl;
    if(param->handle == handle_remote_ctrl){
        ams_debug_array_ex("Remote Ctrl", param->value, param->length);
        struct amsc_rem_ctrl_ind * rsp = KE_MSG_ALLOC_DYN(AMSC_REM_CTRL_IND, TASK_APP, dest_id,
                                                            amsc_rem_ctrl_ind, param->length);
        rsp->len = param->length;
        memcpy(rsp->value, param->value, rsp->len);
        ke_msg_send(rsp);
    }else if(param->handle == handle_entity_update){
        ASSERT_ERR(AMS_EntityIDPlayer<=value[0]&&value[0]<=AMS_EntityIDTrack);
        ASSERT_ERR(0<=value[1]&&value[1]<=3);
        ASSERT_ERR(param->length >= 3);
        uint8_t *value = (uint8_t*)param->value;
        ams_debug("Entity Update(%d,%d)", value[0], value[1]);
        ams_debug_array_ex("", param->value, param->length);
        const uint8_t subscribe_map[3][4] = {
            {PlayerAttributeIDName,PlayerAttributeIDPlaybackInfo,PlayerAttributeIDVolume,AMS_CMD_Reserved},
            {QueueAttributeIDIndex,QueueAttributeIDCount,QueueAttributeIDShuffleMode,QueueAttributeIDRepeatMode},
            {TrackAttributeIDArtist,TrackAttributeIDAlbum,TrackAttributeIDTitle,TrackAttributeIDDuration},
        };
        struct amsc_entity_att_ind * rsp = KE_MSG_ALLOC_DYN(AMSC_ENTITY_ATT_IND,
                                            TASK_APP, dest_id, amsc_entity_att_ind, param->length-3);
        rsp->subscribe_bit = subscribe_map[value[0]][value[1]];
        rsp->flag = value[2];
        rsp->len = param->length - 3;
        memcpy(rsp->data, &param->value[3], rsp->len);
        ke_msg_send(rsp);
    }
    return msg_status;
}

static int amsc_mtu_changed_ind_handler(ke_msg_id_t const msgid, struct gattc_mtu_changed_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    //ke_msg_forward(param, TASK_APP, src_id);
    return KE_MSG_CONSUMED;
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/// Default State handlers definition
KE_MSG_HANDLER_TAB(amsc)
{
    {AMSC_ENABLE_REQ,              (ke_msg_func_t)amsc_enable_req_handler},
    {AMSC_ENCRYPTED_CMD,           (ke_msg_func_t)amsc_encrypted_cmd_handler},
    {AMSC_REM_CTRL_CMD,            (ke_msg_func_t)amsc_rem_ctrl_cmd_handler},
    {GATTC_SDP_SVC_IND,            (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                (ke_msg_func_t)gattc_cmp_evt_handler},
    {GATTC_EVENT_IND,              (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_MTU_CHANGED_IND,        (ke_msg_func_t)amsc_mtu_changed_ind_handler},
};

void amsc_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);

    task_desc->msg_handler_tab = amsc_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(amsc_msg_handler_tab);
    task_desc->state           = amsc_env->state;
    task_desc->idx_max         = AMSC_IDX_MAX;
}


/// @} BASCTASK
