/**
 ****************************************************************************************
 *
 * @file ancsc.c
 *
 * @brief ANCS implementation.
 *
 * Copyright (C) 2020-2030 OnMicro Limited. All rights reserved.
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup ANCSC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"


#include "gap.h"
#include "ancsc.h"
#include "ancsc_task.h"
#include "co_math.h"

#include "ke_mem.h"

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the ANCSC module.
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
static uint8_t ancsc_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  void* params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct ancsc_env_tag* ancsc_env =
            (struct ancsc_env_tag* ) ke_malloc(sizeof(struct ancsc_env_tag), KE_MEM_ATT_DB);

    // allocate ANCSC required environment variable
    env->env = (prf_env_t*) ancsc_env;

    ancsc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    ancsc_env->prf_env.prf_task = env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    env->id                     = TASK_ID_ANCSC;
    ancsc_task_init(&(env->desc));

    for(idx = 0; idx < ANCSC_IDX_MAX ; idx++)
    {
        ancsc_env->env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(env->task, idx), ANCSC_ST_FREE);
    }


    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Destruction of the ANCSC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void ancsc_destroy(struct prf_task_env* env)
{
    uint8_t idx;
    struct ancsc_env_tag* ancsc_env = (struct ancsc_env_tag*) env->env;

    // cleanup environment variable for each task instances
    for(idx = 0; idx < ANCSC_IDX_MAX ; idx++)
    {
        if(ancsc_env->env[idx] != NULL)
        {
            ke_free(ancsc_env->env[idx]);
        }
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(ancsc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void ancsc_create(struct prf_task_env* env, uint8_t conidx)
{
    /* Put ANCS Client in Idle state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), ANCSC_ST_IDLE);
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
static void ancsc_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct ancsc_env_tag* ancsc_env = (struct ancsc_env_tag*) env->env;

    // clean-up environment variable allocated for task instance
    if(ancsc_env->env[conidx] != NULL)
    {
        ke_free(ancsc_env->env[conidx]);
        ancsc_env->env[conidx] = NULL;
    }

    /* Put ANCS Client in Free state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), ANCSC_ST_FREE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// ANCSC Task interface required by profile manager
const struct prf_task_cbs ancsc_itf =
{
        ancsc_init,
        ancsc_destroy,
        ancsc_create,
        ancsc_cleanup,
};


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* ancsc_prf_itf_get(void)
{
   return &ancsc_itf;
}

void ancsc_enable_rsp_send(struct ancsc_env_tag *ancsc_env, uint8_t conidx, uint8_t status)
{
    // Send APP the details of the discovered attributes on ANCSC
    struct ancsc_enable_rsp * rsp = KE_MSG_ALLOC(ANCSC_ENABLE_RSP,
                                                prf_dst_task_get(&(ancsc_env->prf_env) ,conidx),
                                                prf_src_task_get(&(ancsc_env->prf_env) ,conidx),
                                                ancsc_enable_rsp);
    rsp->status = status;
    if (status == GAP_ERR_NO_ERROR)
    {
        rsp->ancs = ancsc_env->env[conidx]->ancs;
        // Register ANCSC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(ancsc_env->prf_env), conidx, &ancsc_env->env[conidx]->ancs.svc);
    }

    ke_msg_send(rsp);
}

void ancsc_encrypted_cmd(uint8_t conidx)
{
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
    ASSERT_ERR(ancsc_env != NULL);
	ke_msg_send_basic(ANCSC_ENCRYPTED_CMD,
	    prf_src_task_get(&(ancsc_env->prf_env) ,conidx),
	    prf_dst_task_get(&(ancsc_env->prf_env) ,conidx)
	);
}


/// @} ANCSC
