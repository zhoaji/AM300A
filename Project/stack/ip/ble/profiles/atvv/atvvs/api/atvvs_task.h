/**
 ****************************************************************************************
 *
 * @file atvvs_task.h
 *
 * @brief Header file - Transport Profile Sensor Task.
 *
 * Copyright (C) Huntersun 2018-2028
 *
 *
 ****************************************************************************************
 */


#ifndef _ATVVS_TASK_H_
#define _ATVVS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup ATVVSTASK Task
 * @ingroup ATVVS
 * @brief Transport Profile Task.
 *
 * The ATVVSTASK is responsible for handling the messages coming in and out of the
 * @ref ATVVS collector block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "rwip_task.h" // Task definitions

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Messages for ATVV SERVER
/*@TRACE*/
/// atvv server error id
enum atvv_server_error_id
{
    ATVV_SERVER_ERROR_OK       = 0,
    ATVV_SERVER_ERROR_NO_BUFF  = 1,
    ATVV_SERVER_ERROR_NO_CONN  = 2,
    ATVV_SERVER_ERROR_NO_NOTI  = 3,
};

/// atvv server message id
enum atvv_server_msg_id
{
    /// Start the Server - at connection used to restore bond data
    ATVV_SERVER_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_ATVVS),//!< @ref struct atvv_server_enable_req
    /// Confirmation of the Server start
    ATVV_SERVER_ENABLE_RSP,                               //!<  @ref struct atvv_server_enable_req
    /// Inform APP that Notification Configuration has been changed
    ATVV_SERVER_NTF_CFG_IND,                         //!< @ref struct atvv_server_ntf_cfg_ind
    ATVV_SERVER_WRITE_IND,
    /// Send notification to gatt client
    ATVV_SERVER_SEND_NTF_CMD,                         //!< @ref struct atvv_server_send_ntf_cmd
    ATVV_SERVER_SEND_CTL_CMD,
    ATVV_SERVER_TIMEOUT_TIMER,
    ATVV_SERVER_BUFFER_EMPTY,
    ATVV_SERVER_BUFFER_FULL,
    ATVV_SERVER_ERROR,
};


/*
 * APIs Structures
 ****************************************************************************************
 */
#define ATVV_ATT_UUID_128_LEN       16
#define ATVV_ATT_UUID_16_LEN         2
struct atvvs_sdp_att
{
    /// Attribute UUID Length
    uint8_t  uuid_len;
    /// Attribute UUID
    uint8_t  uuid[ATVV_ATT_UUID_128_LEN];
};
/// atvv sdp service database
struct atvvs_sdp_svc
{
    /// Service UUID Length
    uint8_t  uuid_len;
    /// Service UUID
    uint8_t  uuid[ATVV_ATT_UUID_128_LEN];
    /// attribute information present in the service
    struct atvvs_sdp_att write_no_res;
    struct atvvs_sdp_att notify;
    struct atvvs_sdp_att write;
};
/// Parameters for the database creation
struct atvv_server_db_cfg
{
    uint8_t *fifo_buffer;
    uint16_t connect_num; // max connectd <= BLE_CONNECTION_MAX
    uint16_t fifo_size;
    uint8_t svc_type; // type=0; use database(16 uuid); type=1:use database(128 uuid); type=2; user custom
    struct atvvs_sdp_svc svc;
};

/// Parameters of the @ref ATVV_SERVER_ENABLE_REQ message
struct atvv_server_enable_req
{
    /// connection index
    uint8_t  conidx;
    /// 1: enable; 0: disable
    uint8_t  en; 
    /// Notification Configuration
    uint8_t  ntf_cfg;
};

/// Parameters of the @ref ATVV_SERVER_ENABLE_RSP message
struct atvv_server_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref ATVV_SERVER_NTF_CFG_IND message
struct atvv_server_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint8_t  ntf_cfg;
    /// type
    uint8_t  type;
};

///Parameters of the @ref ATVV_SERVER_SEND_NTF_CMD message
struct atvv_server_send_ntf_cmd
{
    /// connection index
    uint8_t  conidx;
    uint8_t length;
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref ATVV_SERVER_SEND_NTF_CMD message
struct atvv_server_send_ctl_cmd
{
    /// connection index
    uint8_t  conidx;
    uint16_t length;
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref ATVV_SERVER_WRITE_IND message
struct atvv_server_write_ind
{
    /// connection index
    uint8_t  conidx;
    /// Handle of the attribute that has to be written
    uint16_t handle;
    uint16_t offset;
    uint16_t length;
    uint8_t value[__ARRAY_EMPTY];
};

/// buffer empty event data structure
struct atvv_server_buffer_empty_evt
{
    /// GATT request type
    uint8_t operation;
    /// Status of the request
    uint8_t status;
};

/// buffer full event data structure
struct atvv_server_buffer_full_evt
{
    uint8_t operation;
    uint8_t status;
};

/// error event data structure
struct atvv_server_error_evt
{
    /// GATT request type
    uint16_t operation;
    /// Status of the request
    uint8_t status;
};

/// @} ATVVSTASK

uint8_t atvv_server_check_fifo_full(uint8_t conidx);
uint32_t atvv_server_get_free_size(uint8_t conidx);
uint16_t atvv_server_send_data(uint8_t conidx, uint8_t* pdata, uint16_t len);
void    atvv_server_set_max_mtu(uint8_t conidx, uint16_t mtu);
void    atvv_server_set_data_len(uint8_t conidx, uint16_t tx_len);
uint16_t atvv_server_perfect_once_tx_length(uint16_t mtu, uint16_t mto, uint16_t char_len);
#endif /* _ATVVS_TASK_H_ */
