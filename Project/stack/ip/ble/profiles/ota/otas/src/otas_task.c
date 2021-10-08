/**
 ****************************************************************************************
 *
 * @file otas_task.c
 *
 * @brief OTAS Task implementation.
 *
 * Copyright (C) Huntersun 2011-2018
 *
 * Command/Return/Event:
 * +--------+--------+-------------------------+
 * | cmd(2) | len(2) |         data(n)         |
 * +--------+--------+-------------------------+
 * len == lengthof(data)
 *
 * 1. reboot:
 * -> 0000 0000
 *
 * 2. exchange key:
 * -> 0100 4000, e69d350e480103ccdbfdf4ac1191f4efb9a5f9e9a7832c5e2cbe97f2d203b020 8bd28915d08e1c742430ed8fc24563765c15525abf9a32636deb2a65499c80dc (ecc_xykey_t, 64bytes)
 * <- ...
 *
 * 3. conn param update:
 * -> 0200 0800, 0600 0600 0000 6400 (otas_conn_params_t, 8bytes)
 * <- 0200 0600, 0600 0000 6400 (otas_updated_conn_params_t, 6bytes)
 *
 * 4. mtu exchange:
 * -> 0300 0000
 * <- 0300 0600, 0502 ef01 ef01 (otas_updated_mtu_t, 6bytes)
 *
 * 5. length exchange:
 * -> 0400 0000
 * <- 0400 0800, fb00 fb00 ef01 ef01 (otas_updated_length_t, 8bytes)
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup OTASTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#include "co_utils.h"
#include "gattc_task.h"
#include "otas_task.h"
#include "otas.h"
#include "attm.h"
#include "prf_utils.h"
#include "co.h"
#include "peripheral.h"

#define OTAS_TASK_ID    prf_src_task_get(&(otas_env->prf_env), otas_env->conidx)
#define GAPC_TASK_ID    KE_BUILD_ID(TASK_GAPC, otas_env->conidx)
#define GATTC_TASK_ID   KE_BUILD_ID(TASK_GATTC, otas_env->conidx)

#define OTAS_LOG_RAW_DATA

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

#if OTAS_DEBUG_AUTO_SEND
/**
 * @brief otas_debug_auto_send()
 *
 * @return 
 **/
void otas_debug_auto_send(void)
{
    if(otas_env->is_debug_auto_send_start)
    {
        while(co_fifo_len(TX_DAT_FIFO) < OTAS_TX_DAT_BUFF_SIZE/2)
            otas_write((const uint8_t *)"1234567890", 10);
    }
}
#endif

#if OTAS_SECURITY

/**
 * @brief otas_encrypt()
 *
 * @param[in] from  
 * @param[in] to  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_encrypt(const uint8_t *from, uint8_t *to, uint16_t len)
{
    uint16_t i;

    for(i=0; i<len; ++i)
        to[i] = from[i] ^ otas_env->key.dh[otas_env->tx.count++ % sizeof(ecc_key_t)];
}

/**
 * @brief otas_decrypt()
 *
 * @param[in] from  
 * @param[in] to  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_decrypt(const uint8_t *from, uint8_t *to, uint16_t len)
{
    uint16_t i;

    for(i=0; i<len; ++i)
        to[i] = from[i] ^ otas_env->key.dh[otas_env->rx.count++ % sizeof(ecc_key_t)];
}

#endif

/**
 * @brief otas_gen_dh_key()
 *
 * @param[in] GAP_P256_KEY_LEN  
 * @param[in] GAP_P256_KEY_LEN  
 *
 * @return 
 **/
