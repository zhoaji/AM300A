/**
 ****************************************************************************************
 *
 * @file onmicro_dfu.h
 *
 * @brief Header file - Service Server Role
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _ONMICRO_DFU_H_
#define _ONMICRO_DFU_H_

/**
 ****************************************************************************************
 * @addtogroup ONMICRO_DFU 'Profile' Server
 * @ingroup ONMICRO_DFU
 * @brief 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "dfu_task.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */
 
/*********************************************************************
 * MACROS
 */

#define ONMICRO_DFU_PKG_MAX_LEN       520
#define BLE_UUID_ONMICRO_DFU_SERVICE  0xFE59
#define ONMICRO_DFU_CTRL_UUID     {0x50, 0xEA, 0xDA, 0x30, 0x88, 0x83, 0xB8, 0x9F, 0x60, 0x4F, 0x15, 0xF3, 0x01, 0x00, 0xC9, 0x8E}
#define ONMICRO_DFU_PKG_UUID      {0x50, 0xEA, 0xDA, 0x30, 0x88, 0x83, 0xB8, 0x9F, 0x60, 0x4F, 0x15, 0xF3, 0x02, 0x00, 0xC9, 0x8E}
#define ONMICRO_DFU_VERSION_UUID  {0x03, 0x00}

///Maximum number of Server task instances
#define ONMICRO_DFU_IDX_MAX     0x01

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// ONMICRO_DFU Service Attributes Indexes
enum
{
    ONMICRO_DFU_IDX_SVC,

    ONMICRO_DFU_IDX_CTRL_CHAR,
    ONMICRO_DFU_IDX_CTRL_VAL,
    ONMICRO_DFU_IDX_CTRL_DESC,

    ONMICRO_DFU_IDX_PKG_CHAR,
    ONMICRO_DFU_IDX_PKG_VAL,

    ONMICRO_DFU_IDX_VERSION_CHAR,
    ONMICRO_DFU_IDX_VERSION_VAL,
    ONMICRO_DFU_IDX_VERSION_DESC,

    ONMICRO_DFU_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// 'Profile' Server environment variable
struct onmicro_dfu_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// On-going operation
    struct ke_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
    /// ONMICRO_DFU task state
    ke_state_t state[ONMICRO_DFU_IDX_MAX];
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
const struct prf_task_cbs* onmicro_dfu_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve Attribute handle from service and attribute index
 *
 * @param[in] svc_idx ONMICRO_DFU Service index
 * @param[in] att_idx Attribute index
 *
 * @return ONMICRO_DFU attribute handle or INVALID HANDLE if nothing found
 ****************************************************************************************
 */
uint16_t onmicro_dfu_get_att_handle(uint8_t att_idx);

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
uint8_t onmicro_dfu_get_att_idx(uint16_t handle, uint8_t *att_idx);

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
void onmicro_dfu_task_init(struct ke_task_desc *task_desc);

/// @} ONMICRO_DFU

#endif /* _ONMICRO_DFU_H_ */
