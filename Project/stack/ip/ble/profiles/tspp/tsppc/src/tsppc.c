/**
 ****************************************************************************************
 *
 * @file tsppc.c
 *
 * @brief Transport Profile Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup TSPPC
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
#include "tsppc.h"
#include "tsppc_task.h"

#include "ke_mem.h"
#include "co_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the tsppC module.
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
static uint8_t tsppc_init(struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  void* params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct tsppc_env_tag* tsppc_env =
            (struct tsppc_env_tag* ) ke_malloc(sizeof(struct tsppc_env_tag), KE_MEM_ATT_DB);

    // allocate TSPPC required environment variable
    env->env = (prf_env_t*) tsppc_env;

    tsppc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    tsppc_env->prf_env.prf_task = env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    env->id                     = TASK_ID_TSPPC;
    tsppc_task_init(&(env->desc));

    for(idx = 0; idx < TSPPC_IDX_MAX ; idx++)
    {
        tsppc_env->env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(env->task, idx), TSPPC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Clean-up connection dedicated environment parameters
 * This function performs cleanup of ongoing operations
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void tsppc_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct tsppc_env_tag* tsppc_env = (struct tsppc_env_tag*) env->env;

    // clean-up environment variable allocated for task instance
    if(tsppc_env->env[conidx] != NULL)
    {
        ke_free(tsppc_env->env[conidx]);
        tsppc_env->env[conidx] = NULL;
    }

    /* Put Client in Free state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), TSPPC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the TSPPC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void tsppc_destroy(struct prf_task_env* env)
{
    uint8_t idx;
    struct tsppc_env_tag* tsppc_env = (struct tsppc_env_tag*) env->env;

    // cleanup environment variable for each task instances
    for(idx = 0; idx < TSPPC_IDX_MAX ; idx++)
    {
        tsppc_cleanup(env, idx, 0);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(tsppc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void tsppc_create(struct prf_task_env* env, uint8_t conidx)
{
    /* Put Client in Idle state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), TSPPC_IDLE);
}

/// Task interface required by profile manager
const struct prf_task_cbs tsppc_itf =
{
        tsppc_init,
        tsppc_destroy,
        tsppc_create,
        tsppc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* tsppc_prf_itf_get(void)
{
   return &tsppc_itf;
}


/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void tsppc_enable_rsp_send(struct tsppc_env_tag *tsppc_env, uint8_t conidx, uint8_t status)
{
    //send APP the details of the discovered attributes on HTPT
    struct tsppc_enable_rsp * rsp = KE_MSG_ALLOC(
            TSPPC_ENABLE_RSP,
            prf_dst_task_get(&(tsppc_env->prf_env), conidx),
            prf_src_task_get(&(tsppc_env->prf_env), conidx),
            tsppc_enable_rsp);

    rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        rsp->tspps = tsppc_env->env[conidx]->tspps;
        //register task in gatt for indication/notifications
        prf_register_atthdl2gatt(&tsppc_env->prf_env, conidx, &tsppc_env->env[conidx]->tspps.svc);
        // Go to connected state
        ke_state_set(prf_src_task_get(&(tsppc_env->prf_env), conidx), TSPPC_IDLE);
    }

    ke_msg_send(rsp);
}

void tsppc_error_ind_send(struct tsppc_env_tag *tsppc_env, uint8_t conidx, uint8_t status)
{

}

#endif /* (BLE_TSPP_COLLECTOR) */

/// @} TSPPC
