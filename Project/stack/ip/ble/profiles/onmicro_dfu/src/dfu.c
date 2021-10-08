/**
 ****************************************************************************************
 *
 * @file onmicro_dfu.c
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
 * @addtogroup ONMICRO_DFU
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if BLE_APP_ONMICRO_DFU
#include "dfu.h"
#include "dfu_task.h"
#include "prf_utils.h"
#include "prf.h"

#include "ke_mem.h"

/*
 * ONMICRO_DFU ATTRIBUTES DEFINITION
 ****************************************************************************************
 */
/// Full ONMICRO_DFU Database Description - Used to add attributes into the database
const struct attm_desc_128 onmicro_dfu_att_db[] =
{
    // Service Declaration
    [ONMICRO_DFU_IDX_SVC]            =   {ATT_16_TO_128_ARRAY(ATT_DECL_PRIMARY_SERVICE),  PERM(RD, ENABLE), 0, 0},

    // Characteristic Declaration
    [ONMICRO_DFU_IDX_CTRL_CHAR]      =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // Characteristic Value
    [ONMICRO_DFU_IDX_CTRL_VAL]       =   {ONMICRO_DFU_CTRL_UUID,    PERM(WRITE_REQ, ENABLE) | PERM(NTF, ENABLE), PERM(UUID_LEN, UUID_128), ONMICRO_DFU_PKG_MAX_LEN},
    // Characteristic - Client Characteristic Configuration Descriptor
    [ONMICRO_DFU_IDX_CTRL_DESC]      =   {ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG),  PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE), 0},

    // Characteristic Declaration
    [ONMICRO_DFU_IDX_PKG_CHAR]       =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // Characteristic Value
    [ONMICRO_DFU_IDX_PKG_VAL]        =   {ONMICRO_DFU_PKG_UUID,    PERM(RD, ENABLE) | PERM(WRITE_COMMAND, ENABLE), PERM(UUID_LEN, UUID_128), ONMICRO_DFU_PKG_MAX_LEN},

    // Characteristic Declaration
    [ONMICRO_DFU_IDX_VERSION_CHAR]   =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC),   PERM(RD, ENABLE), 0, 0},
    // Characteristic Value
    [ONMICRO_DFU_IDX_VERSION_VAL]    =   {ONMICRO_DFU_VERSION_UUID,    PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE) | PERM(NTF, ENABLE), PERM(RI, ENABLE), 4},
    // Characteristic - Client Characteristic Configuration Descriptor
    [ONMICRO_DFU_IDX_VERSION_DESC]      ={ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG),  PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE), 0},

};

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the ONMICRO_DFU module.
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
static uint8_t onmicro_dfu_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  void* params)
{
    uint16_t shdl;
    struct onmicro_dfu_env_tag* onmicro_dfu_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    //-------------------- allocate memory required for the profile  ---------------------
    onmicro_dfu_env = (struct onmicro_dfu_env_tag* ) ke_malloc(sizeof(struct onmicro_dfu_env_tag), KE_MEM_ATT_DB);
    memset(onmicro_dfu_env, 0 , sizeof(struct onmicro_dfu_env_tag));

    shdl = *start_hdl;

    //Create ONMICRO_DFU in the DB
    //------------------ create the attribute database for the profile -------------------
    uint8_t service_uuid[ATT_UUID_128_LEN] = ATT_16_TO_128_ARRAY(BLE_UUID_ONMICRO_DFU_SERVICE);
    status = attm_svc_create_db_128(&shdl, service_uuid, NULL,
            ONMICRO_DFU_IDX_NB, NULL, env->task, onmicro_dfu_att_db,
            ((sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS))));

    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // allocate ONMICRO_DFU required environment variable
        env->env = (prf_env_t*) onmicro_dfu_env;
        *start_hdl = shdl;
        onmicro_dfu_env->start_hdl = *start_hdl;
        onmicro_dfu_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        onmicro_dfu_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_ONMICRO_DFU;
        onmicro_dfu_task_init(&(env->desc));
    }
    else if(onmicro_dfu_env != NULL)
    {
        ke_free(onmicro_dfu_env);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the ONMICRO_DFU module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void onmicro_dfu_destroy(struct prf_task_env* env)
{
    struct onmicro_dfu_env_tag* onmicro_dfu_env = (struct onmicro_dfu_env_tag*) env->env;

    // clear on-going operation
    if(onmicro_dfu_env->operation != NULL)
    {
        ke_free(onmicro_dfu_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(onmicro_dfu_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void onmicro_dfu_create(struct prf_task_env* env, uint8_t conidx)
{
    struct onmicro_dfu_env_tag* onmicro_dfu_env = (struct onmicro_dfu_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    onmicro_dfu_env->ntf_cfg[conidx] = 0;
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
static void onmicro_dfu_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct onmicro_dfu_env_tag* onmicro_dfu_env = (struct onmicro_dfu_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    onmicro_dfu_env->ntf_cfg[conidx] = 0;
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// ONMICRO_DFU Task interface required by profile manager
const struct prf_task_cbs onmicro_dfu_itf =
{
        (prf_init_fnct) onmicro_dfu_init,
        onmicro_dfu_destroy,
        onmicro_dfu_create,
        onmicro_dfu_cleanup,
};


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* onmicro_dfu_prf_itf_get(void)
{
   return &onmicro_dfu_itf;
}

uint16_t onmicro_dfu_get_att_handle(uint8_t att_idx)
{
    struct onmicro_dfu_env_tag* onmicro_dfu_env = PRF_ENV_GET(ONMICRO_DFU, onmicro_dfu);
    uint16_t handle = onmicro_dfu_env->start_hdl + att_idx;
    return handle;
}

uint8_t onmicro_dfu_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct onmicro_dfu_env_tag* onmicro_dfu_env = PRF_ENV_GET(ONMICRO_DFU, onmicro_dfu);
    *att_idx = handle - onmicro_dfu_env->start_hdl;
    return ATT_ERR_NO_ERROR;
}

#endif //BLE_APP_ONMICRO_DFU

/// @} ONMICRO_DFU
