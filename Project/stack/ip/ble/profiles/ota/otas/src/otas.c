/**
 ****************************************************************************************
 *
 * @file otas.c
 *
 * @brief Battery Server Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup OTAS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#include "otas.h"
#include "otas_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "co.h"
#include "ke_mem.h"

/*
 * BAS ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

#define OTAS_TIMEOUT_TIME       (5*60*1000)

#define ATT_CHAR_OTAS_RX_CMD    ATT_UUID_16(0xFF01)
#define ATT_CHAR_OTAS_RX_DAT    ATT_UUID_16(0xFF02)
#define ATT_CHAR_OTAS_TX_CMD    ATT_UUID_16(0xFF03)
#define ATT_CHAR_OTAS_TX_DAT    ATT_UUID_16(0xFF04)

/// Full BAS Database Description - Used to add attributes into the database
static const struct attm_desc_128 otas_att_db[OTAS_IDX_NB] =
{
    // OTA Service Declaration
    [OTAS_IDX_SVC]              = {ATT_16_TO_128_ARRAY(ATT_DECL_PRIMARY_SERVICE),  PERM(RD, ENABLE), 0, 0},

    // OTA RX CMD Characteristic Declaration
    [OTAS_IDX_RX_CMD_CHAR]      = {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // OTA RX CMD Characteristic Value
    [OTAS_IDX_RX_CMD_VAL]       = {ATT_16_TO_128_ARRAY(ATT_CHAR_OTAS_RX_CMD),      PERM(WRITE_COMMAND, ENABLE), 0, OTAS_CHAR_MAX_LEN},

    // OTA RX DAT Characteristic Declaration
    [OTAS_IDX_RX_DAT_CHAR]      = {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // OTA RX DAT Characteristic Value
    [OTAS_IDX_RX_DAT_VAL]       = {ATT_16_TO_128_ARRAY(ATT_CHAR_OTAS_RX_DAT),      PERM(WRITE_COMMAND, ENABLE), 0, OTAS_CHAR_MAX_LEN},

    // OTA TX CMD Characteristic Declaration
    [OTAS_IDX_TX_CMD_CHAR]      = {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // OTA TX CMD Characteristic Value
    [OTAS_IDX_TX_CMD_VAL]       = {ATT_16_TO_128_ARRAY(ATT_CHAR_OTAS_TX_CMD),      PERM(NTF, ENABLE), 0, OTAS_CHAR_MAX_LEN},
    // OTA TX CMD Characteristic - Client Characteristic Configuration Descriptor
    [OTAS_IDX_TX_CMD_NTF_CFG]   = {ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG),  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

    // OTA TX CMD Characteristic Declaration
    [OTAS_IDX_TX_DAT_CHAR]      = {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // OTA TX CMD Characteristic Value
    [OTAS_IDX_TX_DAT_VAL]       = {ATT_16_TO_128_ARRAY(ATT_CHAR_OTAS_TX_DAT),      PERM(NTF, ENABLE), 0, OTAS_CHAR_MAX_LEN},
    // OTA TX CMD Characteristic - Client Characteristic Configuration Descriptor
    [OTAS_IDX_TX_DAT_NTF_CFG]   = {ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG),  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
};

static const uint8_t otas_service_uuid128[16] = {0xFB,0x34,0x9B,0x5F,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x34,0x12,0x00,0x00};

struct otas_env_tag *otas_env = NULL;

static co_timer_t otas_timeout_timer;

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 * @brief otas_timeout_timer_handler()
 *
 * @param[in] timer  
 * @param[in] param  
 *
 * @return 
 **/
static void otas_timeout_timer_handler(co_timer_t *timer, void *param)
{
#ifdef CONFIG_BOOTLOADER
    otas_reboot();
#endif
}

