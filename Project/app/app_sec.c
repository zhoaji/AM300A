/**
 ****************************************************************************************
 *
 * @file app_sec.c
 * @brief about Security setting.
 * @date Mon, Jan  7, 2019  4:31:27 PM
 * @author chenzhiyuan
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_COMMON_SEC_C app_sec.c
 * @ingroup APP_COMMON
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#include <string.h>
#include "co_utils.h"
#include "co_math.h"

#if (BLE_APP_SEC)
#include "gapc_task.h"      // GAP Controller Task API Definition
#include "gap.h"            // GAP Definition
#include "gapc.h"           // GAPC Definition
#include "prf_types.h"

#include "app.h"            // Application API Definition
#include "app_sec.h"        // Application Security API Definition
#include "app_task.h"       // Application Manager API Definition

#if (DISPLAY_SUPPORT)
#include "app_display.h"    // Display Application Definitions
#endif //(DISPLAY_SUPPORT)

#if (NVDS_SUPPORT)
#include "nvds.h"           // NVDS API Definitions
#endif //(NVDS_SUPPORT)

#if (BLE_APP_AM0)
#include "app_am0.h"
#endif //(BLE_APP_AM0)
#include "co_debug.h"

#if (BLE_APP_HID)
#include "app_hid.h"
#endif //(BLE_APP_HID)

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Security Environment Structure
struct app_sec_env_tag app_sec_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_sec_init()
{
    memset(&app_sec_env, 0, sizeof(app_sec_env));
    /*------------------------------------------------------
     * RETRIEVE BOND STATUS
     *------------------------------------------------------*/
    #if (NVDS_SUPPORT)
    struct app_sec_conn_info_tag info;
    nvds_tag_len_t length = sizeof(struct app_sec_conn_info_tag);
    uint8_t index = 0;
    uint8_t nvds_len = NVDS_TAG_BLE_LINK_KEY_LAST-NVDS_TAG_BLE_LINK_KEY_FIRST;

    app_sec_env.bonded = false;

    // Get bond status from NVDS
    for(index =0; index <= nvds_len; index++)
    {
        length = sizeof(struct app_sec_conn_info_tag);
        memset(&info, 0, length);
        if (nvds_get(NVDS_TAG_BLE_LINK_KEY_FIRST+index, &length, (uint8_t *)&info) == NVDS_OK)
        {
            app_sec_env.bonded = true;
            break;
        }
    }
    #endif //(NVDS_SUPPORT)
}

/**
 ****************************************************************************************
 * @brief Return if the device is currently bonded
 ****************************************************************************************
 */
bool app_sec_get_bond_status_by_addr(bd_addr_t addr)
{
    #if (NVDS_SUPPORT)
    struct app_sec_conn_info_tag info;
    nvds_tag_len_t length = sizeof(struct app_sec_conn_info_tag);
    uint8_t index = 0;
    uint8_t nvds_len = NVDS_TAG_BLE_LINK_KEY_LAST-NVDS_TAG_BLE_LINK_KEY_FIRST;

    // Get bond status from NVDS
    for(index =0; index <= nvds_len; index++)
    {
        length = sizeof(struct app_sec_conn_info_tag);
        memset(&info, 0, length);
        if (nvds_get(NVDS_TAG_BLE_LINK_KEY_FIRST+index, &length, (uint8_t *)&info) == NVDS_OK)
        {
            if (memcmp(&info.id_addr.addr, &addr, sizeof(bd_addr_t)) == 0)
            {
                return true;
            }
        }
    }
    #endif //(NVDS_SUPPORT)
    return false;
}


bool app_sec_get_bond_status(void)
{
    return app_sec_env.bonded;
}

