/**
 ****************************************************************************************
 *
 * @file onmicro_dfu_task.h
 *
 * @brief Header file - Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _ONMICRO_DFU_TASK_H_
#define _ONMICRO_DFU_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup ONMICRO_DFUSTASK Task
 * @ingroup ONMICRO_DFU
 * @brief 'Profile' Task.
 *
 * The ONMICRO_DFUSTASK is responsible for handling the messages coming in and out of the
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


/// Messages for ONMICRO_DFU
/*@TRACE*/
enum onmicro_dfu_msg_id
{
    /// Start the Server - at connection used to restore bond data
    ONMICRO_DFU_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_ONMICRO_DFU),//!< ONMICRO_DFU_ENABLE_REQ
    /// Delay Timer
    ONMICRO_DFU_END_TIMER,//!< ONMICRO_DFU_END_TIMER
};

/// @} ONMICRO_DFUSTASK

#endif /* _ONMICRO_DFU_TASK_H_ */
