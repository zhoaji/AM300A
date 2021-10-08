/**
 ****************************************************************************************
 *
 * @file tsppc_task.h
 *
 * @brief Header file - Transport Profile Collector Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _TSPPC_TASK_H_
#define _TSPPC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup TSPPCTASK Transport Profile Collector Task
 * @ingroup TSPPC
 * @brief Heart Rate Profile Collector Task
 *
 * The TSPPCTASK is responsible for handling the messages coming in and out of the
 * @ref TSPPC monitor block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "tspp_common.h"
#include "rwip_task.h" // Task definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Characteristics
enum
{
    /// Notify char
    TSPPC_CHAR_NOTIFY,
    /// write char
    TSPPC_CHAR_WRITE,
    /// write without response
    TSPPC_CHAR_WRITE_NO_RESPONSE,

    TSPPC_CHAR_MAX,
};


/// Characteristic descriptors
enum
{
    /// notify client config
    TSPPC_DESC_CLI_CFG,
    TSPPC_DESC_MAX,
    TSPPC_DESC_MASK = 0x10,
};

/*@TRACE*/
enum tsppc_msg_id
{
    /// Start the profile - at connection
    TSPPC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_TSPPC),
    ///Confirm that cfg connection has finished with discovery results, or that normal cnx started
    TSPPC_ENABLE_RSP,

    /// Generic message to read characteristic value
    TSPPC_RD_CHAR_REQ,
    ///Generic message for read responses for APP
    TSPPC_RD_CHAR_RSP,

    ///Generic message for configuring the 2 characteristics that can be handled
    TSPPC_CFG_INDNTF_REQ,
    ///Update IND/NTF config response
    TSPPC_CFG_INDNTF_RSP,

    ///APP request for control point write (to reset Energy Expanded)
    TSPPC_WR_CHAR_REQ,
    ///Write Control Point response
    TSPPC_WR_CHAR_RSP,

    /// notify value send to APP
    TSPPC_NOTIFY_IND,
};

///Structure containing the characteristics handles, value handles and descriptors
struct tspps_content
{
    /// service info
    struct prf_svc svc;

    /// characteristic info:
    ///  - notify
    ///  - write 
    ///  - write without response
    struct prf_char_inf chars[TSPPC_CHAR_MAX];

    /// Descriptor handles:
    ///  - notify client cfg
    struct prf_char_desc_inf descs[TSPPC_DESC_MAX];
};

/// Parameters of the @ref TSPPC_ENABLE_REQ message
struct tsppc_enable_req
{
    ///Connection type
    uint8_t con_type;
    ///Existing handle values tspps
    struct tspps_content tspps;
};

/// Parameters of the @ref TSPPC_ENABLE_RSP message
struct tsppc_enable_rsp
{
    ///status
    uint8_t status;
    ///Existing handle values tspps
    struct tspps_content tspps;
};

///Parameters of the @ref TSPPC_CFG_INDNTF_REQ message
struct tsppc_cfg_indntf_req
{
    ///Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};


///Parameters of the @ref TSPPC_CFG_INDNTF_RSP message
struct tsppc_cfg_indntf_rsp
{
    ///Status
    uint8_t  status;
};

///Parameters of the @ref TSPPC_RD_CHAR_REQ message
struct tsppc_rd_char_req
{
    ///Characteristic value code
    uint8_t  char_code;
};

///Parameters of the @ref TSPPC_RD_CHAR_RSP message
struct tsppc_rd_char_rsp
{
    /// Attribute data information
    struct prf_att_info info;
};

///Parameters of the @ref TSPPC_WR_CHAR_REQ message
struct tsppc_wr_char_req
{
    ///value
    uint16_t length;
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref TSPPC_WR_CHAR_RSP message
struct tsppc_wr_char_rsp
{
    ///Status
    uint8_t  status;
};

///Parameters of the @ref TSPPC_NOTIFY_IND message
struct tsppc_notify_ind
{
    ///notify
    uint16_t length;
    uint8_t value[__ARRAY_EMPTY];
};

/// @} TSPPCTASK

#endif /* _TSPPC_TASK_H_ */
