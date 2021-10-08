/**
 ****************************************************************************************
 *
 * @file app.c
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_SIMPLE_SERVER_APP_C app.c
 * @ingroup APP_SIMPLE_SERVER
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)

#include <string.h>

#include "app_task.h"                // Application task Definition
#include "app.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition

#if (BLE_APP_SEC)
#include "app_sec.h"                 // Application security Definition
#endif // (BLE_APP_SEC)

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)

#include "app_simple_server.h"
#include "simple_server.h"

#if (BLE_APP_DIS)
#include "app_dis.h"                 // Device Information Service Application Definitions
#include "diss.h"
#endif
/*
 * DEFINES
 ****************************************************************************************
 */

/// Default Device Name
#define APP_DFLT_DEVICE_NAME            ("AM300B:00002")
/// Default Device Name Length
#define APP_DFLT_DEVICE_NAME_LEN        (sizeof(APP_DFLT_DEVICE_NAME)-1)

/// uuid in adv
#define APP_ADV_DATA_UUID        "\x03\x03\xB0\xFF"  //"\x03\x03\x12\x18" //
/// uuid len
#define APP_ADV_DATA_UUID_LEN   (4)

/// appearance in adv
#define APP_ADV_DATA_APPEARANCE   "\x03\x19\xC2\x03"
/// appearance len
#define APP_ADV_DATA_APPEARANCE_LEN  (4)

/// scan resposne data
#define APP_SCNRSP_DATA         ""
/// scan resposne data len
#define APP_SCNRSP_DATA_LEN     (0)

/**
 * Advertising Parameters
 */																// ¹ã²¥¼ä¸ô		
#define APP_ADV_FAST_INTERVAL     0x0028   /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL     0x0C80   /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */

#define APP_ADV_FAST_TIMEOUT      0       /**< The duration of the fast advertising period (in seconds). max 655, or 0 is no timeout*/
#define APP_ADV_SLOW_TIMEOUT      0       /**< The duration of the slow advertising period (in seconds). max 655. or 0 is no timeout*/

#define APP_ADV_DIRECTED_TIMEOUT  0       /**< The duration of the fast advertising period (in seconds). mas 655. or 0 is high duty cycle*/

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// typdef appm_add_svc_func_t
typedef void (*appm_add_svc_func_t)(void);

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of service to add in the database
enum appm_svc_list
{
    APPM_SVC_SIMPLE_SERVER,
    #if (BLE_APP_DIS)
    APPM_SVC_DIS,
    #endif //(BLE_APP_DIS)
    APPM_SVC_LIST_STOP,
};

/*
 * LOCAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */

/// Application Task Descriptor
extern const struct ke_task_desc TASK_DESC_APP;

