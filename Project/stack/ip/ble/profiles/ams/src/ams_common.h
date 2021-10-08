/**
 ****************************************************************************************
 *
 * @file ams_common.h
 *
 * @brief Header File - AMS Profile common types.
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */


#ifndef _AMS_COMMON_H_
#define _AMS_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup AMS Profile
 * @ingroup PROFILE
 * @brief AMS Profile
 *
 * The AMS module is the responsible block for implementing the AMS Profile
 * functionalities in the BLE Host.
 *
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#include "prf_types.h"
#include <stdint.h>

/*
 * DEFINES
 ****************************************************************************************
 */
 
/// RemoteCommandID Values
enum ams_RemoteCommandID_values 
{
    AMS_CMD_Play               = 0,
    AMS_CMD_Pause              = 1,
    AMS_CMD_TogglePlayPause    = 2,
    AMS_CMD_NextTrack          = 3,
    AMS_CMD_PreviousTrack      = 4,
    AMS_CMD_VolumeUp           = 5,
    AMS_CMD_VolumeDown         = 6,
    AMS_CMD_AdvanceRepeatMode  = 7,
    AMS_CMD_AdvanceShuffleMode = 8,
    AMS_CMD_SkipForward        = 9,
    AMS_CMD_SkipBackward       = 10,
    AMS_CMD_LikeTrack          = 11,
    AMS_CMD_DislikeTrack       = 12,
    AMS_CMD_BookmarkTrack      = 13,
    AMS_CMD_Reserved           = 255,
};

/// EntityID Values
enum ams_EntityID_values
{
    AMS_EntityIDPlayer   = 0,
    AMS_EntityIDQueue    = 1,
    AMS_EntityIDTrack    = 2,
    AMS_EntityIDReserved = 255,
};

/// EntityUpdateFlags Values
enum AMS_EntityUpdateFlags 
{
    AMS_EntityUpdateFlagTruncated = 1 << 0,
};

/// PlayerAttributeID Values
enum AMS_PlayerAttributeID_values
{
    AMS_PlayerAttributeIDName = 0,
    AMS_PlayerAttributeIDPlaybackInfo,
    AMS_PlayerAttributeIDVolume,
};

/// PlayerAttributeID Values
enum AMS_QueueAttributeID_values
{
    AMS_QueueAttributeIDIndex = 0,
    AMS_QueueAttributeIDCount,
    AMS_QueueAttributeIDShuffleMode,
    AMS_QueueAttributeIDRepeatMode,
};

/// Shuffle Mode Constants
enum AMS_ShuffleMode_values
{
    AMS_ShuffleModeOff = 0,
    AMS_ShuffleModeOne,
    AMS_ShuffleModeAll,
};

/// Repeat Mode Constants
enum AMS_RepeatMode_values
{
    AMS_RepeatModeOff = 0,
    AMS_RepeatModeOne,
    AMS_RepeatModeAll,
};

/// TrackAttributeID Values
enum AMS_TrackAttributeID_values
{
    AMS_TrackAttributeIDArtist = 0,
    AMS_TrackAttributeIDAlbum,
    AMS_TrackAttributeIDTitle,
    AMS_TrackAttributeIDDuration,
};

/// Error Codes Values
enum AMS_ErrorCodes_values
{
    AMS_InvalidState    = 0xA0,
    AMS_InvalidCommand  = 0xA1,
    AMS_AbsentAttribute = 0xA2,
};

/// @} ams_common

#endif /* _AMS_COMMON_H_ */
