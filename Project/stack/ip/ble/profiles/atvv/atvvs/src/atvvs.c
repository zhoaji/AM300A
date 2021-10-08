/**
 ****************************************************************************************
 *
 * @file atvvs.c
 *
 * @brief transport Profile implementation.
 *
 * Copyright (C) Huntersun 2018-2028
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup ATVVS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_ATVV_SERVER)
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "gapc_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"
#include "co.h"
#include "atvvs.h"
#include "atvvs_task.h"

/*
 * ATVV PROFILE ATTRIBUTES
 ****************************************************************************************
 */

/// Full ATVV Database Description - Used to add attributes into the database
enum
{
    ATT_SERVICE_ATVV                        = ATT_UUID_16(0x0001),
    ATT_CHAR_ATVV_WRITE                     = ATT_UUID_16(0x0002),
    ATT_CHAR_ATVV_READ                      = ATT_UUID_16(0x0003),
    ATT_CHAR_ATVV_CONTROL                   = ATT_UUID_16(0x0004),
};

const struct attm_desc atvvs_att_db[ATVVS_IDX_NB] =
{
    // Service Declaration
    [ATVVS_IDX_SVC]              =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // write Characteristic Declaration
    [ATVVS_IDX_WRITE_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // write Characteristic Value
    [ATVVS_IDX_WRITE_VAL]         =   {ATT_CHAR_ATVV_WRITE, PERM(WRITE_REQ, ENABLE), 0, ATVVS_UPLOAD_MAX_LEN},

    // read Characteristic Declaration
    [ATVVS_IDX_READ_CHAR]      =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // read Characteristic Value
    [ATVVS_IDX_READ_VAL]       =   {ATT_CHAR_ATVV_READ, PERM(RD, ENABLE) |PERM(NTF, ENABLE), PERM(RI, ENABLE), ATVVS_UPLOAD_MAX_LEN},
    // read Characteristic - Client Characteristic Configuration Descriptor
    [ATVVS_IDX_READ_NTF_CFG]   =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // control Characteristic Declaration
    [ATVVS_IDX_CONTROL_CHAR]      =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // control Characteristic Value
    [ATVVS_IDX_CONTROL_VAL]       =   {ATT_CHAR_ATVV_CONTROL, PERM(RD, ENABLE) |PERM(NTF, ENABLE), PERM(RI, ENABLE), ATVVS_UPLOAD_MAX_LEN},
    // control Characteristic - Client Characteristic Configuration Descriptor
    [ATVVS_IDX_CONTROL_NTF_CFG]   =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

};

static const uint8_t atvv_service_uuid128[16] = {0x64, 0xB6, 0x17, 0xF6, 0x01, 0xAF, 0x7D, 0xBC, 0x05, 0x4F, 0x21, 0x5A, 0x01, 0x00, 0x5E, 0xAB};
#define ATVVS_IDX_WRITE_VAL_128         {0x64, 0xB6, 0x17, 0xF6, 0x01, 0xAF, 0x7D, 0xBC, 0x05, 0x4F, 0x21, 0x5A, 0x02, 0x00, 0x5E, 0xAB}
#define ATVVS_IDX_READ_VAL_128          {0x64, 0xB6, 0x17, 0xF6, 0x01, 0xAF, 0x7D, 0xBC, 0x05, 0x4F, 0x21, 0x5A, 0x03, 0x00, 0x5E, 0xAB}
#define ATVVS_IDX_CONTROL_VAL_128       {0x64, 0xB6, 0x17, 0xF6, 0x01, 0xAF, 0x7D, 0xBC, 0x05, 0x4F, 0x21, 0x5A, 0x04, 0x00, 0x5E, 0xAB}

const struct attm_desc_128 atvvs_att_db_128[ATVVS_IDX_NB] =
{
    // Service Declaration
    [ATVVS_IDX_SVC]              =   {ATT_16_TO_128_ARRAY(ATT_DECL_PRIMARY_SERVICE), PERM(RD, ENABLE), 0, 0},
    
    // write Characteristic Declaration
    [ATVVS_IDX_WRITE_CHAR]        =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    // write Characteristic Value
    [ATVVS_IDX_WRITE_VAL]         =   {ATVVS_IDX_WRITE_VAL_128, PERM(WRITE_REQ, ENABLE), PERM_VAL(UUID_LEN, PERM_UUID_128), ATVVS_UPLOAD_MAX_LEN},

    // read Characteristic Declaration
    [ATVVS_IDX_READ_CHAR]      =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    // read Characteristic Value
    [ATVVS_IDX_READ_VAL]       =   {ATVVS_IDX_READ_VAL_128, PERM(NTF, ENABLE), PERM(RI, ENABLE)|PERM_VAL(UUID_LEN, PERM_UUID_128), ATVVS_UPLOAD_MAX_LEN},
    // read Characteristic - Client Characteristic Configuration Descriptor
    [ATVVS_IDX_READ_NTF_CFG]   =   {ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG), PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    
    [ATVVS_IDX_CONTROL_CHAR]      =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    // read Characteristic Value
    [ATVVS_IDX_CONTROL_VAL]       =   {ATVVS_IDX_CONTROL_VAL_128, PERM(NTF, ENABLE), PERM(RI, ENABLE)|PERM_VAL(UUID_LEN, PERM_UUID_128), ATVVS_UPLOAD_MAX_LEN},
    // read Characteristic - Client Characteristic Configuration Descriptor
    [ATVVS_IDX_CONTROL_NTF_CFG]   =   {ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG), PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
};

/**
 ****************************************************************************************
 * @brief Initialization of the ATVVS module.
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
static uint8_t atvvs_init(struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl, struct atvv_server_db_cfg* params)
{
    //------------------ create the attribute database for the profile -------------------
    //Database Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;
    uint16_t buffer_size=0;
    uint16_t max_count=ATVVS_IDX_MAX;

    if (params == NULL || params->fifo_size <=0)// || params->fifo_size >10*1024)
    {
        buffer_size = 2048;
    }
    else
    {
        buffer_size = params->fifo_size;
    }
    if (params->connect_num < ATVVS_IDX_MAX)
    {
        max_count = params->connect_num;
    }

    //Add Service Into Database
    if (params != NULL && params->svc_type == 1)
    {
        status = attm_svc_create_db_128(start_hdl, atvv_service_uuid128, NULL,
                ATVVS_IDX_NB, NULL, env->task, &atvvs_att_db_128[0],
                (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE) | PERM(SVC_UUID_LEN, UUID_128)
                );
    }
    else {
        status = attm_svc_create_db(start_hdl, ATT_SERVICE_ATVV, NULL,
                ATVVS_IDX_NB, NULL, env->task, &atvvs_att_db[0],
                (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS))| PERM(SVC_MI, DISABLE));
    }

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate atvvS required environment variable
        struct atvvs_env_tag* atvvs_env =
                (struct atvvs_env_tag* ) ke_malloc((sizeof(struct atvvs_env_tag)), KE_MEM_ATT_DB);

        // Initialize atvvS environment
        env->env           = (prf_env_t*) atvvs_env;
        atvvs_env->start_hdl = *start_hdl;
        atvvs_env->operation = NULL;
        atvvs_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Mono Instantiated task
        atvvs_env->prf_env.prf_task    = env->task | PERM(PRF_MI, DISABLE);
        //atvvs_env->mtu = ATT_DEFAULT_MTU;

        atvvs_env->send_pool_size = buffer_size;
        atvvs_env->send_pool_num = max_count;

        for(uint8_t num = 0; num < max_count ; num++)
        {
            if(params->fifo_buffer != NULL)
            {
               atvvs_env->send_pool[num] = params->fifo_buffer;
               atvvs_env->send_pool_inner[num] = 0;
            }
            else
            {
                atvvs_env->send_pool[num] = ke_malloc(atvvs_env->send_pool_size, KE_MEM_ENV);
                atvvs_env->send_pool_inner[num] = 1;
            }
            co_fifo_init(&atvvs_env->send_fifo[num], atvvs_env->send_pool[num], atvvs_env->send_pool_size);
        }
        
        // initialize environment variable
        env->id                     = TASK_ID_ATVVS;
        atvv_server_task_init(&(env->desc));

        /* Put HRS in Idle state */
        for(uint8_t idx = 0; idx < ATVVS_IDX_MAX ; idx++)
        {
            /* Put WSCS in disabled state */
            ke_state_set(KE_BUILD_ID(env->task, idx), ATVVS_IDLE);
        }
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the ATVVS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void atvvs_destroy(struct prf_task_env* env)
{
    struct atvvs_env_tag* atvvs_env = (struct atvvs_env_tag*) env->env;

    // clear on-going operation
    if(atvvs_env->operation != NULL)
    {
        ke_free(atvvs_env->operation);
    }

    for(uint8_t num = 0; num < atvvs_env->send_pool_num; num++)
    {
        if(atvvs_env->send_pool_inner[num]) {
            ke_free(atvvs_env->send_pool[num]);
        }
    }
    env->env = NULL;
    ke_free(atvvs_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void atvvs_create(struct prf_task_env* env, uint8_t conidx)
{
    struct atvvs_env_tag* atvv_server_env = (struct atvvs_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    //memset(atvv_server_env, 0, sizeof(struct atvvs_env_tag));
    atvv_server_env->ntf_cfg[conidx][0] = 0;
    atvv_server_env->ntf_cfg[conidx][1] = 0;
    atvv_server_env->mtu[conidx] = ATT_DEFAULT_MTU;
    atvv_server_env->tx_len[conidx] = BLE_MIN_OCTETS;
    atvv_server_env->perfect_once_tx_length[conidx] = ATT_DEFAULT_MTU-3;
    ke_state_set(KE_BUILD_ID(env->task, conidx), ATVVS_CONNECTED);
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
static void atvvs_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct atvvs_env_tag* atvv_server_env = (struct atvvs_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    //atvv_server_env->ntf_cfg[conidx] = 0
    //memset(atvv_server_env, 0, sizeof(struct atvvs_env_tag));
    atvv_server_env->ntf_cfg[conidx][0] = 0;
    atvv_server_env->ntf_cfg[conidx][1] = 0;
    atvv_server_env->mtu[conidx] = ATT_DEFAULT_MTU;
    for(uint8_t num = 0; num < atvv_server_env->send_pool_num; num++)
    {
        if(atvv_server_env->send_pool[num] != NULL)
        {
            co_fifo_reset(&(atvv_server_env->send_fifo[num]));
        }
    }

    ke_state_set(KE_BUILD_ID(env->task, conidx), ATVVS_IDLE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// ATVVS Task interface required by profile manager
const struct prf_task_cbs atvvs_itf =
{
        (prf_init_fnct) atvvs_init,
        atvvs_destroy,
        atvvs_create,
        atvvs_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* atvv_server_prf_itf_get(void)
{
   return &atvvs_itf;
}


/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

uint16_t atvv_server_get_att_handle(uint8_t att_idx)
{
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    uint16_t handle = atvv_server_env->start_hdl + att_idx;
    return handle;
}

uint8_t atvv_server_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    *att_idx = handle - atvv_server_env->start_hdl;
    return ATT_ERR_NO_ERROR;
}

/**atvv
 * @brief atvv_server_perfect_once_tx_length()
 *
 * @param[in] mtu  
 * @param[in] mto  
 * @param[in] char_len  
 *
 * @return 
 **/
uint16_t atvv_server_perfect_once_tx_length(uint16_t mtu, uint16_t mto, uint16_t char_len)
{
    uint16_t mtux = MIN(ATVVS_MTU_TO_NTF_WRTCMD_LEN(mtu), char_len);
    uint16_t mtox = ATVVS_MTO_TO_NTF_WRTCMD_LEN(mto);

    return (mtux > mtox) ? (mtox + mto * ((mtux-mtox) / mto)) : (mtux);
}

void atvv_server_set_max_mtu(uint8_t conidx, uint16_t mtu)
{
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    atvv_server_env->mtu[conidx] = mtu;
    atvv_server_env->perfect_once_tx_length[conidx] = atvv_server_perfect_once_tx_length(mtu, 
                                                                                 atvv_server_env->tx_len[conidx],
                                                                                 GAP_MAX_LE_MTU);
    }

void atvv_server_set_data_len(uint8_t conidx, uint16_t tx_len)
{
    struct atvvs_env_tag* atvv_server_env = PRF_ENV_GET(ATVVS, atvvs);
    atvv_server_env->tx_len[conidx] = tx_len;
    atvv_server_env->perfect_once_tx_length[conidx] = atvv_server_perfect_once_tx_length(atvv_server_env->mtu[conidx], 
                                                                                atvv_server_env->tx_len[conidx],
                                                                                GAP_MAX_LE_MTU);
}
#endif /* BLE_ATVV_SERVER */

/// @} atvvS
