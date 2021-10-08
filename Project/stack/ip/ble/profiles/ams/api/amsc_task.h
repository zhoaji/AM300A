/**
 ****************************************************************************************
 *
 * @file amsc_task.h
 *
 * @brief Header file - AMS Client Role Task.
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */


#ifndef _AMSC_TASK_H_
#define _AMSC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup AMSCTASK AMS Client Task
 * @ingroup AMSC
 * @brief AMS Client Task
 *
 * The AMSCTASK is responsible for handling the messages coming in and out of the
 * @ref AMSC block of the BLE Host.
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
#include "ams_common.h"
#include "co_debug.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#if 0
#define ams_debug log_debug
#define ams_debug_array_ex log_debug_array_ex
#else
#define ams_debug(...)
#define ams_debug_array_ex(...)
#endif

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*@TRACE*/
enum amsc_msg_id
{
    /// Start the AMS @ref amsc_enable_req
    AMSC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_AMSC),
    ///Confirm that cfg connection has finished with discovery results @ref amsc_enable_rsp
    AMSC_ENABLE_RSP,
    /// Link encrypted
    AMSC_ENCRYPTED_CMD,
    AMSC_REM_CTRL_CMD,
    AMSC_REM_CTRL_IND,
    AMSC_ENTITY_ATT_IND,
};

enum ams_attr_subscribe
{
    PlayerAttributeIDName = 0,
    PlayerAttributeIDPlaybackInfo,
    PlayerAttributeIDVolume,
    QueueAttributeIDIndex,
    QueueAttributeIDCount,
    QueueAttributeIDShuffleMode,
    QueueAttributeIDRepeatMode,
    TrackAttributeIDArtist,
    TrackAttributeIDAlbum,
    TrackAttributeIDTitle,
    TrackAttributeIDDuration,
};

/// AMS Characteristics
enum ams_char_type
{
	AMS_REMOTE_COMMAND_CHAR,
	AMS_ENTITY_UPDATE_CHAR,
	AMS_ENTITY_ATTR_CHAR,
	AMS_CHAR_MAX,
};

/// AMS Descriptors
enum ams_desc_type
{
    /// AMSC Client NTF Characteristic Configuration
	AMS_DESC_REMOTE_COMMAND_CFG,
	AMS_DESC_ENTITY_UPDATE_CFG,
	AMS_DESC_MAX,
};

/*
 * APIs Structure
 ****************************************************************************************
 */

///Structure containing the characteristics handles, value handles and descriptors
struct ams_content
{
    /// service info
    struct prf_svc svc;
    /// Characteristic Info:
    struct prf_char_inf chars[AMS_CHAR_MAX];
    /// Descriptor handles:
    struct prf_char_desc_inf descs[AMS_DESC_MAX];
};

/// Parameters of the @ref AMSC_ENABLE_REQ message
struct amsc_enable_req
{
    uint8_t  conidx;
    ///Connection type
    uint8_t con_type;
    /// Existing handle values ams
    struct ams_content ams;
    uint16_t ams_subscribe;
};

/// Parameters of the @ref AMSC_ENABLE_RSP message
struct amsc_enable_rsp
{
    /// Status
    uint8_t status;
    ///Existing handle values ams
    struct ams_content ams;
};


///Parameters of the @ref AMSC_ENTITY_ATT_IND message
struct amsc_entity_att_ind
{
	uint8_t subscribe_bit;
	uint8_t flag;
	uint16_t len;
	uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref AMSC_REM_CTRL_CMD message
struct amsc_rem_ctrl_cmd
{
	enum ams_RemoteCommandID_values value;
};

///Parameters of the @ref AMSC_REM_CTRL_IND message
struct amsc_rem_ctrl_ind
{
	uint16_t len;
	enum ams_RemoteCommandID_values value[__ARRAY_EMPTY];
};

/**
 ****************************************************************************************
 * @brief notify ams_task the link is encrypted by app.
 ****************************************************************************************
 */
void amsc_encrypted_cmd(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Mobile media remote control command
 ****************************************************************************************
 */
void amsc_remote_ctrl_cmd(uint8_t conidx, enum ams_RemoteCommandID_values value);


/// @} AMSCTASK

#endif /* _AMSC_TASK_H_ */
