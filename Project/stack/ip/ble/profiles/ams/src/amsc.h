/**
 ****************************************************************************************
 *
 * @file amsc.h
 *
 * @brief Header file - AMS - Client Role.
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */

#ifndef _AMSC_H_
#define _AMSC_H_

/**
 ****************************************************************************************
 * @addtogroup ACNSC Client
 * @ingroup AMS
 * @brief AMS Client
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#include <stdint.h>
#include <stdbool.h>
#include "ke_task.h"
#include "prf_types.h"
#include "prf_utils.h"
#include "amsc_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Battery Client task instances
#define AMSC_IDX_MAX     (BLE_CONNECTION_MAX)

/// Possible states of the BAPC task

#define AMSC_ST_ENCRYPTED 0x80
#define AMS_GET_ST_ENCRYPTED(state) (state & AMSC_ST_ENCRYPTED)
#define AMS_SET_ST_ENCRYPTED(state) (state | AMSC_ST_ENCRYPTED)
#define AMS_GET_STATE(state)        (state & ~AMSC_ST_ENCRYPTED)
#define AMS_NEW_STATE(state, new_state) (AMS_GET_ST_ENCRYPTED(state) | new_state)

enum amsc_state
{
    /// Disconnected state
    AMSC_ST_FREE,
	AMSC_ST_IDLE,
	AMSC_ST_DISCOVER,
	AMSC_ST_REGISTER_GATTC,
	AMSC_ST_ENABLE_REM_CMD,
	AMSC_ST_ENABLE_ENTITY_UPDATE,
	AMSC_ST_SET_PLAYER_UPDATE,
	AMSC_ST_SET_QUEUE_UPDATE,
	AMSC_ST_SET_TRACK_UPDATE,
	AMSC_ST_WAIT_NTF,
	AMSC_ST_WAIT_DATA,        ///////////////
	AMSC_ST_PERFORM,
	AMSC_ST_ERROR,
	/// Number of defined states.
	AMSC_STATE_MAX
};

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Environment variable for each Connections
struct amsc_cnx_env
{
    ///AMS characteristics
    struct ams_content ams;
    uint16_t ams_subscribe;
};

/// AMS 'Profile' Client environment variable
struct amsc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct amsc_cnx_env* env[AMSC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[AMSC_IDX_MAX];
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
 * @brief Retrieve AMS client profile interface
 *
 * @return AMS client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* amsc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send AMS ATT DB discovery results to AMSC host.
 ****************************************************************************************
 */
void amsc_enable_rsp_send(struct amsc_env_tag *amsc_env, uint8_t conidx, uint8_t status);

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
void amsc_task_init(struct ke_task_desc *task_desc);

/// @} AMSC

#endif /* _AMSC_H_ */
