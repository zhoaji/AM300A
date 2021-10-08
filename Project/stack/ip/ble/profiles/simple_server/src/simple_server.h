/**
 ****************************************************************************************
 *
 * @file simple_server.h
 *
 * @brief Header file - Service Server Role
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _SIMPLE_SERVER_H_
#define _SIMPLE_SERVER_H_

/**
 ****************************************************************************************
 * @addtogroup SIMPLE_SERVER 'Profile' Server
 * @ingroup SIMPLE_SERVER
 * @brief 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "simple_server_task.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define ATT_SVC_SIMPLE_SERVER_SERVICE {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xB0, 0xFF, 0x00, 0x00}
#define ATT_SVC_SIMPLE_SERVER_CHAC1   {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xB2, 0xFF, 0x00, 0x00}
#define ATT_SVC_SIMPLE_SERVER_CHAC2   {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xB1, 0xFF, 0x00, 0x00}


///Maximum number of Server task instances
#define SIMPLE_SERVER_IDX_MAX     0x01


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the SIMPLE_SERVER task
enum simple_server_state
{
    /// Idle state
    SIMPLE_SERVER_IDLE,
    /// busy state
    SIMPLE_SERVER_BUSY,
    /// Number of defined states.
    SIMPLE_SERVER_STATE_MAX
};

/// SIMPLE_SERVER Service Attributes Indexes
enum
{
    SIMPLE_SERVER_IDX_SVC,

    SIMPLE_SERVER_IDX_DEMO_CHAR1,
    SIMPLE_SERVER_IDX_DEMO_VAL1,
    SIMPLE_SERVER_IDX_DEMO_NTF_CFG,

    SIMPLE_SERVER_IDX_DEMO_CHAR2,
    SIMPLE_SERVER_IDX_DEMO_VAL2,

    SIMPLE_SERVER_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// 'Profile' Server environment variable
struct simple_server_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// On-going operation
    struct ke_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
    /// SIMPLE_SERVER task state
    ke_state_t state[SIMPLE_SERVER_IDX_MAX];
    /// Notification configuration of peer devices.
    uint8_t ntf_cfg[BLE_CONNECTION_MAX];
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
 * @brief Retrieve service profile interface
 *
 * @return service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* simple_server_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve Attribute handle from service and attribute index
 *
 * @param[in] svc_idx SIMPLE_SERVER Service index
 * @param[in] att_idx Attribute index
 *
 * @return SIMPLE_SERVER attribute handle or INVALID HANDLE if nothing found
 ****************************************************************************************
 */
uint16_t simple_server_get_att_handle(uint8_t att_idx);

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
uint8_t simple_server_get_att_idx(uint16_t handle, uint8_t *att_idx);

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
void simple_server_task_init(struct ke_task_desc *task_desc);

/// @} SIMPLE_SERVER

#endif /* _SIMPLE_SERVER_H_ */