#if (NVDS_SUPPORT)
void app_sec_remove_bond(void)
{
    #if (BLE_APP_HID)
    uint16_t ntf_cfg = PRF_CLI_STOP_NTFIND;
    #endif //(BLE_APP_HID)

    // Check if we are well bonded
    if (app_sec_env.bonded == true)
    {
        // Update the environment variable
        app_sec_env.bonded = false;

        uint8_t index = 0;
        uint8_t nvds_len = NVDS_TAG_BLE_LINK_KEY_LAST-NVDS_TAG_BLE_LINK_KEY_FIRST;

        for(index =0; index <= nvds_len; index++)
        {
            nvds_del(NVDS_TAG_BLE_LINK_KEY_FIRST+index) ;
        }


#if (BLE_APP_HID)
        if (nvds_put(NVDS_TAG_MOUSE_NTF_CFG, NVDS_LEN_MOUSE_NTF_CFG,
                    (uint8_t *)&ntf_cfg) != NVDS_OK)
        {
            ASSERT_ERR(0);
        }
#endif //(BLE_APP_HID)
    }
    #if (BLE_APP_SEC_CON)
        memset(&app_sec_env, 0, sizeof(app_sec_env));
    #endif
}
#endif //(NVDS_SUPPORT)

void app_sec_send_security_req(uint8_t conidx)
{
    // Send security request
    struct gapc_security_cmd *cmd = KE_MSG_ALLOC(GAPC_SECURITY_CMD,
                                                 KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                                 gapc_security_cmd);

    cmd->operation = GAPC_SECURITY_REQ;
    cmd->auth      = GAP_AUTH_REQ_NO_MITM_BOND;

    // Send the message
    ke_msg_send(cmd);
}


void app_sec_bond_req(uint8_t conidx)
{
    struct gapc_bond_cmd *cmd = KE_MSG_ALLOC(GAPC_BOND_CMD,
                                                 KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                                 gapc_bond_cmd);

    cmd->operation = GAPC_BOND;
    /// IO capabilities (@see gap_io_cap)
    cmd->pairing.iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    cmd->pairing.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    /// Authentication (@see gap_auth)
    /// Note in BT 4.1 the Auth Field is extended to include 'Key Notification' and
    /// and 'Secure Connections'.
    cmd->pairing.auth = GAP_AUTH_REQ_NO_MITM_BOND;
    /// Encryption key size (7 to 16)
    cmd->pairing.key_size = 16;
    ///Initiator key distribution (@see gap_kdist)
    cmd->pairing.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;
    ///Responder key distribution (@see gap_kdist)
    cmd->pairing.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;

    /// Device security requirements (minimum security level). (@see gap_sec_req)
    cmd->pairing.sec_req = GAP_NO_SEC;
    // Send the message
    ke_msg_send(cmd);
}

void app_sec_encrypt_req(uint8_t conidx, bd_addr_t peer_addr)
{
    struct gapc_encrypt_cmd *cmd = KE_MSG_ALLOC(GAPC_ENCRYPT_CMD,
                                                 KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                                 gapc_encrypt_cmd);

    cmd->operation = GAPC_ENCRYPT;
    #if (NVDS_SUPPORT)
    struct app_sec_conn_info_tag info;
    nvds_tag_len_t length = sizeof(struct app_sec_conn_info_tag);
    uint8_t index = 0;
    uint8_t nvds_len = NVDS_TAG_BLE_LINK_KEY_LAST-NVDS_TAG_BLE_LINK_KEY_FIRST;

    // Get bond status from NVDS
    for(index =0; index <= nvds_len; index++)
    {
        length = sizeof(struct app_sec_conn_info_tag);
        memset(&info, 0, length);
        if (nvds_get(NVDS_TAG_BLE_LINK_KEY_FIRST+index, &length, (uint8_t *)&info) == NVDS_OK)
        {
            if (memcmp(&info.id_addr.addr, &peer_addr, sizeof(bd_addr_t)) == 0)
            {
                memcpy(&cmd->ltk, &info.ltk, sizeof(struct gapc_ltk));
                break;
            }
        }
    }
    #endif //(NVDS_SUPPORT)
    
    // Send the message
    ke_msg_send(cmd);
}