/// List of functions used to create the database
static const appm_add_svc_func_t appm_add_svc_func_list[APPM_SVC_LIST_STOP] =
{
    (appm_add_svc_func_t)app_simple_server_add_service,
    #if (BLE_APP_DIS)
    (appm_add_svc_func_t)app_dis_add_dis,
    #endif //(BLE_APP_DIS)
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Environment Structure
struct app_env_tag app_env;

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
static void appm_build_adv_data(uint16_t max_length, uint16_t *p_length, uint8_t *p_buf)
{
    // Remaining Length
    uint8_t rem_len = max_length;
	
/* Remove 20210607   // Set appearance
    memcpy(p_buf, APP_ADV_DATA_APPEARANCE, APP_ADV_DATA_APPEARANCE_LEN);
    *p_length += APP_ADV_DATA_APPEARANCE_LEN;
    p_buf += APP_ADV_DATA_APPEARANCE_LEN;
*/
    // Set list of UUIDs
    memcpy(p_buf, APP_ADV_DATA_UUID, APP_ADV_DATA_UUID_LEN);
    *p_length += APP_ADV_DATA_UUID_LEN;
    p_buf += APP_ADV_DATA_UUID_LEN;

    // Sanity check
    ASSERT_ERR(rem_len >= max_length);

    // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
    rem_len -= *p_length;

    // Check if additional data can be added to the Advertising data - 2 bytes needed for type and length
    if (rem_len > 2)
    {
        uint8_t dev_name_length = co_min(app_env.dev_name_len, (rem_len - 2));

        // Device name length
        *p_buf = dev_name_length + 1;
        // Device name flag (check if device name is complete or not)
        *(p_buf + 1) = (dev_name_length == app_env.dev_name_len) ? '\x08' : '\x09';
        // Copy device name
        memcpy(p_buf + 2, app_env.dev_name, dev_name_length);

        // Update advertising data length
        *p_length += (dev_name_length + 2);
    }
}

static void appm_advertising_init(void)
{
    struct app_adv_modes_config_tag adv_config;

    adv_config.whitelist_enabled = false;
    adv_config.directed_enabled = false;
    adv_config.directed_timeout = APP_ADV_DIRECTED_TIMEOUT;
    adv_config.fast_enabled = true;
    adv_config.fast_interval = APP_ADV_FAST_INTERVAL;
    adv_config.fast_timeout = APP_ADV_FAST_TIMEOUT;
    adv_config.slow_enabled = false;
    adv_config.slow_interval = APP_ADV_SLOW_INTERVAL;
    adv_config.slow_timeout = APP_ADV_SLOW_TIMEOUT;

    appm_adv_init(NULL, &adv_config);

    uint8_t default_adv_data[ADV_DATA_LEN - 3];
    uint16_t adv_len = 0;
    appm_build_adv_data(ADV_DATA_LEN - 3, &adv_len, (uint8_t*)default_adv_data);
    appm_adv_set_adv_data((uint8_t*)default_adv_data, adv_len);

    //uint8_t res_data[APP_ADV_MAX_LEN];
    //uint8_t res_data_len = APP_SCNRSP_DATA_LEN;
    //memcpy(res_data, APP_SCNRSP_DATA, sizeof(APP_SCNRSP_DATA));
    //appm_adv_set_res_data(res_data, res_data_len);
}

static void appm_send_gapm_reset_cmd(void)
{
    // Reset the stack
    struct gapm_reset_cmd *p_cmd = KE_MSG_ALLOC(GAPM_RESET_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_reset_cmd);

    p_cmd->operation = GAPM_RESET;

    ke_msg_send(p_cmd);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void appm_init(void)
{
    // Reset the application manager environment
    memset(&app_env, 0, sizeof(app_env));

    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    // Initialize Task state
    ke_state_set(TASK_APP, APPM_INIT);

    // Get the Device Name to add in the Advertising Data (Default one or NVDS one)
    #if (NVDS_SUPPORT)
    app_env.dev_name_len = APP_DEVICE_NAME_MAX_LEN;
    if (nvds_get(NVDS_TAG_DEVICE_NAME, &(app_env.dev_name_len), app_env.dev_name) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        // Get default Device Name (No name if not enough space)
        memcpy(app_env.dev_name, APP_DFLT_DEVICE_NAME, APP_DFLT_DEVICE_NAME_LEN);
        app_env.dev_name_len = APP_DFLT_DEVICE_NAME_LEN;

        // TODO update this value per profiles
    }


    /*------------------------------------------------------
     * INITIALIZE ALL MODULES
     *------------------------------------------------------*/

    // load device information:

    #if (BLE_APP_SEC)
    // Security Module
    app_sec_init();
    #endif // (BLE_APP_SEC)

    #if (BLE_APP_DIS)
    // Device Information Module
    app_dis_init();
    #endif //(BLE_APP_DIS)

    #if (BLE_APP_SIMPLE_SERVER)
    // simple_server Module
    app_simple_server_init();
    #endif //(BLE_APP_SIMPLE_SERVER)

    // Reset the stack
    appm_send_gapm_reset_cmd();

    // start adversting
    appm_advertising_init();
}

void appm_reg_svc_itf(void)
{
    struct prf_itf_pair itf_pair[] = {
        #if BLE_APP_SIMPLE_SERVER
        { TASK_ID_SIMPLE_SERVER, simple_server_prf_itf_get() },
        #endif
        #if BLE_APP_WECHAT
        { TASK_ID_WECHAT, wechat_prf_itf_get() },
        #endif // BLE_APP_WECHAT
        #if (BLE_APP_DIS)
        { TASK_ID_DISS, diss_prf_itf_get() },
        #endif //(BLE_APP_DIS)
        #if (BLE_APP_BATT)
        { TASK_ID_BASS, bass_prf_itf_get() },
        #endif //(BLE_APP_BATT)
        #if (BLE_APP_HID)
        { TASK_ID_HOGPD, hogpd_prf_itf_get() },
        #endif //(BLE_APP_HID)
        #if (BLE_TSPP_SERVER)
        { TASK_ID_TSPPS, tspp_server_prf_itf_get() },
        #endif //(BLE_TSPP_SERVER)
        #if (BLE_APP_ANCSC)
        { TASK_ID_ANCSC, ancsc_prf_itf_get() },
        #endif //(BLE_APP_ANCSC)
        #if (BLE_APP_NORDIC_DFU)
        { TASK_ID_NORDIC_DFU, nordic_dfu_prf_itf_get() },
        #endif //(BLE_APP_NORDIC_DFU)
    };
    prf_itf_register(itf_pair, sizeof(itf_pair)/sizeof(itf_pair[0]));
}

bool appm_add_svc(void)
{
    // Indicate if more services need to be added in the database
    bool more_svc = false;

    // Check if another should be added in the database
    if (app_env.next_svc != APPM_SVC_LIST_STOP)
    {
        ASSERT_INFO(appm_add_svc_func_list[app_env.next_svc] != NULL, app_env.next_svc, 1);

        // Call the function used to add the required service
        appm_add_svc_func_list[app_env.next_svc]();

        // Select following service to add
        app_env.next_svc++;
        more_svc = true;

        // Go to the create db state
        ke_state_set(TASK_APP, APPM_CREATE_DB);
    }

    return more_svc;
}

void appm_disconnect(uint8_t conidx)
{
    struct gapc_disconnect_cmd *cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                                   KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                   gapc_disconnect_cmd);

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;

    // Send the message
    ke_msg_send(cmd);
}

void appm_update_param(struct gapc_conn_param *conn_param)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_param_update_cmd);

    cmd->operation  = GAPC_UPDATE_PARAMS;
    cmd->intv_min   = conn_param->intv_min;
    cmd->intv_max   = conn_param->intv_max;
    cmd->latency    = conn_param->latency;
    cmd->time_out   = conn_param->time_out;

    // not used by a slave device
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;

    // Send the message
    ke_msg_send(cmd);
}

uint8_t appm_get_dev_name(uint8_t* name)
{
    // copy name to provided pointer
    memcpy(name, app_env.dev_name, app_env.dev_name_len);
    // return name length
    return app_env.dev_name_len;
}

#endif //(BLE_APP_PRESENT)

/// @} APP