/**
 ****************************************************************************************
 * @brief Initialization of the OTAS module.
 *
 * @param[out]    env        Collector or Service allocated environment data.
 * @param[in|out] start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task   Application task number.
 * @param[in]     sec_lvl    Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     param      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t otas_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl, struct otas_db_cfg* params)
{
    // DB Creation Status
    uint8_t status;

    status = attm_svc_create_db_128(start_hdl, otas_service_uuid128, NULL,
            OTAS_IDX_NB, NULL, env->task, &otas_att_db[0],
            PERM(SVC_UUID_LEN, UUID_128));

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        otas_env = (struct otas_env_tag* ) ke_malloc(sizeof(struct otas_env_tag), KE_MEM_ATT_DB);

        // allocate OTA required environment variable
        env->env = (prf_env_t*) otas_env;
        otas_env->shdl = *start_hdl;
        otas_env->prf_env.app_task = app_task;
        otas_env->prf_env.prf_task = env->task;
        otas_env->rx.packet_handler = params->packet_handler;

        // initialize environment variable
        env->id = TASK_ID_OTAS;
        otas_task_init(&(env->desc));

        // service is ready, go into an Idle state
        ke_state_set(env->task, OTAS_IDLE);

        // init tx fifo
        co_fifo_init(TX_CMD_FIFO, otas_env->tx.cmd_buff, OTAS_TX_CMD_BUFF_SIZE);
        co_fifo_init(TX_DAT_FIFO, otas_env->tx.dat_buff, OTAS_TX_DAT_BUFF_SIZE);
        co_fifo_init(RX_CMD_FIFO, otas_env->rx.cmd_buff, OTAS_RX_CMD_BUFF_SIZE);
        co_fifo_init(RX_DAT_FIFO, otas_env->rx.dat_buff, OTAS_RX_DAT_BUFF_SIZE);
    }

    // Enable timeout timer
    co_timer_set(&otas_timeout_timer, OTAS_TIMEOUT_TIME, TIMER_ONE_SHOT, otas_timeout_timer_handler, NULL);

    return (status);
}
/**
 ****************************************************************************************
 * @brief Destruction of the OTAS module - due to a reset for instance.
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void otas_destroy(struct prf_task_env* env)
{
    ASSERT_ERR(otas_env == (struct otas_env_tag*)env->env);

    ke_free(otas_env);

    env->env = NULL;
    otas_env = NULL;
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void otas_create(struct prf_task_env* env, uint8_t conidx)
{
    otas_log("Connected\n");

    co_fifo_reset(TX_CMD_FIFO);
    co_fifo_reset(TX_DAT_FIFO);
    co_fifo_reset(RX_CMD_FIFO);
    co_fifo_reset(RX_DAT_FIFO);

#if OTAS_SECURITY
    otas_env->tx.count = 0;
    otas_env->rx.count = 0;
    memset(otas_env->key.dh, 0, sizeof(ecc_key_t));
#endif

#if OTAS_DEBUG_AUTO_SEND
    otas_env->is_debug_auto_send_start = false;
#endif

    otas_env->mtu = ATT_DEFAULT_MTU;
    otas_env->mto = BLE_MIN_OCTETS;
    otas_env->mro = BLE_MIN_OCTETS;
    otas_env->perfect_once_tx_length = OTAS_MTU_TO_NTF_WRTCMD_LEN(ATT_DEFAULT_MTU);

    otas_env->is_rebooting = false;
    otas_env->is_notifying = false;

    otas_env->tx_cmd_ntf_cfg = 0; // PRF_CLI_START_NTF
    otas_env->tx_dat_ntf_cfg = 0;

    otas_env->conidx = conidx;

    // Disable timeout timer
    co_timer_del(&otas_timeout_timer);
}

/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void otas_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    otas_log("Disconnected 0x%02X\n", reason);

    // Enable timeout timer
    co_timer_set(&otas_timeout_timer, OTAS_TIMEOUT_TIME, TIMER_ONE_SHOT, otas_timeout_timer_handler, NULL);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// OTAS Task interface required by profile manager
const struct prf_task_cbs otas_itf =
{
    (prf_init_fnct) otas_init,
    otas_destroy,
    otas_create,
    otas_cleanup,
};


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* otas_prf_itf_get(void)
{
   return &otas_itf;
}

/// @} OTAS
