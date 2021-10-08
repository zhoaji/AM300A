/**
 ****************************************************************************************
 *
 * @file tspps.c
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
 * @addtogroup TSPPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_TSPP_SERVER)
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "gapc_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"
#include "co.h"
#include "tspps.h"
#include "tspps_task.h"

/*
 * TSPP PROFILE ATTRIBUTES
 ****************************************************************************************
 */

/// Full TSPP Database Description - Used to add attributes into the database
enum
{
    ATT_SERVICE_TSPP                        = ATT_UUID_16(0xFF01),
    ATT_CHAR_TSPP_REV1                      = ATT_UUID_16(0xFF02),
    ATT_CHAR_TSPP_UPLOAD                    = ATT_UUID_16(0xFF03),
    ATT_CHAR_TSPP_REV2                      = ATT_UUID_16(0xFF04),
};

const struct attm_desc tspps_att_db[TSPPS_IDX_NB] =
{
    // Service Declaration
    [TSPPS_IDX_SVC]              =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // rev1 Characteristic Declaration
    [TSPPS_IDX_REV1_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // rev1 Characteristic Value
    [TSPPS_IDX_REV1_VAL]         =   {ATT_CHAR_TSPP_REV1, PERM(WRITE_COMMAND, ENABLE), 0, TSPPS_UPLOAD_MAX_LEN},

    // upload Characteristic Declaration
    [TSPPS_IDX_UPLOAD_CHAR]      =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // upload Characteristic Value
    [TSPPS_IDX_UPLOAD_VAL]       =   {ATT_CHAR_TSPP_UPLOAD, PERM(NTF, ENABLE), PERM(RI, ENABLE), TSPPS_UPLOAD_MAX_LEN},
    // upload Characteristic - Client Characteristic Configuration Descriptor
    [TSPPS_IDX_UPLOAD_NTF_CFG]   =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // rev2 Characteristic Declaration
    [TSPPS_IDX_REV2_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // rev2 Characteristic Value
    [TSPPS_IDX_REV2_VAL]         =   {ATT_CHAR_TSPP_REV2, PERM(WRITE_REQ, ENABLE), 0, TSPPS_UPLOAD_MAX_LEN},
};

static const uint8_t tspp_service_uuid128[16] = {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0xFF, 0x40, 0x6E};
#define TSPPS_IDX_REV1_VAL_128   {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0xFF, 0x40, 0x6E}
#define TSPPS_IDX_UPLOAD_VAL_128 {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0xFF, 0x40, 0x6E}
#define TSPPS_IDX_REV2_VAL_128   {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x04, 0xFF, 0x40, 0x6E}

const struct attm_desc_128 tspps_att_db_128[TSPPS_IDX_NB] =
{
    // Service Declaration
    [TSPPS_IDX_SVC]              =   {ATT_16_TO_128_ARRAY(ATT_DECL_PRIMARY_SERVICE), PERM(RD, ENABLE), 0, 0},
    
    // rev1 Characteristic Declaration
    [TSPPS_IDX_REV1_CHAR]        =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    // rev1 Characteristic Value
    [TSPPS_IDX_REV1_VAL]         =   {TSPPS_IDX_REV1_VAL_128, PERM(WRITE_COMMAND, ENABLE), PERM_VAL(UUID_LEN, PERM_UUID_128), TSPPS_UPLOAD_MAX_LEN},

    // upload Characteristic Declaration
    [TSPPS_IDX_UPLOAD_CHAR]      =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    // upload Characteristic Value
    [TSPPS_IDX_UPLOAD_VAL]       =   {TSPPS_IDX_UPLOAD_VAL_128, PERM(NTF, ENABLE), PERM(RI, ENABLE)|PERM_VAL(UUID_LEN, PERM_UUID_128), TSPPS_UPLOAD_MAX_LEN},
    // upload Characteristic - Client Characteristic Configuration Descriptor
    [TSPPS_IDX_UPLOAD_NTF_CFG]   =   {ATT_16_TO_128_ARRAY(ATT_DESC_CLIENT_CHAR_CFG), PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // rev2 Characteristic Declaration
    [TSPPS_IDX_REV2_CHAR]        =   {ATT_16_TO_128_ARRAY(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    // rev2 Characteristic Value
    [TSPPS_IDX_REV2_VAL]         =   {TSPPS_IDX_REV2_VAL_128, PERM(WRITE_REQ, ENABLE), PERM_VAL(UUID_LEN, PERM_UUID_128), TSPPS_UPLOAD_MAX_LEN},
};

/**
 ****************************************************************************************
 * @brief Initialization of the TSPPS module.
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
static uint8_t tspps_init(struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl, struct tspp_server_db_cfg* params)
{
    //------------------ create the attribute database for the profile -------------------
    //Database Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;
    uint16_t buffer_size=0;
    uint16_t max_count=TSPPS_IDX_MAX;

    struct attm_desc_128 temp_128_db[TSPPS_IDX_NB];

    if (params == NULL || params->fifo_size <=0)// || params->fifo_size >10*1024)
    {
        buffer_size = 2048;
    }
    else
    {
        buffer_size = params->fifo_size;
    }
    if (params->connect_num < TSPPS_IDX_MAX)
    {
        max_count = params->connect_num;
    }

    //Add Service Into Database
    if (params != NULL && params->svc_type == 1)
    {
        status = attm_svc_create_db_128(start_hdl, tspp_service_uuid128, NULL,
                TSPPS_IDX_NB, NULL, env->task, &tspps_att_db_128[0],
                (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE) | PERM(SVC_UUID_LEN, UUID_128)
                );
    }
    else if (params != NULL && params->svc_type == 2)
    {
        uint8_t service_type = 0;
        memcpy(temp_128_db, tspps_att_db_128, sizeof(struct attm_desc_128)*TSPPS_IDX_NB);
        memcpy(temp_128_db[TSPPS_IDX_REV1_VAL].uuid,params->svc.write_no_res.uuid, params->svc.write_no_res.uuid_len);
        if (params->svc.write_no_res.uuid_len == ATT_UUID_128_LEN)
        {
            temp_128_db[TSPPS_IDX_REV1_VAL].ext_perm = PERM_VAL(UUID_LEN, PERM_UUID_128);
        }
        else if (params->svc.write_no_res.uuid_len == ATT_UUID_32_LEN)
        {
            temp_128_db[TSPPS_IDX_REV1_VAL].ext_perm = PERM_VAL(UUID_LEN, PERM_UUID_32);
        }
        else
        {
            temp_128_db[TSPPS_IDX_REV1_VAL].ext_perm = PERM_VAL(UUID_LEN, PERM_UUID_16);
        }

        memcpy(temp_128_db[TSPPS_IDX_UPLOAD_VAL].uuid,params->svc.notify.uuid, params->svc.notify.uuid_len);
        if (params->svc.notify.uuid_len == ATT_UUID_128_LEN)
        {
            temp_128_db[TSPPS_IDX_UPLOAD_VAL].ext_perm = PERM(RI, ENABLE)|PERM_VAL(UUID_LEN, PERM_UUID_128);
        }
        else if (params->svc.notify.uuid_len == ATT_UUID_32_LEN)
        {
            temp_128_db[TSPPS_IDX_UPLOAD_VAL].ext_perm = PERM(RI, ENABLE)|PERM_VAL(UUID_LEN, PERM_UUID_32);
        }
        else
        {
            temp_128_db[TSPPS_IDX_UPLOAD_VAL].ext_perm = PERM(RI, ENABLE)|PERM_VAL(UUID_LEN, PERM_UUID_16);
        }

        memcpy(temp_128_db[TSPPS_IDX_REV2_VAL].uuid,params->svc.write.uuid, params->svc.write.uuid_len);
        if (params->svc.write.uuid_len == ATT_UUID_128_LEN)
        {
            temp_128_db[TSPPS_IDX_REV2_VAL].ext_perm = PERM_VAL(UUID_LEN, PERM_UUID_128);
        }
        else if (params->svc.write.uuid_len == ATT_UUID_32_LEN)
        {
            temp_128_db[TSPPS_IDX_REV2_VAL].ext_perm = PERM_VAL(UUID_LEN, PERM_UUID_32);
        }
        else
        {
            temp_128_db[TSPPS_IDX_REV2_VAL].ext_perm = PERM_VAL(UUID_LEN, PERM_UUID_16);
        }

        if (params->svc.uuid_len == ATT_UUID_128_LEN)
        {
            service_type = PERM(SVC_UUID_LEN, UUID_128);
        }
        else if (params->svc.write.uuid_len == ATT_UUID_32_LEN)
        {
            service_type = PERM(SVC_UUID_LEN, UUID_32);
        }
        else
        {
            service_type = PERM(SVC_UUID_LEN, UUID_16);
        }

        status = attm_svc_create_db_128(start_hdl, params->svc.uuid, NULL,
                TSPPS_IDX_NB, NULL, env->task, &temp_128_db[0],
                (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE) | service_type
                );
    }
    else {
        status = attm_svc_create_db(start_hdl, ATT_SERVICE_TSPP, NULL,
                TSPPS_IDX_NB, NULL, env->task, &tspps_att_db[0],
                (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS))| PERM(SVC_MI, DISABLE));
    }

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate TSPPS required environment variable
        struct tspps_env_tag* tspps_env =
                (struct tspps_env_tag* ) ke_malloc((sizeof(struct tspps_env_tag)), KE_MEM_ATT_DB);

        // Initialize TSPPS environment
        env->env           = (prf_env_t*) tspps_env;
        tspps_env->start_hdl = *start_hdl;
        tspps_env->operation = NULL;

        tspps_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Mono Instantiated task
        tspps_env->prf_env.prf_task    = env->task | PERM(PRF_MI, DISABLE);
        //tspps_env->mtu = ATT_DEFAULT_MTU;

        tspps_env->send_pool_size = buffer_size;
        tspps_env->send_pool_num = max_count;

        for(uint8_t num = 0; num < max_count ; num++)
        {
            if(params->fifo_buffer)
            {
                tspps_env->send_pool[num] = params->fifo_buffer;
                tspps_env->send_pool_inner[num] = 0;
            }
            else
            {
                tspps_env->send_pool[num] = ke_malloc(tspps_env->send_pool_size, KE_MEM_ATT_DB);
                tspps_env->send_pool_inner[num] = 1;
            }
            co_fifo_init(&tspps_env->send_fifo[num], tspps_env->send_pool[num], tspps_env->send_pool_size);
        }
        
        // initialize environment variable
        env->id                     = TASK_ID_TSPPS;
        tspp_server_task_init(&(env->desc));

        /* Put HRS in Idle state */
        for(uint8_t idx = 0; idx < TSPPS_IDX_MAX ; idx++)
        {
            /* Put WSCS in disabled state */
            ke_state_set(KE_BUILD_ID(env->task, idx), TSPPS_IDLE);
        }
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the TSPPS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void tspps_destroy(struct prf_task_env* env)
{
    struct tspps_env_tag* tspps_env = (struct tspps_env_tag*) env->env;

    // clear on-going operation
    if(tspps_env->operation != NULL)
    {
        ke_free(tspps_env->operation);
    }

    for(uint8_t num = 0; num < tspps_env->send_pool_num; num++)
    {
        if(tspps_env->send_pool_inner[num])
            ke_free(tspps_env->send_pool[num]);
    }
    env->env = NULL;
    ke_free(tspps_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void tspps_create(struct prf_task_env* env, uint8_t conidx)
{
    struct tspps_env_tag* tspp_server_env = (struct tspps_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    tspp_server_env->ntf_cfg[conidx] = 0;
    tspp_server_env->mtu[conidx] = ATT_DEFAULT_MTU;
    tspp_server_env->tx_len[conidx] = BLE_MIN_OCTETS;
    tspp_server_env->perfect_once_tx_length[conidx] = ATT_DEFAULT_MTU-3;
    ke_state_set(KE_BUILD_ID(env->task, conidx), TSPPS_CONNECTED);
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
static void tspps_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct tspps_env_tag* tspp_server_env = (struct tspps_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    tspp_server_env->ntf_cfg[conidx] = 0;
    tspp_server_env->mtu[conidx] = ATT_DEFAULT_MTU;
    for(uint8_t num = 0; num < tspp_server_env->send_pool_num; num++)
    {
        if(tspp_server_env->send_pool[num] != NULL)
        {
            co_fifo_reset(&(tspp_server_env->send_fifo[num]));
        }
    }

    ke_state_set(KE_BUILD_ID(env->task, conidx), TSPPS_IDLE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// TSPPS Task interface required by profile manager
const struct prf_task_cbs tspps_itf =
{
        (prf_init_fnct) tspps_init,
        tspps_destroy,
        tspps_create,
        tspps_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* tspp_server_prf_itf_get(void)
{
   return &tspps_itf;
}


/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

uint16_t tspp_server_get_att_handle(uint8_t att_idx)
{
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);
    uint16_t handle = tspp_server_env->start_hdl + att_idx;
    return handle;
}

uint8_t tspp_server_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);
    *att_idx = handle - tspp_server_env->start_hdl;
    return ATT_ERR_NO_ERROR;
}

/**
 * @brief tspp_server_perfect_once_tx_length()
 *
 * @param[in] mtu  
 * @param[in] mto  
 * @param[in] char_len  
 *
 * @return 
 **/
uint16_t tspp_server_perfect_once_tx_length(uint16_t mtu, uint16_t mto, uint16_t char_len)
{
    uint16_t mtux = MIN(TSPPS_MTU_TO_NTF_WRTCMD_LEN(mtu), char_len);
    uint16_t mtox = TSPPS_MTO_TO_NTF_WRTCMD_LEN(mto);

    return (mtux > mtox) ? (mtox + mto * ((mtux-mtox) / mto)) : (mtux);
}

void tspp_server_set_max_mtu(uint8_t conidx, uint16_t mtu)
{
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);
    tspp_server_env->mtu[conidx] = mtu;
    tspp_server_env->perfect_once_tx_length[conidx] = tspp_server_perfect_once_tx_length(mtu, 
                                                                                         tspp_server_env->tx_len[conidx],
                                                                                         GAP_MAX_LE_MTU);
}

void tspp_server_set_data_len(uint8_t conidx, uint16_t tx_len)
{
    struct tspps_env_tag* tspp_server_env = PRF_ENV_GET(TSPPS, tspps);
    tspp_server_env->tx_len[conidx] = tx_len;
    tspp_server_env->perfect_once_tx_length[conidx] = tspp_server_perfect_once_tx_length(tspp_server_env->mtu[conidx], 
                                                                                        tspp_server_env->tx_len[conidx],
                                                                                        GAP_MAX_LE_MTU);
}
#endif /* BLE_TSPP_SERVER */

/// @} TSPPS
