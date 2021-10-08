/**
 ****************************************************************************************
 *
 * @file tspps.h
 *
 * @brief Header file - Transport Profile.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _TSPPS_H_
#define _TSPPS_H_

/**
 ****************************************************************************************
 * @addtogroup TSPPS Transport Profile
 * @ingroup TSPP
 * @brief Transport Profile
 *
 * Transport (TSP) profile provides functionalities to upper layer module
 * application. The device using this profile takes the role of transport.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from APP)
 *  - Disable the profile role (from APP)
 *  - Notify data (from APP)
 *  - write data (form client)
 *
 * Transport Profile. (TSPPS): A TSPPS 
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_TSPP_SERVER)
#include "tspp_common.h"
#include "prf_types.h"
#include "prf.h"
#include "tspps_task.h"
#include "attm.h"

/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of transport task instances
#define TSPPS_IDX_MAX     BLE_CONNECTION_MAX//0x01

#define TSPPS_UPLOAD_MAX_LEN            (BLE_MAX_OCTETS-4-3) // l2cap header 4B, notify header 3B
#define TSPPS_MTU_TO_NTF_WRTCMD_LEN(n)   ((n) - 3)
#define TSPPS_MTO_TO_NTF_WRTCMD_LEN(n)   ((n) - 7)
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/// Possible states of the TSPPS task
enum
{
    /// Idle state
    TSPPS_IDLE,
    /// Connected state
    TSPPS_CONNECTED,
    // send data
    TSPPS_BUSY,

    /// Number of defined states.
    TSPPS_STATE_MAX,
};

///Attributes State Machine
enum
{
    TSPPS_IDX_SVC,

    TSPPS_IDX_REV1_CHAR,
    TSPPS_IDX_REV1_VAL,

    TSPPS_IDX_UPLOAD_CHAR,
    TSPPS_IDX_UPLOAD_VAL,
    TSPPS_IDX_UPLOAD_NTF_CFG,

    TSPPS_IDX_REV2_CHAR,
    TSPPS_IDX_REV2_VAL,

    TSPPS_IDX_NB,
};

enum
{
    TSPPS_REV1_CHAR,
    TSPPS_UPLOAD_CHAR,
    TSPPS_REV2_CHAR,
    TSPPS_CHAR_MAX,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Transport Profile environment variable
struct tspps_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// On-going operation
    struct ke_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
    /// TSPP_SERVER task state
    ke_state_t state[TSPPS_IDX_MAX];
    /// Notification configuration of peer devices.
    uint8_t ntf_cfg[TSPPS_IDX_MAX];
    // max mtu;
    uint16_t mtu[TSPPS_IDX_MAX];
    uint16_t tx_len[TSPPS_IDX_MAX];
    uint16_t perfect_once_tx_length[TSPPS_IDX_MAX];

    // for send buffer.
    uint32_t   send_pool_size;
    uint32_t   send_pool_num;
    co_fifo_t  send_fifo[TSPPS_IDX_MAX];
    uint8_t   send_pool_inner[TSPPS_IDX_MAX];
    uint8_t   *send_pool[TSPPS_IDX_MAX];
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
 * @brief Retrieve TSPP service profile interface
 *
 * @return TSPP service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* tspp_server_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve Attribute handle from service and attribute index
 *
 * @param[in] svc_idx TSPP_SERVER Service index
 * @param[in] att_idx Attribute index
 *
 * @return TSPP_SERVER attribute handle or INVALID HANDLE if nothing found
 ****************************************************************************************
 */
uint16_t tspp_server_get_att_handle(uint8_t att_idx);

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
uint8_t tspp_server_get_att_idx(uint16_t handle, uint8_t *att_idx);

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
void tspp_server_task_init(struct ke_task_desc *task_desc);

#endif /* #if (BLE_TSPP_SERVER) */

/// @} TSPPS

#endif /* _HRPS_H_ */