/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */

static int gapc_bond_req_ind_handler(ke_msg_id_t const msgid,
                                     struct gapc_bond_req_ind const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    //log_debug("%s@%d, request=%x\n", __func__, __LINE__, param->request);

    // Prepare the GAPC_BOND_CFM message
    struct gapc_bond_cfm *cfm = KE_MSG_ALLOC(GAPC_BOND_CFM,
                                             src_id, TASK_APP,
                                             gapc_bond_cfm);

    switch (param->request)
    {
        case (GAPC_PAIRING_REQ):
        {
            cfm->request = GAPC_PAIRING_RSP;

            #ifndef BLE_APP_AM0
            cfm->accept  = false;

            // Check if we are already bonded (Only one bonded connection is supported)
            if (!app_sec_env.con_bonded[conidx])
            #endif // !BLE_APP_AM0
            {
                cfm->accept  = true;

                #if (BLE_APP_HID || BLE_APP_HT)
                // Pairing Features
                cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_MITM_BOND;
                #elif defined(BLE_APP_AM0)
                #if (BLE_APP_SEC_CON == 1)
                if (param->data.auth_req & GAP_AUTH_SEC_CON)
                {
                    cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_SEC_CON_BOND;
                    app_sec_env.sec_con_enabled[conidx] = true;
                }
                else
                {
                    cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_NO_MITM_BOND;
                    app_sec_env.sec_con_enabled[conidx] = false;
                }
                #else  // !(BLE_APP_SEC_CON)
                cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_NO_MITM_BOND;
                app_sec_env.sec_con_enabled[conidx] = false;
                #endif // (BLE_APP_SEC_CON)
                #else
                cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_NO_MITM_BOND;
                #endif //(BLE_APP_HID || BLE_APP_HT)

                #if (BLE_APP_HT)
                cfm->data.pairing_feat.iocap     = GAP_IO_CAP_DISPLAY_ONLY;
                #else
                cfm->data.pairing_feat.iocap     = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
                #endif //(BLE_APP_HT)

                cfm->data.pairing_feat.key_size  = 16;
                cfm->data.pairing_feat.oob       = GAP_OOB_AUTH_DATA_NOT_PRESENT;
                cfm->data.pairing_feat.sec_req   = GAP_NO_SEC;

                #if (defined(BLE_APP_AM0))
                cfm->data.pairing_feat.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;
                #if BLE_APP_AM0
                // No Keys required from the Initiator
                cfm->data.pairing_feat.ikey_dist = 0;
                #else
                cfm->data.pairing_feat.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;
                #endif
                #else
#if (BLE_APP_PRIVACY)
                cfm->data.pairing_feat.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;
                cfm->data.pairing_feat.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;
#else
                cfm->data.pairing_feat.ikey_dist = GAP_KDIST_IDKEY;
                cfm->data.pairing_feat.rkey_dist = GAP_KDIST_ENCKEY;
                cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_MITM_BOND;
#endif
                #endif // (defined(BLE_APP_AM0))
            }
        } break;

        case (GAPC_LTK_EXCH):
        {
            // Counter
            uint8_t counter;

            cfm->accept  = true;
            cfm->request = GAPC_LTK_EXCH;

            // Generate all the values
            cfm->data.ltk.ediv = (uint16_t)co_rand_word();

            for (counter = 0; counter < RAND_NB_LEN; counter++)
            {
                cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
                cfm->data.ltk.randnb.nb[counter] = (uint8_t)co_rand_word();
            }

            for (counter = RAND_NB_LEN; counter < KEY_LEN; counter++)
            {
                cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
            }

            #if (0)// (NVDS_SUPPORT)
            if (nvds_put(NVDS_TAG_LTK, NVDS_LEN_LTK,
                         (uint8_t *)&cfm->data.ltk) != NVDS_OK)
            {
                ASSERT_ERR(0);
            }
            #else
            memcpy(&app_sec_env.info[conidx].ltk, &cfm->data.ltk, sizeof(struct gapc_ltk));
            memcpy(&app_sec_env.info[conidx].ltk.randnb.nb[0], &cfm->data.ltk.randnb.nb[0], RAND_NB_LEN);
            app_sec_env.info[conidx].ltk.ediv = cfm->data.ltk.ediv;
            #endif // #if (NVDS_SUPPORT)
        } break;


        case (GAPC_IRK_EXCH):
        {
            #if (NVDS_SUPPORT)
            nvds_tag_len_t addr_len = BD_ADDR_LEN;
            #endif //(NVDS_SUPPORT)

            cfm->accept  = true;
            cfm->request = GAPC_IRK_EXCH;

            // Load IRK
            memcpy(cfm->data.irk.irk.key, app_env.loc_irk, KEY_LEN);

            #if (NVDS_SUPPORT)
            if (nvds_get(NVDS_TAG_BD_ADDRESS, &addr_len, cfm->data.irk.addr.addr.addr) != NVDS_OK)
            #endif //(NVDS_SUPPORT)
            {
                //ASSERT_ERR(0);
                memcpy(cfm->data.irk.addr.addr.addr, &co_default_bdaddr,sizeof(co_default_bdaddr));
            }
            // load device address
            cfm->data.irk.addr.addr_type = (cfm->data.irk.addr.addr.addr[5] & 0xC0) ? ADDR_RAND : ADDR_PUBLIC;
        } break;


        //#if (BLE_APP_HT)
        case (GAPC_TK_EXCH):
        {
            // Generate a PIN Code- (Between 100000 and 999999)
            uint32_t pin_code = (100000 + (co_rand_word()%900000));

            cfm->accept  = true;
            cfm->request = GAPC_TK_EXCH;

            // Set the TK value
            memset(cfm->data.tk.key, 0, KEY_LEN);

            cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
            cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
            cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
            cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);
        } break;
        //#endif //(BLE_APP_HT)
        case (GAPC_NC_EXCH):
        {
            cfm->accept  = true;
            cfm->request = GAPC_NC_EXCH;
        } break;

        default:
        {
            ASSERT_ERR(0);
        } break;
    }

    // Send the message
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

