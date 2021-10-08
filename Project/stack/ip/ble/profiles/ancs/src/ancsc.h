/**
 ****************************************************************************************
 *
 * @file ancsc.h
 *
 * @brief Header file - ANCS - Client Role.
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */

#ifndef _ANCSC_H_
#define _ANCSC_H_

/**
 ****************************************************************************************
 * @addtogroup ACNSC Client
 * @ingroup ANCS
 * @brief ANCS Client
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
#include "ancsc_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Battery Client task instances
#define ANCSC_IDX_MAX     (BLE_CONNECTION_MAX)

/// Possible states of the BAPC task

#define ANCSC_ST_ENCRYPTED 0x80
#define ANCS_GET_ST_ENCRYPTED(state) (state & ANCSC_ST_ENCRYPTED)
#define ANCS_SET_ST_ENCRYPTED(state) (state | ANCSC_ST_ENCRYPTED)
#define ANCS_GET_STATE(state)        (state & ~ANCSC_ST_ENCRYPTED)
#define ANCS_NEW_STATE(state, new_state) (ANCS_GET_ST_ENCRYPTED(state) | new_state)

enum ancsc_state
{
    /// Disconnected state
    ANCSC_ST_FREE,
	ANCSC_ST_IDLE,
	ANCSC_ST_DISCOVER,
	ANCSC_ST_REGISTER_GATTC,
	ANCSC_ST_ENABLE_DATA,
	ANCSC_ST_ENABLE_NTF,
	ANCSC_ST_WAIT_NTF,
	ANCSC_ST_WAIT_DATA,
	ANCSC_ST_PERFORM,
	ANCSC_ST_ERROR,
	/// Number of defined states.
	ANCSC_STATE_MAX
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
struct ancsc_cnx_env
{
    ///ANCS characteristics
    struct ancs_content ancs;
};

/// ANCS 'Profile' Client environment variable
struct ancsc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct ancsc_cnx_env* env[ANCSC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[ANCSC_IDX_MAX];
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
 * @brief Retrieve ANCS client profile interface
 *
 * @return ANCS client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* ancsc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send ANCS ATT DB discovery results to ANCSC host.
 ****************************************************************************************
 */
void ancsc_enable_rsp_send(struct ancsc_env_tag *ancsc_env, uint8_t conidx, uint8_t status);

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
void ancsc_task_init(struct ke_task_desc *task_desc);

/// @} ANCSC

#endif /* _ANCSC_H_ */
