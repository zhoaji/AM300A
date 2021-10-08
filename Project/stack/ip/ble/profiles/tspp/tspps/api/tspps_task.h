/**
 ****************************************************************************************
 *
 * @file tspps_task.h
 *
 * @brief Header file - Transport Profile Sensor Task.
 *
 * Copyright (C) Huntersun 2018-2028
 *
 *
 ****************************************************************************************
 */


#ifndef _TSPPS_TASK_H_
#define _TSPPS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup TSPPSTASK Task
 * @ingroup TSPPS
 * @brief Transport Profile Task.
 *
 * The TSPPSTASK is responsible for handling the messages coming in and out of the
 * @ref TSPPS collector block of the BLE Host.
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

/// Messages for TSPP SERVER
/*@TRACE*/
/// tspp server error id
enum tspp_server_error_id
{
    TSPP_SERVER_ERROR_OK       = 0,
    TSPP_SERVER_ERROR_NO_BUFF  = 1,
    TSPP_SERVER_ERROR_NO_CONN  = 2,
    TSPP_SERVER_ERROR_NO_NOTI  = 3,
};

/// tspp server message id
enum tspp_server_msg_id
{
    /// Start the Server - at connection used to restore bond data
    TSPP_SERVER_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_TSPPS),//!< @ref struct tspp_server_enable_req
    /// Confirmation of the Server start
    TSPP_SERVER_ENABLE_RSP,                               //!<  @ref struct tspp_server_enable_req
    /// Inform APP that Notification Configuration has been changed
    TSPP_SERVER_NTF_CFG_IND,                         //!< @ref struct tspp_server_ntf_cfg_ind
    TSPP_SERVER_WRITE_IND,
    /// Send notification to gatt client
    TSPP_SERVER_SEND_NTF_CMD,                         //!< @ref struct tspp_server_send_ntf_cmd
    TSPP_SERVER_TIMEOUT_TIMER,
    TSPP_SERVER_BUFFER_EMPTY,
    TSPP_SERVER_BUFFER_FULL,
    TSPP_SERVER_ERROR,
};


/*
 * APIs Structures
 ****************************************************************************************
 */
#define TSPP_ATT_UUID_128_LEN       16
#define TSPP_ATT_UUID_16_LEN         2
struct tspps_sdp_att
{
    /// Attribute UUID Length
    uint8_t  uuid_len;
    /// Attribute UUID
    uint8_t  uuid[TSPP_ATT_UUID_128_LEN];
};
/// tspp sdp service database
struct tspps_sdp_svc
{
    /// Service UUID Length
    uint8_t  uuid_len;
    /// Service UUID
    uint8_t  uuid[TSPP_ATT_UUID_128_LEN];
    /// attribute information present in the service
    struct tspps_sdp_att write_no_res;
    struct tspps_sdp_att notify;
    struct tspps_sdp_att write;
};
/// Parameters for the database creation
struct tspp_server_db_cfg
{
    uint16_t connect_num; // max connectd <= BLE_CONNECTION_MAX
    uint16_t fifo_size;
    uint8_t *fifo_buffer;
    uint8_t svc_type; // type=0; use database(16 uuid); type=1:use database(128 uuid); type=2; user custom
    struct tspps_sdp_svc svc;
};

/// Parameters of the @ref TSPP_SERVER_ENABLE_REQ message
struct tspp_server_enable_req
{
    /// connection index
    uint8_t  conidx;
    /// Notification Configuration
    uint8_t  ntf_cfg;
};

/// Parameters of the @ref TSPP_SERVER_ENABLE_RSP message
struct tspp_server_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref TSPP_SERVER_NTF_CFG_IND message
struct tspp_server_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint8_t  ntf_cfg;
};

///Parameters of the @ref TSPP_SERVER_SEND_NTF_CMD message
struct tspp_server_send_ntf_cmd
{
    /// connection index
    uint8_t  conidx;
    uint16_t length;
    uint8_t value[__ARRAY_EMPTY];
};


///Parameters of the @ref TSPP_SERVER_WRITE_IND message
struct tspp_server_write_ind
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
struct tspp_server_buffer_empty_evt
{
    /// GATT request type
    uint8_t operation;
    /// Status of the request
    uint8_t status;
};

/// buffer full event data structure
struct tspp_server_buffer_full_evt
{
    uint8_t operation;
    uint8_t status;
};

/// error event data structure
struct tspp_server_error_evt
{
    /// GATT request type
    uint16_t operation;
    /// Status of the request
    uint8_t status;
};

/// @} TSPPSTASK

/**
 ****************************************************************************************
 * @brief get free size
 *
 * @param[in] conidx , the connect index
 * @param[out] the free size
 * ****************************************************************************************
 */
uint32_t tspp_server_get_free_size(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief check fifo is full
 *
 * @param[in] conidx , the connect index
 * @param[out] 0; buffer not full. 1: buffer full
 * ****************************************************************************************
 */
uint8_t tspp_server_check_fifo_full(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief set mtu size
 *
 * @param[in] conidx , the connect index
 * @param[in] mtu; mtu size
 * ****************************************************************************************
 */
void    tspp_server_set_max_mtu(uint8_t conidx, uint16_t mtu);

/**
 ****************************************************************************************
 * @brief set data len
 *
 * @param[in] conidx , the connect index
 * @param[in] tx_len; max tx_len
 * ****************************************************************************************
 */
void    tspp_server_set_data_len(uint8_t conidx, uint16_t tx_len);

/**
 ****************************************************************************************
 * @brief get the perfect once tx len
 *
 * @param[in] conidx , the connect index
 * @param[in] mto: mto len
 * @param[in] char_len: char len
 * ****************************************************************************************
 */
uint16_t tspp_server_perfect_once_tx_length(uint16_t mtu, uint16_t mto, uint16_t char_len);
#endif /* _TSPPS_TASK_H_ */
