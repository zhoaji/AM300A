/**
 ****************************************************************************************
 *
 * @file simple_server.c
 *
 * @brief Server Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup SIMPLE_SERVER
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "simple_server.h"
#include "simple_server_task.h"
#include "prf_utils.h"
#include "prf.h"

#include "ke_mem.h"

/*
 * SIMPLE_SERVER ATTRIBUTES DEFINITION
 ****************************************************************************************
 */
/// Full SIMPLE_SERVER Database Description - Used to add attributes into the database
const struct attm_desc_128 simple_server_att_db[SIMPLE_SERVER_IDX_NB] =
{
    // Service Declaration
    [SIMPLE_SERVER_IDX_SVC]               =   {ATT_16_TO_128_ARRAY(ATT_DECL_PRIMARY_SERVICE),  PERM(RD, ENABLE), 0, 0},

    // Characteristic Declaration
    [SIMPLE_SERVER_IDX_DEMO_CHAR1]        =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // Characteristic Value
    [SIMPLE_SERVER_IDX_DEMO_VAL1]         =   {ATT_SVC_SIMPLE_SERVER_CHAC1,    PERM(WRITE_REQ, ENABLE) | PERM(NTF, ENABLE) ,PERM(UUID_LEN, UUID_128), SIMPLE_SERVER_MAX_CHAC_LEN},
    // Characteristic - Client Characteristic Configuration Descriptor
    [SIMPLE_SERVER_IDX_DEMO_NTF_CFG]     =   {ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG),  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

    // Characteristic Declaration
    [SIMPLE_SERVER_IDX_DEMO_CHAR2]        =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // Characteristic Value
    [SIMPLE_SERVER_IDX_DEMO_VAL2]         =   {ATT_SVC_SIMPLE_SERVER_CHAC2,    PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), PERM(UUID_LEN, UUID_128), SIMPLE_SERVER_MAX_CHAC_LEN},
};

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the SIMPLE_SERVER module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    env        Collector or Service allocated environment data.
 * @param[in|out] start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task   Application task number.
 * @param[in]     sec_lvl    Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     param      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t simple_server_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct simple_server_db_cfg* params)
{
    uint16_t shdl;
    struct simple_server_env_tag* simple_server_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    //-------------------- allocate memory required for the profile  ---------------------
    simple_server_env = (struct simple_server_env_tag* ) ke_malloc(sizeof(struct simple_server_env_tag), KE_MEM_ATT_DB);
    memset(simple_server_env, 0 , sizeof(struct simple_server_env_tag));

    shdl = *start_hdl;

    //Create SIMPLE_SERVER in the DB
    //------------------ create the attribute database for the profile -------------------
    uint8_t service_uuid[ATT_UUID_128_LEN] = ATT_SVC_SIMPLE_SERVER_SERVICE;
    status = attm_svc_create_db_128(&shdl, service_uuid, NULL,
            SIMPLE_SERVER_IDX_NB, NULL, env->task, simple_server_att_db,
            ((sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_UUID_LEN, UUID_128)));

    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // allocate SIMPLE_SERVER required environment variable
        env->env = (prf_env_t*) simple_server_env;
        *start_hdl = shdl;
        simple_server_env->start_hdl = *start_hdl;
        simple_server_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        simple_server_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_SIMPLE_SERVER;
        simple_server_task_init(&(env->desc));

        // service is ready, go into an Idle state
        ke_state_set(env->task, SIMPLE_SERVER_IDLE);
    }
    else if(simple_server_env != NULL)
    {
        ke_free(simple_server_env);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the SIMPLE_SERVER module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void simple_server_destroy(struct prf_task_env* env)
{
    struct simple_server_env_tag* simple_server_env = (struct simple_server_env_tag*) env->env;

    // clear on-going operation
    if(simple_server_env->operation != NULL)
    {
        ke_free(simple_server_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(simple_server_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void simple_server_create(struct prf_task_env* env, uint8_t conidx)
{
    struct simple_server_env_tag* simple_server_env = (struct simple_server_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    simple_server_env->ntf_cfg[conidx] = PRF_CLI_START_NTF; // 20210707  0;
}

/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void simple_server_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct simple_server_env_tag* simple_server_env = (struct simple_server_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    simple_server_env->ntf_cfg[conidx] = PRF_CLI_START_NTF; // 20210707  0;
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// SIMPLE_SERVER Task interface required by profile manager
const struct prf_task_cbs simple_server_itf =
{
        (prf_init_fnct) simple_server_init,
        simple_server_destroy,
        simple_server_create,
        simple_server_cleanup,
};


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* simple_server_prf_itf_get(void)
{
   return &simple_server_itf;
}

uint16_t simple_server_get_att_handle(uint8_t att_idx)
{
    struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);
    uint16_t handle = simple_server_env->start_hdl + att_idx;
    return handle;
}

uint8_t simple_server_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);
    *att_idx = handle - simple_server_env->start_hdl;
    return ATT_ERR_NO_ERROR;
}

/// @} SIMPLE_SERVER