static int gapc_bond_ind_handler(ke_msg_id_t const msgid,
                                 struct gapc_bond_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);

    //log_debug("%s@%d, info=%x\n", __func__, __LINE__, param->info);

    switch (param->info)
    {
        case (GAPC_PAIRING_SUCCEED):
        {
            // Update the bonding status in the environment
            app_sec_env.bonded = true;
            app_sec_env.con_bonded[conidx] = true;

            // Update the bonding status in the environment
            #if (0)//(PLF_NVDS)
            if (nvds_put(NVDS_TAG_PERIPH_BONDED, NVDS_LEN_PERIPH_BONDED,
                         (uint8_t *)&app_sec_env.bonded) != NVDS_OK)
            {
                // An error has occurred during access to the NVDS
                ASSERT_ERR(0);
            }

            // Set the BD Address of the peer device in NVDS
            if (nvds_put(NVDS_TAG_PEER_BD_ADDRESS, NVDS_LEN_PEER_BD_ADDRESS,
                         (uint8_t *)gapc_get_bdaddr(conidx, SMPC_INFO_PEER)) != NVDS_OK)
            {
                // An error has occurred during access to the NVDS
                ASSERT_ERR(0);
            }
            #else
#if (NVDS_SUPPORT)
            struct app_sec_conn_info_tag info;
            nvds_tag_len_t length = sizeof(struct app_sec_conn_info_tag);
            uint8_t index = 0;
            uint8_t nvds_len = NVDS_TAG_BLE_LINK_KEY_LAST-NVDS_TAG_BLE_LINK_KEY_FIRST;

            memcpy(&app_sec_env.info[conidx].id_addr, (uint8_t *)gapc_get_bdaddr(conidx, SMPC_INFO_PEER), sizeof(struct gap_bdaddr));
            //log_debug("rom,%x,\r\n", app_sec_env.info[conidx].ltk.ediv);
            //log_debug_array_ex("nb,", &app_sec_env.info[conidx].ltk.randnb.nb[0], GAP_RAND_NB_LEN);
            // Get bond status from NVDS
            for(index =0; index <= nvds_len; index++)
            {
                length = sizeof(struct app_sec_conn_info_tag);
                memset(&info, 0, length);
                if (nvds_get(NVDS_TAG_BLE_LINK_KEY_FIRST+index, &length, (uint8_t *)&info) != NVDS_OK)
                {
                    length = sizeof(struct app_sec_conn_info_tag);
                    nvds_put(NVDS_TAG_BLE_LINK_KEY_FIRST+index, length, (uint8_t*)&app_sec_env.info[conidx]);
                    break;
                }
                else
                {
                     //log_debug("get,i=%d,%x,\r\n", index, info.ltk.ediv);
                     //log_debug_array_ex("nb,", &info.ltk.randnb.nb[0], GAP_RAND_NB_LEN);
                }
            }
            if (index > nvds_len)
            {
                nvds_del(NVDS_TAG_BLE_LINK_KEY_FIRST);
                length = sizeof(struct app_sec_conn_info_tag);
                nvds_put(NVDS_TAG_BLE_LINK_KEY_FIRST, length, (uint8_t*)&app_sec_env.info[conidx]);
            }
#endif
            #endif //(PLF_NVDS)

            #if (BLE_APP_ANCSC)
            ancsc_encrypted_cmd(KE_IDX_GET(src_id));
            #endif
            #if (BLE_APP_AMSC)
            amsc_encrypted_cmd(KE_IDX_GET(src_id));
            #endif
            #if (BLE_APP_AM0)
            app_am0_send_audio_init(KE_IDX_GET(src_id));
            #endif //(BLE_APP_AM0)
            #if (BLE_APP_WHITE_LIST==1 && BLE_APP_PRIVACY == 0)
            appm_whl_add_device((uint8_t *)gapc_get_bdaddr(0, SMPC_INFO_PEER));
            #endif
            #if (BLE_APP_HID)
            // Enable HID Service
            app_hid_enable_prf(app_env.conidx);
            #endif //(BLE_APP_HID)
        } break;

        case (GAPC_REPEATED_ATTEMPT):
        {
            appm_disconnect(KE_IDX_GET(src_id));
        } break;

        case (GAPC_IRK_EXCH):
        {
#if (NVDS_SUPPORT)
            // Store peer identity in NVDS
            //if (nvds_put(NVDS_TAG_PEER_IRK, NVDS_LEN_PEER_IRK, (uint8_t *)&param->data.irk) != NVDS_OK)
            //{
            //    ASSERT_ERR(0);
            //}
            memcpy(&app_sec_env.info[conidx].irk, (uint8_t *)&param->data.irk, sizeof(struct gapc_irk));
#endif // (NVDS_SUPPORT)
            //log_debug_array_ex("irk ", (uint8_t *)&param->data.irk, NVDS_LEN_PEER_IRK);
            //log_debug_array_ex("local irk ", app_env.loc_irk, KEY_LEN);
#if (BLE_APP_WHITE_LIST)
            appm_whl_add_device((uint8_t *)&param->data.irk.addr);
#endif
#if (BLE_APP_PRIVACY)
            struct gap_ral_dev_info device;
            memcpy(&device.addr, &param->data.irk.addr, sizeof(struct gap_bdaddr));
            device.priv_mode = 1;
            memcpy(device.peer_irk, param->data.irk.irk.key, GAP_KEY_LEN);
            memcpy(device.local_irk, app_env.loc_irk, GAP_KEY_LEN);
            appm_privacy_add_device((uint8_t*)&device);
#endif
        } break;

        case (GAPC_PAIRING_FAILED):
        {
            //app_sec_send_security_req(0);
            log_debug("pairing failed reason=%x\n", param->data.reason);
        } break;

        // In Secure Connections we get BOND_IND with SMPC calculated LTK
        case (GAPC_LTK_EXCH) :
        {
            #if (BLE_APP_SEC_CON)
            //del the if , when master pair, ,edir & nb = 0;
            //if (app_sec_env.sec_con_enabled[conidx] == true)
            {
                #if (NVDS_SUPPORT)
                // Store LTK in NVDS
                //if (nvds_put(NVDS_TAG_LTK, NVDS_LEN_LTK,(uint8_t *)&param->data.ltk.ltk.key[0]) != NVDS_OK)
                //{
                //    ASSERT_ERR(0);
                //}
                memcpy(&app_sec_env.info[conidx].ltk, (uint8_t *)&param->data.ltk, sizeof(struct gapc_ltk));
                #endif // (NVDS_SUPPORT)
            }
            #endif // (BLE_APP_SEC_CON)
        }
        break;

        default:
        {
            ASSERT_ERR(0);
        } break;
    }

    return (KE_MSG_CONSUMED);
}