static void otas_gen_dh_key(uint8_t x[GAP_P256_KEY_LEN], uint8_t y[GAP_P256_KEY_LEN])
{
    // Prepare the GAPM_GEN_DH_KEY_CMD message
    struct gapm_gen_dh_key_cmd *cmd = KE_MSG_ALLOC(GAPM_GEN_DH_KEY_CMD,
                                                     TASK_GAPM, OTAS_TASK_ID,
                                                     gapm_gen_dh_key_cmd);

    // Command Operation Code (shall be GAPM_GEN_DH_KEY)
    cmd->operation = GAPM_GEN_DH_KEY;
    // X coordinate
    memcpy(cmd->operand_1, x, GAP_P256_KEY_LEN);
    // Y coordinate
    memcpy(cmd->operand_2, y, GAP_P256_KEY_LEN);

    // Send the message
    ke_msg_send(cmd);
}

/**
 * @brief otas_get_pub_key()
 *
 * @return 
 **/
static void otas_get_pub_key(void)
{
    // Prepare the GAPM_GET_PUB_KEY_CMD message
    struct gapm_get_pub_key_cmd *cmd = KE_MSG_ALLOC(GAPM_GET_PUB_KEY_CMD,
                                                     TASK_GAPM, OTAS_TASK_ID,
                                                     gapm_get_pub_key_cmd);

    // Command Operation Code (shall be GAPM_GET_PUB_KEY)
    cmd->operation = GAPM_GET_PUB_KEY;

    // Send the message
    ke_msg_send(cmd);
}

/**
 * @brief otas_conn_param_update()
 *
 * @param[in] conn_param  
 *
 * @return 
 **/
static void otas_conn_param_update(const otas_conn_params_t *conn_param)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     GAPC_TASK_ID, OTAS_TASK_ID,
                                                     gapc_param_update_cmd);

    cmd->operation = GAPC_UPDATE_PARAMS;
    cmd->intv_min  = conn_param->min_conn_interval;
    cmd->intv_max  = conn_param->max_conn_interval;
    cmd->latency   = conn_param->slave_latency;
    cmd->time_out  = conn_param->conn_sup_timeout;

    // not used by a slave device
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;

    // Send the message
    ke_msg_send(cmd);
}

/**
 * @brief otas_mtu_exchange()
 *
 * @return 
 **/
static void otas_mtu_exchange(void)
{
    struct gattc_exc_mtu_cmd *cmd = KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
                                                 GATTC_TASK_ID, OTAS_TASK_ID,
                                                 gattc_exc_mtu_cmd);

    cmd->operation = GATTC_MTU_EXCH;

    // Send the message
    ke_msg_send(cmd);
}

/**
 * @brief otas_length_exchange()
 *
 * @return 
 **/
static void otas_length_exchange(void)
{
    struct gapc_set_le_pkt_size_cmd *cmd = KE_MSG_ALLOC(GAPC_SET_LE_PKT_SIZE_CMD,
                                                        GAPC_TASK_ID, OTAS_TASK_ID,
                                                        gapc_set_le_pkt_size_cmd);

    cmd->operation = GAPC_SET_LE_PKT_SIZE;
    cmd->tx_octets = LE_MAX_OCTETS;
    cmd->tx_time   = LE_MAX_TIME;

    // Send the message
    ke_msg_send(cmd);
}

/**
 * @brief otas_disconnect()
 *
 * @return 
 **/
static void otas_disconnect(void)
{
    struct gapc_disconnect_cmd *cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                                   GAPC_TASK_ID, OTAS_TASK_ID,
                                                   gapc_disconnect_cmd);

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason = CO_ERROR_REMOTE_DEV_POWER_OFF;

    // Send the message
    ke_msg_send(cmd);
}

/**
 * @brief otas_reboot()
 *
 * @return 
 **/
void otas_reboot(void)
{
    __disable_irq();

    // Set flag
    HS_PMU->SW_STATUS |= PMU_SW_STATUS_REBOOT_FROM_OTA_ISP_MASK;

    // reboot
    pmu_force_reboot();
}

/**
 * @brief otas_perfect_once_tx_length()
 *
 * @param[in] mtu  
 * @param[in] mto  
 * @param[in] char_len  
 *
 * @return 
 **/
