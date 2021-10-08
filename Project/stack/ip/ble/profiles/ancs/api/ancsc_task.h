/**
 ****************************************************************************************
 *
 * @file ancsc_task.h
 *
 * @brief Header file - ANCS Client Role Task. V201130.1.0
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */


#ifndef _ANCSC_TASK_H_
#define _ANCSC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup ANCSCTASK ANCS Client Task
 * @ingroup ANCSC
 * @brief ANCS Client Task
 *
 * The ANCSCTASK is responsible for handling the messages coming in and out of the
 * @ref ANCSC block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>
#include "rwip_task.h" // Task definitions
#include "prf_types.h"
#include "ancs_common.h"
#include "co_debug.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#if 1
#define ancs_debug log_debug
#define ancs_debug_array_ex log_debug_array_ex
#else
#define ancs_debug(...)
#define ancs_debug_array_ex(...)
#endif

/*capacity*/
#define ANCS_APPID_CAP        40
#define ANCS_TITLE_CAP        32
#define ANCS_SUBTITLE_CAP     16
#define ANCS_MSG_CAP          200
#define ANCS_DATE_CAP         15
#define ANCS_ACT_LABEL_CAP    4
#define ANCS_MAX_PENDING_NUM  16
#define ANCS_GET_ATT_TOUT_MS  3000

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*@TRACE*/
enum ancsc_msg_id
{
    /// Start the ANCS @ref ancsc_enable_req
    ANCSC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_ANCSC),
    ///Confirm that cfg connection has finished with discovery results @ref ancsc_enable_rsp
    ANCSC_ENABLE_RSP,
    /// Link encrypted
    ANCSC_ENCRYPTED_CMD,
    /// Notification source ind @ref ancsc_notification_ind
    ANCSC_NOTIFICATION_IND,
	/// Get notification attribute command @ref ancsc_get_ntf_attr_cmd
    ANCSC_GET_NTF_ATTR_CMD,
    /// notification attribute indicate @ref ancsc_ntf_attr_ind
    ANCSC_NTF_ATTR_IND,
	/// Get APP attribute command @ref ancsc_get_app_attr_cmd
    ANCSC_GET_APP_ATTR_CMD,
    /// application attribute indicate @ref ancsc_app_attr_ind
    ANCSC_APP_ATTR_IND,
	/// Perform notification action @ref ancsc_perform_act_cmd
    ANCSC_PERFORM_ACT_CMD,
};

/// ANCS Characteristics
enum ancs_char_type
{
	ANCS_CNTL_POINT_CHAR,
	ANCS_NTF_SOURCE_CHAR,
	ANCS_DATA_SOURCE_CHAR,
	ANCS_CHAR_MAX,
};

/// ANCS Descriptors
enum ancs_desc_type
{
    /// ANCSC Client NTF Characteristic Configuration
	ANCS_DESC_NTF_SRC_CFG,
	ANCS_DESC_DATA_SRC_CFG,
	ANCS_DESC_MAX,
};

/*
 * APIs Structure
 ****************************************************************************************
 */

///Structure containing the characteristics handles, value handles and descriptors
struct ancs_content
{
    /// service info
    struct prf_svc svc;
    /// Characteristic Info:
    struct prf_char_inf chars[ANCS_CHAR_MAX];
    /// Descriptor handles:
    struct prf_char_desc_inf descs[ANCS_DESC_MAX];
};


/// Parameters of the @ref ANCSC_ENABLE_REQ message
struct ancsc_enable_req
{
    uint8_t  conidx;
    ///Connection type
    uint8_t con_type;
    /// Existing handle values ancs
    struct ancs_content ancs;
};

/// Parameters of the @ref ANCSC_ENABLE_RSP message
struct ancsc_enable_rsp
{
    /// Status
    uint8_t status;
    ///Existing handle values ancs
    struct ancs_content ancs;
};


///Parameters of the @ref ANCSC_NOTIFICATION_IND message
struct ancsc_notification_ind
{
	uint8_t event_id;          // @ref ancs_eventID_values
	uint8_t event_flags;       // @ref ancs_event_flags
	uint8_t category_id;       // @ref ancs_categoryID_values
	uint8_t category_count;    // category count
	uint32_t notification_uid; // message UID
};

///Parameters of the @ref ANCSC_GET_NTF_ATTR_CMD message
struct ancsc_get_ntf_attr_cmd
{
	uint32_t uid; // message UID
};

///Parameters of the @ref ANCSC_NTF_ATTR_IND message
struct ancsc_ntf_attr_ind
{
	uint32_t notif_uid;
	uint8_t date[ANCS_DATE_CAP];
	uint16_t app_id_len;
	uint16_t title_len;
	uint16_t subtitle_len;
	uint16_t msg_len;

	uint8_t *app_id;
	uint8_t *title;
	uint8_t *subtitle;
	uint8_t *msg;
	uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref ANCSC_GET_APP_ATTR_CMD message
struct ancsc_get_app_attr_cmd
{
	uint8_t attr_id_len;
	uint8_t id[__ARRAY_EMPTY];
};

///Parameters of the @ref ANCSC_APP_ATTR_IND message
struct ancsc_app_attr_ind
{
	uint8_t app_attr_len;
	uint8_t app_attr[__ARRAY_EMPTY];
};

///Parameters of the @ref ANCSC_PERFORM_ACT_CMD message
struct ancsc_perform_act_cmd
{
	uint32_t uid; // message UID
	uint8_t act_id; // The action to be performed on the iOS notifcation. @ref ancs_actionID_values
};


/**
 ****************************************************************************************
 * @brief notify ancs_task the link is encrypted by app.
 ****************************************************************************************
 */
void ancsc_encrypted_cmd(uint8_t conidx);


/// @} ANCSCTASK

#endif /* _ANCSC_TASK_H_ */