static int gapc_bond_cfm_handler(ke_msg_id_t const msgid,
                                        struct gapc_bond_cfm const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    log_debug("%s@%d\n", __func__, __LINE__);
    log_debug("request=%x,accept=%x\n", param->request, param->accept);
    //union gapc_bond_cfm_data data;
    //
    return (KE_MSG_CONSUMED);
}

static int gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
                                        struct gapc_encrypt_req_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);

    // Prepare the GAPC_ENCRYPT_CFM message
    struct gapc_encrypt_cfm *cfm = KE_MSG_ALLOC(GAPC_ENCRYPT_CFM,
                                                src_id, TASK_APP,
                                                gapc_encrypt_cfm);

    cfm->found    = false;
    if (app_sec_env.con_bonded[conidx])
    {
        if ((param->ediv == app_sec_env.info[conidx].ltk.ediv) &&
                !memcmp(&param->rand_nb.nb[0], &app_sec_env.info[conidx].ltk.randnb.nb[0], sizeof(struct rand_nb)))
            {
                cfm->found    = true;
                cfm->key_size = 16;
                memcpy(&cfm->ltk, &app_sec_env.info[conidx].ltk.ltk, sizeof(struct gap_sec_key));
            }
    }
    else if (app_sec_env.bonded)
    {
#if (NVDS_SUPPORT)
        struct app_sec_conn_info_tag info;
        nvds_tag_len_t length = sizeof(struct app_sec_conn_info_tag);
        uint8_t index = 0;
        uint8_t nvds_len = NVDS_TAG_BLE_LINK_KEY_LAST-NVDS_TAG_BLE_LINK_KEY_FIRST;

        //app_sec_env.bonded = false;
        //log_debug("ori,%x,\r\n", param->ediv);
        //log_debug_array_ex("nb1,", &param->rand_nb.nb[0], GAP_RAND_NB_LEN);

        // Get bond status from NVDS
        for(index =0; index <= nvds_len; index++)
        {
            length = sizeof(struct app_sec_conn_info_tag);
            memset(&info, 0, length);
            if (nvds_get(NVDS_TAG_BLE_LINK_KEY_FIRST+index, &length, (uint8_t *)&info) == NVDS_OK)
            {
                if ((param->ediv == info.ltk.ediv) &&
                        !memcmp(&param->rand_nb.nb[0], &info.ltk.randnb.nb[0], sizeof(struct rand_nb)))
                {
                    cfm->found    = true;
                    cfm->key_size = 16;
                    memcpy(&cfm->ltk, &info.ltk.ltk, sizeof(struct gap_sec_key));
                    break;
                }
                else
                {
                    //log_debug("ok,i=%d,%x,\r\n", index, info.ltk.ediv);
                    //log_debug_array_ex("nb,", &info.ltk.randnb.nb[0], GAP_RAND_NB_LEN);
                }
            }
            else
            {
                //log_debug("fail,i=%d,%x,\r\n", index, info.ltk.ediv);
                //log_debug_array_ex("nb,", &info.ltk.randnb.nb[0], GAP_RAND_NB_LEN);
            }
        }

#endif // #if (NVDS_SUPPORT)
    }
    /*
     * else the peer device is not known, an error should trigger a new pairing procedure.
     */

    // Send the message
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}


