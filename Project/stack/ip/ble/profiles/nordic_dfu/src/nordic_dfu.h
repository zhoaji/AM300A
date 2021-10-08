/**
 ****************************************************************************************
 *
 * @file nordic_dfu.h
 *
 * @brief Header file - Service Server Role
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _NORDIC_DFU_H_
#define _NORDIC_DFU_H_

/**
 ****************************************************************************************
 * @addtogroup NORDIC_DFU 'Profile' Server
 * @ingroup NORDIC_DFU
 * @brief 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "nordic_dfu_task.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */
 
/*********************************************************************
 * MACROS
 */

#define NORDIC_DFU_DEBUG
#if defined(NORDIC_DFU_DEBUG)
#define dfu_debug log_debug
#define dfu_debug_array_ex log_debug_array_ex
#else
#define dfu_debug(...)
#define dfu_debug_array_ex(...)
#endif

#define NORDIC_DFU_PKG_MAX_LEN       520

#define NEW_APP_BASE_ADDR1           0x03000
#define NEW_APP_BASE_ADDR2           0x23000
#define MAX_APP_SIZE                 0x20000
#define NEW_PATCH_BASE_ADDR1         0x7A000
#define NEW_PATCH_BASE_ADDR2         0x75000
#define MAX_PATCH_SIZE               0x02000
#define NEW_CFG_BASE_ADDR1           0x7C000
#define NEW_CFG_BASE_ADDR2           0x77000
#define MAX_CFG_SIZE                 0x03000

//APP version
#if !defined(DFU_VERSION) && !defined(DFU_VER_SIZE)
#define DFU_VERSION  "version-v1.0"
#define DFU_VER_SIZE (sizeof(DFU_VERSION) - 1)
#endif

#define BLE_UUID_NORDIC_DFU_SERVICE  0xFE59
#define NORDIC_DFU_CTRL_UUID     {0x50, 0xEA, 0xDA, 0x30, 0x88, 0x83, 0xB8, 0x9F, 0x60, 0x4F, 0x15, 0xF3, 0x01, 0x00, 0xC9, 0x8E}
#define NORDIC_DFU_PKG_UUID      {0x50, 0xEA, 0xDA, 0x30, 0x88, 0x83, 0xB8, 0x9F, 0x60, 0x4F, 0x15, 0xF3, 0x02, 0x00, 0xC9, 0x8E}
#define NORDIC_DFU_VERSION_UUID  {0x03, 0x00}


extern const uint8_t dfu_version[DFU_VER_SIZE];


#define DFU_DATA_OBJ_MAX_SIZE       (MAX_APP_SIZE + MAX_PATCH_SIZE)
#define DFU_COMMAND_OBJ_MAX_SIZE    256
#define DFU_BUF_SIZE                256

#define DFU_CTRL_CREATE             0x01
#define DFU_CTRL_SET_PRN            0x02
#define DFU_CTRL_CAL_CHECKSUM       0x03
#define DFU_CTRL_EXECTUE            0x04
#define DFU_CTRL_RESERVE            0x05
#define DFU_CTRL_SELECT             0x06
#define DFU_CTRL_RESPONSE           0x60

#define DFU_CTRL_CREATE_REQ_LEN       6
#define DFU_CTRL_SET_PRN_REQ_LEN      3
#define DFU_CTRL_CAL_CHECKSUM_REQ_LEN 1
#define DFU_CTRL_EXECTUE_REQ_LEN      1
#define DFU_CTRL_SELECT_REQ_LEN       2
#define DFU_CTRL_RESPONSE_LEN         3

#define DFU_INVALID_CODE             0x00
#define DFU_SUCCESS                  0x01
#define DFU_OPCODE_NOT_SUPPORT       0x02
#define DFU_INVALID_PARAMETER        0x03
#define DFU_INSUFFICIENT_RESOURCES   0x04
#define DFU_INVALID_OBJECT           0x05
#define DFU_UNSUPPORTED_TYPE         0x07
#define DFU_OPERATION_NOT_PERMITTED  0x08
#define DFU_OPERATION_FAILED         0x0A

enum DFU_PKG_TYPE{
    DFU_PKG_TYPE_INVALIED    = 0,
    DFU_PKG_TYPE_COMMAND_OBJ = 1,
    DFU_PKG_TYPE_DATA_OBJ    = 2,
};




///Maximum number of Server task instances
#define NORDIC_DFU_IDX_MAX     BLE_CONNECTION_MAX

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// NORDIC_DFU Service Attributes Indexes
enum
{
    NORDIC_DFU_IDX_SVC,

    NORDIC_DFU_IDX_CTRL_CHAR,
    NORDIC_DFU_IDX_CTRL_VAL,
    NORDIC_DFU_IDX_CTRL_DESC,

    NORDIC_DFU_IDX_PKG_CHAR,
    NORDIC_DFU_IDX_PKG_VAL,

    NORDIC_DFU_IDX_VERSION_CHAR,
    NORDIC_DFU_IDX_VERSION_VAL,

    NORDIC_DFU_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// 'Profile' Server environment variable
struct nordic_dfu_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// On-going operation
    struct ke_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
    /// NORDIC_DFU task state
    ke_state_t state[NORDIC_DFU_IDX_MAX];
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
const struct prf_task_cbs* nordic_dfu_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve Attribute handle from service and attribute index
 *
 * @param[in] svc_idx NORDIC_DFU Service index
 * @param[in] att_idx Attribute index
 *
 * @return NORDIC_DFU attribute handle or INVALID HANDLE if nothing found
 ****************************************************************************************
 */
uint16_t nordic_dfu_get_att_handle(uint8_t att_idx);

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
uint8_t nordic_dfu_get_att_idx(uint16_t handle, uint8_t *att_idx);

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
void nordic_dfu_task_init(struct ke_task_desc *task_desc);

/// @} NORDIC_DFU

#endif /* _NORDIC_DFU_H_ */