static uint16_t otas_perfect_once_tx_length(uint16_t mtu, uint16_t mto, uint16_t char_len)
{
    uint16_t mtux = MIN(OTAS_MTU_TO_NTF_WRTCMD_LEN(mtu), char_len);
    uint16_t mtox = OTAS_MTO_TO_NTF_WRTCMD_LEN(mto);

    return (mtux > mtox) ? (mtox + mto * ((mtux-mtox) / mto)) : (mtux);
}

/**
 * @brief otas_notify()
 *
 * @param[in] otas_idx  
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_notify(uint16_t otas_idx, const uint8_t *pdata, uint16_t len)
{
    uint16_t att_handle = OTAS_HANDLE(otas_idx);
    struct gattc_send_evt_cmd *req = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            GATTC_TASK_ID, OTAS_TASK_ID, gattc_send_evt_cmd, len);

    // Fill in the parameter structure
    req->operation = GATTC_NOTIFY;
    req->handle    = att_handle;
    req->length    = len;
    memcpy(req->value, pdata, len);

    // Send the event
    ke_msg_send(req);

    otas_env->is_notifying = true;
}

/**
 * @brief otas_notify_tx_cmd()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_notify_tx_cmd(const uint8_t *pdata, uint16_t len)
{
#ifdef OTAS_LOG_RAW_DATA
    otas_log_array_ex("-> tx-cmd", pdata, len);
#endif

    if(otas_env->tx_cmd_ntf_cfg & PRF_CLI_START_NTF)
        otas_notify(OTAS_IDX_TX_CMD_VAL, pdata, len);
    else
        otas_log("tx_cmd_ntf_cfg is disabled\n");
}

/**
 * @brief otas_notify_tx_dat()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_notify_tx_dat(uint8_t *pdata, uint16_t len)
{
#if OTAS_SECURITY
    otas_encrypt(pdata, pdata, len);
#endif

#ifdef OTAS_LOG_RAW_DATA
    otas_log_array_ex("-> tx-dat", pdata, len);
#endif

    if(otas_env->tx_dat_ntf_cfg & PRF_CLI_START_NTF)
        otas_notify(OTAS_IDX_TX_DAT_VAL, pdata, len);
    else
        otas_log("tx_dat_ntf_cfg is disabled\n");
}

/**
 * @brief otas_write_check()
 *
 * @return 
 **/
static void otas_write_check(void)
{
    uint8_t buff[OTAS_CHAR_MAX_LEN];
    uint16_t pkt_len;

    if(!co_fifo_is_empty(TX_CMD_FIFO))
    {
        pkt_len = co_fifo_out(TX_CMD_FIFO, buff, otas_env->perfect_once_tx_length);
        otas_notify_tx_cmd(buff, pkt_len);
    }
    else if(!co_fifo_is_empty(TX_DAT_FIFO))
    {
        pkt_len = co_fifo_out(TX_DAT_FIFO, buff, otas_env->perfect_once_tx_length);
        otas_notify_tx_dat(buff, pkt_len);
    }
}

/**
 * @brief otas_is_write_idle()
 *
 * @return 
 **/
static bool otas_is_write_idle(void)
{
    return co_fifo_is_empty(TX_CMD_FIFO) && co_fifo_is_empty(TX_DAT_FIFO) && !otas_env->is_notifying;
}

