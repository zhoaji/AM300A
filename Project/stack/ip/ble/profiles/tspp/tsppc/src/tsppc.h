/**
 ****************************************************************************************
 *
 * @file tsppc.h
 *
 * @brief Header file - Transport Profile - Collector Role.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _TSPPC_H_
#define _TSPPC_H_

/**
 ****************************************************************************************
 * @addtogroup Transport Profile Collector
 * @ingroup TSPP
 * @brief Transport Profile Collector
 *
 * The TSPPC is responsible for providing Transport Profile Collector functionalities
 * to upper layer module or application. The device using this profile takes the role
 * of Transport Profile Collector.
 *
 * Transport Profile Collector. (TSPPC): A TSPPC (e.g. PC, phone, etc)
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#if (BLE_TSPP_COLLECTOR)

#include "tspp_common.h"
#include "tsppc_task.h"
#include "ke_task.h"
#include "prf_types.h"
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Heart Rate Collector task instances
#define TSPPC_IDX_MAX    (BLE_CONNECTION_MAX)

/// Possible states of the TSPPC task
enum
{
    /// Free state
    TSPPC_FREE,
    /// Idle state
    TSPPC_IDLE,
    /// Discovering SVC and Chars
    TSPPC_DISCOVERING,
    /// Busy state
    TSPPC_BUSY,

    /// Number of defined states.
    TSPPC_STATE_MAX
};

/// Internal codes for reading a TSPPS characteristic with one single request
enum
{
    ATT_SERVICE_TSPP                        = ATT_UUID_16(0xFF00),
    ATT_CHAR_TSPP_UPLOAD                    = ATT_UUID_16(0xFF01),
    ATT_CHAR_TSPP_REV1                      = ATT_UUID_16(0xFF02),
    ATT_CHAR_TSPP_REV2                      = ATT_UUID_16(0xFF03),
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

struct tsppc_cnx_env
{
    /// Current operation code
    void *operation;
    /// Last requested UUID(to keep track of the two services and char)
    uint16_t last_uuid_req;
    /// Last characteristic code used in a read or a write request
    uint16_t last_char_code;
    /// Counter used to check service uniqueness
    uint8_t nb_svc;
    ///HTS characteristics
    struct tspps_content tspps;
};

/// Transport Profile Collector environment variable
struct tsppc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connection
    struct tsppc_cnx_env* env[TSPPC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[TSPPC_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve TSPP client profile interface
 * @return TSPP client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* tsppc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send Heart Rate ATT DB discovery results to TSPPC host.
 ****************************************************************************************
 */
void tsppc_enable_rsp_send(struct tsppc_env_tag *tsppc_env, uint8_t conidx, uint8_t status);


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
void tsppc_task_init(struct ke_task_desc *task_desc);


#endif /* (BLE_TSPP_COLLECTOR) */

/// @} TSPPC

#endif /* _TSPPC_H_ */