static int gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
                                    struct gapc_encrypt_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // encryption/ re-encryption succeeded

    #if (BLE_APP_AM0)
    // Need to Setup Authenicated Payload TO for the connection.
    struct gapc_set_le_ping_to_cmd *cmd = KE_MSG_ALLOC(GAPC_SET_LE_PING_TO_CMD,
                                                       KE_BUILD_ID(TASK_GAPC, KE_IDX_GET(src_id)), TASK_APP,
                                                       gapc_set_le_ping_to_cmd);

    // encryption/ re-encryption succeeded

    app_am0_send_audio_init(KE_IDX_GET(src_id));

    cmd->operation = GAPC_SET_LE_PING_TO;
    cmd->timeout = 1000; // 10 Sec

    // Send the message
    ke_msg_send(cmd);
    #endif //(BLE_APP_AM0)
    #if (BLE_APP_ANCSC)
    ancsc_encrypted_cmd(KE_IDX_GET(src_id));
    #endif
    #if (BLE_APP_AMSC)
    amsc_encrypted_cmd(KE_IDX_GET(src_id));
    #endif
    #if (BLE_APP_HID)
    // Enable HID Service
    app_hid_enable_prf(app_env.conidx);
    #endif //(BLE_APP_HID)
    return (KE_MSG_CONSUMED);
}

