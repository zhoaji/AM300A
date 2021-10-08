/**
 ****************************************************************************************
 *
 * @file atvvs.h
 *
 * @brief Header file - ATV Voice Profile.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _ATVVS_H_
#define _ATVVS_H_

/**
 ****************************************************************************************
 * @addtogroup ATVVS Transport Profile
 * @ingroup ATVVS
 * @brief ATV Voice Profile
 *
 * ATVV profile provides functionalities to upper layer module
 * application. The device using this profile takes the role of transport.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from APP)
 *  - Disable the profile role (from APP)
 *  - Notify data (from APP)
 *  - write data (form client)
 *
 * ATV Voice Profile. (ATVV): A ATVVS 
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_ATVV_SERVER)
#include "atvv_common.h"
#include "prf_types.h"
#include "prf.h"
#include "atvvs_task.h"
#include "attm.h"

/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of transport task instances
#define ATVVS_IDX_MAX     0x01

#define ATVVS_UPLOAD_MAX_LEN            (BLE_MAX_OCTETS-4-3) // l2cap header 4B, notify header 3B
#define ATVVS_MTU_TO_NTF_WRTCMD_LEN(n)   ((n) - 3)
#define ATVVS_MTO_TO_NTF_WRTCMD_LEN(n)   ((n) - 7)
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/// Possible states of the ATVVS task
enum
{
    /// Idle state
    ATVVS_IDLE,
    /// Connected state
    ATVVS_CONNECTED,
    // send data
    ATVVS_BUSY,

    /// Number of defined states.
    ATVVS_STATE_MAX,
};

///Attributes State Machine
enum
{
    ATVVS_IDX_SVC,

    ATVVS_IDX_WRITE_CHAR,
    ATVVS_IDX_WRITE_VAL,

    ATVVS_IDX_READ_CHAR,
    ATVVS_IDX_READ_VAL,
    ATVVS_IDX_READ_NTF_CFG,

    ATVVS_IDX_CONTROL_CHAR,
    ATVVS_IDX_CONTROL_VAL,
    ATVVS_IDX_CONTROL_NTF_CFG,

    ATVVS_IDX_NB,
};

enum
{
    ATVVS_WRITE_CHAR,
    ATVVS_READ_CHAR,
    ATVVS_CONTROL_CHAR,
    ATVVS_CHAR_MAX,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// ATV Voice profile environment variable
struct atvvs_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// On-going operation
    struct ke_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
    /// ATVV_SERVER task state
    ke_state_t state[ATVVS_IDX_MAX];
    /// Notification configuration of peer devices.
    uint8_t ntf_cfg[ATVVS_IDX_MAX][2];
    // max mtu;
    uint16_t mtu[ATVVS_IDX_MAX];
    uint16_t tx_len[ATVVS_IDX_MAX];
    uint16_t perfect_once_tx_length[ATVVS_IDX_MAX];

    // for send buffer.
    uint32_t   send_pool_size;
    uint32_t   send_pool_num;
    co_fifo_t  send_fifo[ATVVS_IDX_MAX];
    uint8_t    send_pool_inner[ATVVS_IDX_MAX];
    uint8_t   *send_pool[ATVVS_IDX_MAX];
    uint16_t   send_seq;
    uint16_t   sent_num;
};


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve ATVV service profile interface
 *
 * @return ATVV service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* atvv_server_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve Attribute handle from service and attribute index
 *
 * @param[in] svc_idx ATVV_SERVER Service index
 * @param[in] att_idx Attribute index
 *
 * @return ATVV_SERVER attribute handle or INVALID HANDLE if nothing found
 ****************************************************************************************
 */
uint16_t atvv_server_get_att_handle(uint8_t att_idx);

/**
 ****************************************************************************************
 * @brief Retrieve Service and attribute index form attribute handle
 *
 * @param[out] handle  Attribute handle
 * @param[out] svc_idx Service index
 * @param[out] att_idx Attribute index
 *
 * @return Success if attribute and service index found, else Application error
 ****************************************************************************************
 */
uint8_t atvv_server_get_att_idx(uint16_t handle, uint8_t *att_idx);

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
void atvv_server_task_init(struct ke_task_desc *task_desc);

#endif /* #if (BLE_ATVV_SERVER) */

/// @} ATVVS

#endif /* _HRPS_H_ */