/**
 * @brief otas_cmd_write()
 *
 * @param[in] cmd  
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static bool otas_cmd_write(uint16_t cmd, const uint8_t *pdata, uint16_t len)
{
    otas_cmd_t otas_cmd;
    bool trigger = otas_is_write_idle();

    co_assert(len+OTAS_CMD_HEAD_LEN <= co_fifo_avail(TX_CMD_FIFO));

    if(len+OTAS_CMD_HEAD_LEN > co_fifo_avail(TX_CMD_FIFO))
        return false;

    otas_cmd.cmd = cmd;
    otas_cmd.len = len;

    co_fifo_in(TX_CMD_FIFO, (uint8_t *)&otas_cmd, OTAS_CMD_HEAD_LEN);
    co_fifo_in(TX_CMD_FIFO, pdata, len);

    if(trigger)
        otas_write_check();

    return true;
}

/**
 * @brief otas_cmd_reboot_handler()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_cmd_reboot_to_app_handler(const uint8_t *pdata, uint16_t len)
{
    otas_log("Disconnecting\n");

    otas_env->is_rebooting = true;

    otas_disconnect();
}

/**
 * @brief otas_cmd_exchange_key_handler()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_cmd_exchange_key_handler(const uint8_t *pdata, uint16_t len)
{
#if OTAS_SECURITY
    memcpy(&otas_env->key.remote, pdata, sizeof(ecc_xykey_t));

    otas_log_array_ex("Remote Public Key X", otas_env->key.remote.x, sizeof(ecc_key_t));
    otas_log_array_ex("Remote Public Key Y", otas_env->key.remote.y, sizeof(ecc_key_t));

    otas_get_pub_key();
#endif
}

/**
 * @brief otas_cmd_conn_param_update_handler()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_cmd_conn_param_update_handler(const uint8_t *pdata, uint16_t len)
{
    const otas_conn_params_t *params = (const otas_conn_params_t *)pdata;

    otas_log("Conn Param Updating: intv_min=%d intv_max=%d latency=%d timeout=%d\n",
            params->min_conn_interval, params->max_conn_interval, params->slave_latency, params->conn_sup_timeout);

    otas_conn_param_update(params);
}

/**
 * @brief otas_cmd_mtu_exchange_handler()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_cmd_mtu_exchange_handler(const uint8_t *pdata, uint16_t len)
{
    otas_log("MTU exchanging\n");

    otas_mtu_exchange();
}

/**
 * @brief otas_cmd_length_exchange_handler()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_cmd_length_exchange_handler(const uint8_t *pdata, uint16_t len)
{
    otas_log("Length exchanging\n");

    otas_length_exchange();
}

/**
 * @brief otas_on_rx_cmd_handler()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_on_rx_cmd_handler(const uint8_t *pdata, uint16_t len)
{
    otas_cmd_t otas_cmd;
    unsigned peek_len;
    uint8_t otas_cmd_data[OTAS_CHAR_MAX_LEN];

#ifdef OTAS_LOG_RAW_DATA
    otas_log_array_ex("<- rx-cmd", pdata, len);
#endif

    co_fifo_in(RX_CMD_FIFO, pdata, len);

    peek_len = co_fifo_peek(RX_CMD_FIFO, (uint8_t *)&otas_cmd, OTAS_CMD_HEAD_LEN);

    if(peek_len < OTAS_CMD_HEAD_LEN)
        return;

    co_assert(otas_cmd.len <= OTAS_CHAR_MAX_LEN);

    if(co_fifo_len(RX_CMD_FIFO) < otas_cmd.len+OTAS_CMD_HEAD_LEN)
        return;

    peek_len = co_fifo_out(RX_CMD_FIFO, (uint8_t *)&otas_cmd, OTAS_CMD_HEAD_LEN);
    co_assert(peek_len == OTAS_CMD_HEAD_LEN);

    if(otas_cmd.len > OTAS_CHAR_MAX_LEN)
    {
        co_fifo_reset(RX_CMD_FIFO);
    }
    else
    {
        const otas_cmd_handler_t otas_cmd_handler_table[] = {
            /* 0, OTAS_CMD_REBOOT_TO_APP     */ otas_cmd_reboot_to_app_handler,
            /* 1, OTAS_CMD_EXCHANGE_KEY      */ otas_cmd_exchange_key_handler,
            /* 2, OTAS_CMD_CONN_PARAM_UPDATE */ otas_cmd_conn_param_update_handler,
            /* 3, OTAS_CMD_MTU_EXCHANGE      */ otas_cmd_mtu_exchange_handler,
            /* 4, OTAS_CMD_LENGTH_EXCHANGE   */ otas_cmd_length_exchange_handler,
        };

        peek_len = co_fifo_out(RX_CMD_FIFO, otas_cmd_data, otas_cmd.len);
        co_assert(peek_len == otas_cmd.len);

        if(otas_cmd.cmd < countof(otas_cmd_handler_table))
            otas_cmd_handler_table[otas_cmd.cmd](otas_cmd_data, otas_cmd.len);
    }
}

