/**
 ****************************************************************************************
 *
 * @file otas.h
 *
 * @brief Header file - Battery Service Server Role
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _OTAS_H_
#define _OTAS_H_

/**
 ****************************************************************************************
 * @addtogroup BAPS Battery 'Profile' Server
 * @ingroup BAP
 * @brief Battery 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#include "otas_task.h"
#include "prf_types.h"
#include "prf.h"
#include "co.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define OTAS_DEBUG                  1
#define OTAS_DEBUG_AUTO_SEND        0
#define OTAS_SECURITY               1

#define OTAS_MTU_TO_NTF_WRTCMD_LEN(n)   ((n) - 3)
#define OTAS_MTO_TO_NTF_WRTCMD_LEN(n)   ((n) - 7)

#define BLE_UUID_OTA_RX_CMD_CHAR    0xFF01
#define BLE_UUID_OTA_RX_DAT_CHAR    0xFF02
#define BLE_UUID_OTA_TX_CMD_CHAR    0xFF03
#define BLE_UUID_OTA_TX_DAT_CHAR    0xFF04

#define OTAS_CHAR_MAX_LEN           (OTAS_MTO_TO_NTF_WRTCMD_LEN(BLE_MAX_OCTETS) + BLE_MAX_OCTETS)

#define OTAS_TX_DAT_BUFF_SIZE       (1024*4)
#define OTAS_RX_DAT_BUFF_SIZE       (1024*4)
#define OTAS_TX_CMD_BUFF_SIZE       (1024)
#define OTAS_RX_CMD_BUFF_SIZE       (1024)

#define TX_CMD_FIFO                 (&otas_env->tx.cmd_fifo)
#define TX_DAT_FIFO                 (&otas_env->tx.dat_fifo)
#define RX_CMD_FIFO                 (&otas_env->rx.cmd_fifo)
#define RX_DAT_FIFO                 (&otas_env->rx.dat_fifo)

#define OTAS_CMD_HEAD_LEN           (sizeof(otas_cmd_t)-4)

#if OTAS_DEBUG
#define otas_log(format, ...)      log_debug("[OTAS] "format, ## __VA_ARGS__)
#define otas_log_array_ex(n, a, l) log_debug_array_ex("[OTAS] "n, a, l)
#else
#define otas_log(format, ...)
#define otas_log_array_ex(n, a, l)
#endif

#define OTAS_HANDLE(idx)            (otas_env->shdl + (idx))
#define OTAS_IDX(hdl)               ((hdl) - otas_env->shdl)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the OTAS task
enum otas_state
{
    /// Idle state
    OTAS_IDLE,
    /// busy state
    OTAS_BUSY,
    /// Number of defined states.
    OTAS_STATE_MAX
};

/// OTAS Service Attributes Indexes
enum
{
    OTAS_IDX_SVC,
    OTAS_IDX_RX_CMD_CHAR,
    OTAS_IDX_RX_CMD_VAL,
    OTAS_IDX_RX_DAT_CHAR,
    OTAS_IDX_RX_DAT_VAL,
    OTAS_IDX_TX_CMD_CHAR,
    OTAS_IDX_TX_CMD_VAL,
    OTAS_IDX_TX_CMD_NTF_CFG,
    OTAS_IDX_TX_DAT_CHAR,
    OTAS_IDX_TX_DAT_VAL,
    OTAS_IDX_TX_DAT_NTF_CFG,

    OTAS_IDX_NB,
};

/// OTAS commands
enum
{
    OTAS_CMD_REBOOT_TO_APP     = 0,
    OTAS_CMD_EXCHANGE_KEY      = 1,
    OTAS_CMD_CONN_PARAM_UPDATE = 2,
    OTAS_CMD_MTU_EXCHANGE      = 3,
    OTAS_CMD_LENGTH_EXCHANGE   = 4,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef struct
{
    uint16_t min_conn_interval;
    uint16_t max_conn_interval;
    uint16_t slave_latency;
    uint16_t conn_sup_timeout;
}otas_conn_params_t;

typedef struct
{
    uint16_t conn_interval;
    uint16_t slave_latency;
    uint16_t conn_sup_timeout;
}otas_updated_conn_params_t;

typedef struct
{
    uint16_t mtu;
    uint16_t perfect_once_tx_length;
    uint16_t perfect_once_rx_length;
}otas_updated_mtu_t;

typedef struct
{
    uint16_t mto;
    uint16_t mro;
    uint16_t perfect_once_tx_length;
    uint16_t perfect_once_rx_length;
}otas_updated_length_t;

typedef void (* otas_packet_handler_t)(const uint8_t *pdata, uint16_t len);

typedef uint8_t ecc_key_t[32];

typedef struct
{
    ecc_key_t x;
    ecc_key_t y;
}ecc_xykey_t;

typedef struct
{
    uint16_t cmd;
    uint16_t len;
    uint8_t  data[4];
}otas_cmd_t;

typedef void (* otas_cmd_handler_t)(const uint8_t *pdata, uint16_t len);

/// OTA Profile Server environment variable
struct otas_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    ///IAS Start Handle
    uint16_t shdl;
    /// State of different task instances
    ke_state_t state[1];

#if OTAS_SECURITY
    struct
    {
        ecc_key_t dh;
        ecc_xykey_t local;
        ecc_xykey_t remote;
    }key;
#endif

    struct
    {
        otas_packet_handler_t packet_handler;

        co_fifo_t cmd_fifo;
        uint8_t   cmd_buff[OTAS_RX_CMD_BUFF_SIZE];

#if OTAS_SECURITY
        uint32_t  count;
#endif
        co_fifo_t dat_fifo;
        uint8_t   dat_buff[OTAS_RX_DAT_BUFF_SIZE];
    }rx;

    struct
    {
        co_fifo_t cmd_fifo;
        uint8_t   cmd_buff[OTAS_TX_CMD_BUFF_SIZE];

#if OTAS_SECURITY
        uint32_t  count;
#endif
        co_fifo_t dat_fifo;
        uint8_t   dat_buff[OTAS_TX_DAT_BUFF_SIZE];
    }tx;

#if OTAS_DEBUG_AUTO_SEND
    bool is_debug_auto_send_start;
#endif

    uint8_t conidx;

    uint16_t mtu;
    uint16_t mto;
    uint16_t mro;

    uint16_t perfect_once_tx_length;

    bool is_rebooting;
    bool is_notifying;

    uint16_t tx_cmd_ntf_cfg;
    uint16_t tx_dat_ntf_cfg;
};

struct otas_db_cfg
{
    otas_packet_handler_t packet_handler;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct otas_env_tag *otas_env;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve BAS service profile interface
 *
 * @return BAS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* otas_prf_itf_get(void);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param task_desc Task descriptor to fill
 ****************************************************************************************
 */
void otas_task_init(struct ke_task_desc *task_desc);


/// @} OTAS

#endif /* _OTAS_H_ */
