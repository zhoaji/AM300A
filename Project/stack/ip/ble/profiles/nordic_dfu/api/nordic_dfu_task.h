/**
 ****************************************************************************************
 *
 * @file nordic_dfu_task.h
 *
 * @brief Header file - Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _NORDIC_DFU_TASK_H_
#define _NORDIC_DFU_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup NORDIC_DFUSTASK Task
 * @ingroup NORDIC_DFU
 * @brief 'Profile' Task.
 *
 * The NORDIC_DFUSTASK is responsible for handling the messages coming in and out of the
 * @ref BAPS block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "prf_types.h"
#include "rwip_task.h" // Task definitions
#include "co_utils.h"
#include "co_debug.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Messages for NORDIC_DFU
/*@TRACE*/
enum nordic_dfu_msg_id
{
    /// Start the Server - at connection used to restore bond data
    NORDIC_DFU_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_NORDIC_DFU),//!< NORDIC_DFU_ENABLE_REQ
    /// Update start
    NORDIC_DFU_UPDATE_START_IND,//!< NORDIC_DFU_UPDATE_START_IND
    /// Update end
    NORDIC_DFU_UPDATE_END_IND,//!< NORDIC_DFU_UPDATE_END_IND @ref struct nordic_dfu_update_end_ind
    /// Update process
    NORDIC_DFU_UPDATE_PROG_IND,//!< NORDIC_DFU_UPDATE_PROG_IND @ref struct nordic_dfu_update_prog_ind
    /// Delay Timer
    NORDIC_DFU_END_TIMER,//!< NORDIC_DFU_END_TIMER
};


/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct nordic_dfu_db_cfg
{
    void *param;
};

struct nordic_dfu_update_end_ind
{
    uint32_t new_app_base_addr;
    uint32_t new_app_size;
    uint32_t new_patch_base_addr;
    uint32_t new_patch_size;
    uint32_t new_cfg_base_addr;
    uint32_t new_cfg_size;
    uint8_t status;
};

struct nordic_dfu_update_prog_ind
{
    uint32_t value_now;
    uint32_t value_max;
};

/// @} NORDIC_DFUSTASK

#endif /* _NORDIC_DFU_TASK_H_ */