/**
 * @brief otas_on_rx_dat_handler()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
static void otas_on_rx_dat_handler(uint8_t *pdata, uint16_t len)
{
#if OTAS_SECURITY
    otas_decrypt(pdata, pdata, len);
#endif

#ifdef OTAS_LOG_RAW_DATA
    otas_log_array_ex("<- rx-dat", pdata, len);
#endif

    if(otas_env->rx.packet_handler)
        otas_env->rx.packet_handler(pdata, len);

#if OTAS_DEBUG_AUTO_SEND
    if(otas_env->tx_dat_ntf_cfg & PRF_CLI_START_NTF)
    {
        otas_env->is_debug_auto_send_start = !otas_env->is_debug_auto_send_start;
        otas_debug_auto_send();
    }
#endif
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref  GATTC_WRITE_REQ_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int otas_gattc_write_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_write_req_ind *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t att_idx = OTAS_IDX(param->handle);
    struct gattc_write_cfm * cfm;

    switch (att_idx)
    {
        case OTAS_IDX_TX_CMD_NTF_CFG:
            otas_env->tx_cmd_ntf_cfg = co_read16p(&param->value[0]);
            otas_log("tx_cmd_ntf_cfg=0x%04X\n", otas_env->tx_cmd_ntf_cfg);
            break;

        case OTAS_IDX_TX_DAT_NTF_CFG:
            otas_env->tx_dat_ntf_cfg = co_read16p(&param->value[0]);
            otas_log("tx_dat_ntf_cfg=0x%04X\n", otas_env->tx_dat_ntf_cfg);
            break;

        case OTAS_IDX_RX_CMD_VAL:
            otas_on_rx_cmd_handler(param->value, param->length);
            break;

        case OTAS_IDX_RX_DAT_VAL:
            otas_on_rx_dat_handler(param->value, param->length);
            break;
    }

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    cfm->handle = param->handle;
    cfm->status = GAP_ERR_NO_ERROR;
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
static int otas_gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                            ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm *cfm;
    uint8_t att_idx = OTAS_IDX(param->handle);
    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t length = 0;
    uint8_t buffer[2];

    switch (att_idx)
    {
        case OTAS_IDX_TX_CMD_NTF_CFG:
            length = 2;
            co_write16p(buffer, otas_env->tx_cmd_ntf_cfg);
            break;

        case OTAS_IDX_TX_DAT_NTF_CFG:
            length = 2;
            co_write16p(buffer, otas_env->tx_dat_ntf_cfg);
            break;

        default:
            status = PRF_APP_ERROR;
            break;
    }

    //Send write response
    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
    cfm->handle = param->handle;
    cfm->status = status;
    cfm->length = length;

    memcpy(cfm->value, buffer, length);

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
static int otas_gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if (param->operation == GATTC_NOTIFY)
    {
        otas_env->is_notifying = false;

        otas_write_check();

#if OTAS_DEBUG_AUTO_SEND
        otas_debug_auto_send();
#endif
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles DH_KEY request
 * @param[in] msgid     Id of the message received.
 * @param[in] req       Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int otas_gapm_gen_dh_key_ind_handler(ke_msg_id_t const msgid,
                                          struct gapm_gen_dh_key_ind *param,
                                          ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    otas_log_array_ex("DHKey", param->result, GAP_P256_KEY_LEN);

    memcpy(otas_env->key.dh, param->result, GAP_P256_KEY_LEN);

    otas_cmd_write(OTAS_CMD_EXCHANGE_KEY, (uint8_t *)&otas_env->key.local, sizeof(ecc_xykey_t));

    return KE_MSG_CONSUMED;
}

/**
 ****************************************************************************************
 * @brief Handles PUB_KEY request
 * @param[in] msgid     Id of the message received.
 * @param[in] req       Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int otas_gapm_pub_key_ind_handler(ke_msg_id_t const msgid,
                                          struct gapm_pub_key_ind *param,
                                          ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    otas_log_array_ex("Local Public Key X", param->pub_key_x, GAP_P256_KEY_LEN);
    otas_log_array_ex("Local Public Key Y", param->pub_key_y, GAP_P256_KEY_LEN);

    memcpy(otas_env->key.local.x, param->pub_key_x, GAP_P256_KEY_LEN);
    memcpy(otas_env->key.local.y, param->pub_key_y, GAP_P256_KEY_LEN);

    otas_gen_dh_key(otas_env->key.remote.x, otas_env->key.remote.y);

    return KE_MSG_CONSUMED;
}

/**
 ****************************************************************************************
 * @brief Handle reception of GAPM_CMP_EVT message
 *
 * @param[in] msg_id    Id of the message received.
 * @param[in] param   Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int otas_gapm_cmp_evt_handler(ke_msg_id_t const msg_id, struct gapm_cmp_evt *param,
                                  ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    // Security operations
    if (param->status != GAP_ERR_NO_ERROR)
    {
        switch (param->operation)
        {
            case GAPM_GEN_DH_KEY:
            case GAPM_GET_PUB_KEY:
                memset(&otas_env->key.local, 0, sizeof(ecc_xykey_t));
                otas_cmd_write(OTAS_CMD_EXCHANGE_KEY, (uint8_t *)&otas_env->key.local, sizeof(ecc_xykey_t));
                otas_log("gapm_cmp_evt fail: GAPM_GET_PUB_KEY/GAPM_GEN_DH_KEY\n");
                break;

            default:
                otas_log("gapm_cmp_evt fail: %d", param->operation);
                break;
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of random number generated message
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int otas_gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid, struct gattc_mtu_changed_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    otas_updated_mtu_t updated_mtu;

    otas_env->mtu = param->mtu;
    otas_env->perfect_once_tx_length = otas_perfect_once_tx_length(otas_env->mtu, otas_env->mto, OTAS_CHAR_MAX_LEN);

    updated_mtu.mtu = otas_env->mtu;
    updated_mtu.perfect_once_tx_length = otas_env->perfect_once_tx_length;
    updated_mtu.perfect_once_rx_length = otas_perfect_once_tx_length(otas_env->mtu, otas_env->mro, OTAS_CHAR_MAX_LEN);

    otas_log("MTU=%d perfect_once_tx_length=%d perfect_once_rx_length=%d\n", otas_env->mtu, otas_env->perfect_once_tx_length, updated_mtu.perfect_once_rx_length);

    otas_cmd_write(OTAS_CMD_MTU_EXCHANGE, (uint8_t *)&updated_mtu, sizeof(otas_updated_mtu_t));

    return KE_MSG_CONSUMED;
}

/**
 * @brief otas_gapc_pkt_size_ind_handler()
 *
 * @param[in] msgid  
 * @param[in] param  
 * @param[in] dest_id  
 * @param[in] src_id  
 *
 * @return 
 **/
