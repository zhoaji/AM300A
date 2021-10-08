/**
 ****************************************************************************************
 *
 * @file amsc.c
 *
 * @brief AMS implementation.
 *
 * Copyright (C) 2020-2030 OnMicro Limited. All rights reserved.
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup AMSC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"


#include "gap.h"
#include "amsc.h"
#include "amsc_task.h"
#include "co_math.h"

#include "ke_mem.h"

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the AMSC module.
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
static uint8_t amsc_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  void* params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------
    struct amsc_env_tag* amsc_env =
            (struct amsc_env_tag* ) ke_malloc(sizeof(struct amsc_env_tag), KE_MEM_ATT_DB);

    // allocate AMSC required environment variable
    env->env = (prf_env_t*) amsc_env;

    amsc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    amsc_env->prf_env.prf_task = env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    env->id                     = TASK_ID_AMSC;
    amsc_task_init(&(env->desc));

    for(idx = 0; idx < AMSC_IDX_MAX ; idx++)
    {
        amsc_env->env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(env->task, idx), AMSC_ST_FREE);
    }


    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Destruction of the AMSC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void amsc_destroy(struct prf_task_env* env)
{
    uint8_t idx;
    struct amsc_env_tag* amsc_env = (struct amsc_env_tag*) env->env;

    // cleanup environment variable for each task instances
    for(idx = 0; idx < AMSC_IDX_MAX ; idx++)
    {
        if(amsc_env->env[idx] != NULL)
        {
            ke_free(amsc_env->env[idx]);
        }
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(amsc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void amsc_create(struct prf_task_env* env, uint8_t conidx)
{
    /* Put AMS Client in Idle state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), AMSC_ST_IDLE);
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
static void amsc_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct amsc_env_tag* amsc_env = (struct amsc_env_tag*) env->env;

    // clean-up environment variable allocated for task instance
    if(amsc_env->env[conidx] != NULL)
    {
        ke_free(amsc_env->env[conidx]);
        amsc_env->env[conidx] = NULL;
    }

    /* Put AMS Client in Free state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), AMSC_ST_FREE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// AMSC Task interface required by profile manager
const struct prf_task_cbs amsc_itf =
{
        amsc_init,
        amsc_destroy,
        amsc_create,
        amsc_cleanup,
};


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* amsc_prf_itf_get(void)
{
   return &amsc_itf;
}

void amsc_enable_rsp_send(struct amsc_env_tag *amsc_env, uint8_t conidx, uint8_t status)
{
    // Send APP the details of the discovered attributes on AMSC
    struct amsc_enable_rsp * rsp = KE_MSG_ALLOC(AMSC_ENABLE_RSP,
                                                prf_dst_task_get(&(amsc_env->prf_env) ,conidx),
                                                prf_src_task_get(&(amsc_env->prf_env) ,conidx),
                                                amsc_enable_rsp);
    rsp->status = status;
    if (status == GAP_ERR_NO_ERROR)
    {
        rsp->ams = amsc_env->env[conidx]->ams;
        // Register AMSC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(amsc_env->prf_env), conidx, &amsc_env->env[conidx]->ams.svc);
    }

    ke_msg_send(rsp);
}

void amsc_encrypted_cmd(uint8_t conidx)
{
    struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);
    ASSERT_ERR(amsc_env != NULL);
	ke_msg_send_basic(AMSC_ENCRYPTED_CMD,
	    prf_src_task_get(&(amsc_env->prf_env) ,conidx),
	    prf_dst_task_get(&(amsc_env->prf_env) ,conidx)
	);
}

void amsc_remote_ctrl_cmd(uint8_t conidx, enum ams_RemoteCommandID_values value)
{
    struct amsc_env_tag *amsc_env = PRF_ENV_GET(AMSC, amsc);
    ASSERT_ERR(amsc_env != NULL);
    struct amsc_rem_ctrl_cmd * cmd = KE_MSG_ALLOC(AMSC_REM_CTRL_CMD,
                                                prf_src_task_get(&(amsc_env->prf_env) ,conidx),
                                                prf_dst_task_get(&(amsc_env->prf_env) ,conidx),
                                                amsc_rem_ctrl_cmd);
    cmd->value = value;
    ke_msg_send(cmd);
}


/// @} AMSC