static int app_sec_msg_dflt_handler(ke_msg_id_t const msgid,
                                    void *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Drop the message

    return (KE_MSG_CONSUMED);
}

int app_sec_gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                    void *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    //app_sec_env.bonded = false;

    if (conidx < BLE_CONNECTION_MAX) {
#if (BLE_APP_SEC_CON)
    app_sec_env.sec_con_enabled[conidx] = false;
#endif
    app_sec_env.con_bonded[conidx] = false;
    }
    return (KE_MSG_CONSUMED);
}
 /*
  * LOCAL VARIABLE DEFINITIONS
  ****************************************************************************************
  */

/// Default State handlers definition
const struct ke_msg_handler app_sec_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,  (ke_msg_func_t)app_sec_msg_dflt_handler},

    {GAPC_BOND_REQ_IND,       (ke_msg_func_t)gapc_bond_req_ind_handler},
    {GAPC_BOND_IND,           (ke_msg_func_t)gapc_bond_ind_handler},
    {GAPC_BOND_CFM,           (ke_msg_func_t)gapc_bond_cfm_handler},

    {GAPC_ENCRYPT_REQ_IND,    (ke_msg_func_t)gapc_encrypt_req_ind_handler},
    {GAPC_ENCRYPT_IND,        (ke_msg_func_t)gapc_encrypt_ind_handler},
};

const struct app_subtask_handlers app_sec_handlers = {&app_sec_msg_handler_list[0], ARRAY_LEN(app_sec_msg_handler_list)};

#else
/**
 ****************************************************************************************
 * @brief Return if the device is currently bonded in the flash
 * @return if bonded return true, else return false.
 ****************************************************************************************
 */
bool app_sec_get_bond_status(void)
{
    return 0;
}

#endif //(BLE_APP_SEC)

/// @} APP