int otas_gapc_pkt_size_ind_handler(ke_msg_id_t const msgid,  struct gapc_le_pkt_size_ind const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    otas_updated_length_t updated_length;

    otas_env->mto = param->max_tx_octets;
    otas_env->mro = param->max_rx_octets;
    otas_env->perfect_once_tx_length = otas_perfect_once_tx_length(otas_env->mtu, otas_env->mto, OTAS_CHAR_MAX_LEN);

    updated_length.mto = param->max_tx_octets;
    updated_length.mro = param->max_rx_octets;
    updated_length.perfect_once_tx_length = otas_env->perfect_once_tx_length;
    updated_length.perfect_once_rx_length = otas_perfect_once_tx_length(otas_env->mtu, otas_env->mro, OTAS_CHAR_MAX_LEN);

    otas_log("MTO=%d MRO=%d perfect_once_tx_length=%d perfect_once_rx_length=%d\n", otas_env->mto, otas_env->mro, otas_env->perfect_once_tx_length, updated_length.perfect_once_rx_length);

    otas_cmd_write(OTAS_CMD_LENGTH_EXCHANGE, (uint8_t *)&updated_length, sizeof(otas_updated_length_t));

    return (KE_MSG_CONSUMED);
}

/**
 * @brief otas_gapc_param_update_ind_handler()
 *
 * @param[in] msgid  
 * @param[in] param  
 * @param[in] dest_id  
 * @param[in] src_id  
 *
 * @return 
 **/
