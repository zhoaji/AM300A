/**
 ****************************************************************************************
 *
 * @file simple_server_task.h
 *
 * @brief Header file - Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _SIMPLE_SERVER_TASK_H_
#define _SIMPLE_SERVER_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup SIMPLE_SERVERSTASK Task
 * @ingroup SIMPLE_SERVER
 * @brief 'Profile' Task.
 *
 * The SIMPLE_SERVERSTASK is responsible for handling the messages coming in and out of the
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

///Maximal number of SIMPLE_SERVER that can be added in the DB
#define SIMPLE_SERVER_NB_INSTANCES_MAX         (1)

#define SIMPLE_SERVER_MAX_CHAC_LEN    20


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Messages for SIMPLE_SERVER
/*@TRACE*/
enum simple_server_msg_id
{
    /// Start the Server - at connection used to restore bond data
    SIMPLE_SERVER_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_SIMPLE_SERVER),//!< SIMPLE_SERVER_ENABLE_REQ @ref struct simple_server_enable_req
    /// Confirmation of the Server start
    SIMPLE_SERVER_ENABLE_RSP,                               //!< SIMPLE_SERVER_ENABLE_RSP @ref struct simple_server_enable_req
    /// Inform APP that Notification Configuration has been changed
    SIMPLE_SERVER_NTF_CFG_IND,                         //!< SIMPLE_SERVER_NTF_CFG_IND @ref struct simple_server_demo_ntf_cfg_ind
    /// Send notification to gatt client
    SIMPLE_SERVER_SEND_NTF_CMD,                         //!< SIMPLE_SERVER_SEND_NTF_CMD @ref struct simple_server_send_ntf_cmd
    SIMPLE_SERVER_TIMEOUT_TIMER,
};


/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct simple_server_db_cfg
{
    void *param;
};

/// Parameters of the @ref SIMPLE_SERVER_ENABLE_REQ message
struct simple_server_enable_req
{
    /// connection index
    uint8_t  conidx;
    /// Notification Configuration
    uint8_t  ntf_cfg;
};

/// Parameters of the @ref SIMPLE_SERVER_ENABLE_RSP message
struct simple_server_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref SIMPLE_SERVER_DEMO_NTF_CFG_IND message
struct simple_server_demo_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint8_t  ntf_cfg;
};

///Parameters of the @ref SIMPLE_SERVER_SEND_NTF_CMD message
struct simple_server_send_ntf_cmd
{
    /// connection index
    uint8_t  conidx;
    uint16_t length;
    uint8_t value[__ARRAY_EMPTY];
};


void enable_notification(uint8_t enable);

/// @} SIMPLE_SERVERSTASK

#endif /* _SIMPLE_SERVER_TASK_H_ */