int otas_gapc_param_update_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_param_updated_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    otas_updated_conn_params_t updated_params;

    updated_params.conn_interval    = param->con_interval;
    updated_params.slave_latency    = param->con_latency;
    updated_params.conn_sup_timeout = param->sup_to;

    otas_log("Conn Param Updated: intv=%d latency=%d timeout=%d\n",
            param->con_interval, param->con_latency, param->sup_to);

    otas_cmd_write(OTAS_CMD_CONN_PARAM_UPDATE, (uint8_t *)&updated_params, sizeof(otas_updated_conn_params_t));

    return (KE_MSG_CONSUMED);
}

/**
 * @brief otas_gapc_disconnect_ind_handler()
 *
 * @param[in] msgid  
 * @param[in] param  
 * @param[in] dest_id  
 * @param[in] src_id  
 *
 * @return 
 **/
int otas_gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if(otas_env->is_rebooting)
    {
        otas_log("Rebooting\n");
        otas_reboot();
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles GAP controller command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int otas_gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        case (GAPC_UPDATE_PARAMS):
            if (param->status != GAP_ERR_NO_ERROR)
            {
                otas_log("Conn Param Update fail, status=0x%02x\n", param->status);
            }
            break;

        default:
            break;
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(otas)
{
    {GAPM_GEN_DH_KEY_IND,       (ke_msg_func_t)otas_gapm_gen_dh_key_ind_handler},
    {GAPM_PUB_KEY_IND,          (ke_msg_func_t)otas_gapm_pub_key_ind_handler},
    {GAPM_CMP_EVT,              (ke_msg_func_t)otas_gapm_cmp_evt_handler},
    {GATTC_MTU_CHANGED_IND,     (ke_msg_func_t)otas_gattc_mtu_changed_ind_handler},
    {GAPC_LE_PKT_SIZE_IND,      (ke_msg_func_t)otas_gapc_pkt_size_ind_handler},
    {GAPC_PARAM_UPDATED_IND,    (ke_msg_func_t)otas_gapc_param_update_ind_handler},
    {GAPC_DISCONNECT_IND,       (ke_msg_func_t)otas_gapc_disconnect_ind_handler},
    {GAPC_CMP_EVT,              (ke_msg_func_t)otas_gapc_cmp_evt_handler},

    {GATTC_WRITE_REQ_IND,       (ke_msg_func_t)otas_gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,        (ke_msg_func_t)otas_gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,             (ke_msg_func_t)otas_gattc_cmp_evt_handler},
};

void otas_task_init(struct ke_task_desc *task_desc)
{
    // Get the address of the environment
    task_desc->msg_handler_tab = otas_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(otas_msg_handler_tab);
    task_desc->state           = otas_env->state;
    task_desc->idx_max         = 1;
}

/**
 * @brief otas_write()
 *
 * @param[in] pdata  
 * @param[in] len  
 *
 * @return 
 **/
bool otas_write(const uint8_t *pdata, uint16_t len)
{
    bool trigger = otas_is_write_idle();

    if(len > co_fifo_avail(TX_DAT_FIFO))
        return false;

    co_fifo_in(TX_DAT_FIFO, pdata, len);

    if(trigger)
        otas_write_check();

    return true;
}

/// @} OTASTASK
