/**
 ****************************************************************************************
 *
 * @file app_adv.c
 * @brief Advertising data create and send.
 * @date Mon, Jan  7, 2019  4:31:27 PM
 * @author chenzhiyuan
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_COMMON_ADV_C app_adv.c
 * @ingroup APP_COMMON
 *
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
#include "gapm_task.h"               // GAP Manager Task API
#include "app_task.h"                // Application task Definition
#include "app.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapc_task.h"               // GAP Controller Task API

#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "co_debug.h"
//#if (BLE_APP_SEC)
#include "app_sec.h"                 // Application security Definition
//#endif // (BLE_APP_SEC)
#include "app_adv.h"
#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)

/*
 * DEFINES
 ****************************************************************************************
 */
/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           (0x07)
/// advertising direct interval
#define APP_ADV_DIRECT_INTERVAL 0x20 //must be > 0x20
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// adv next event
enum app_adv_next_event
{
    /// Advertising activity start
    APP_ADV_NEXT_EVENT_START = (1<<0),
    /// Advertising activity stop
    APP_ADV_NEXT_EVENT_STOP  = (1<<1),
    /// Advertising activity delete
    APP_ADV_NEXT_EVENT_DEL   = (1<<2),
};

/**@struct app_adv_env_tag
 * @brief the advertising env information */
struct app_adv_env_tag
{
    /// Advertising data len
    uint8_t   adv_data_len;
    /// Advertising data
    uint8_t   adv_data[APP_ADV_MAX_LEN];
    /// response data len
    uint8_t   res_data_len;
    /// response data
    uint8_t   res_data[APP_ADV_MAX_LEN];
    /// Advertising config
    struct    app_adv_modes_config_tag config;

    /// Advertising activity index
    uint8_t actv_idx;
    /// Current advertising state (@see enum app_adv_state)
    uint8_t state;
    /// Next expected operation completed event
    uint8_t op;
    /// the current mode
    uint8_t cur_mode;
    /// the next mode
    uint8_t next_mode;
    /// enum @ref app_adv_next_event
    uint8_t next_event;
};
/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*
 * LOCAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */
static struct app_adv_env_tag app_adv_env; /// the adv env info
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static void appm_adv_build_adv_data(struct app_adv_data_tag* adv_data)
{
    uint8_t pos = 0, len = 0;

    // flags into gapm_adv
    // appearance
    if ((adv_data->include &APP_ADV_DATA_INCLUDE_APPEARANCE)==APP_ADV_DATA_INCLUDE_APPEARANCE) {
        app_adv_env.adv_data[pos++] = 3;
        app_adv_env.adv_data[pos++] = APP_ADV_DATATYPE_APPEARANCE;
        app_adv_env.adv_data[pos++] = (uint8_t) ((adv_data->appearance & 0x00FF) >> 0);
        app_adv_env.adv_data[pos++] = (uint8_t) ((adv_data->appearance & 0xFF00) >> 8);
    }

    // tx power level
    if ((adv_data->include &APP_ADV_DATA_INCLUDE_TX_POWER_LEVEL)==APP_ADV_DATA_INCLUDE_TX_POWER_LEVEL) {
        app_adv_env.adv_data[pos++] = 2;
        app_adv_env.adv_data[pos++] = APP_ADV_DATATYPE_TX_POWER_LEVEL;
        app_adv_env.adv_data[pos++] = (uint8_t) (adv_data->tx_power_level);
    }

    // uuid
    if ((adv_data->include &APP_ADV_DATA_INCLUDE_UUID)==APP_ADV_DATA_INCLUDE_UUID) {
        app_adv_env.adv_data[pos++] = adv_data->uuid.len+1;
        app_adv_env.adv_data[pos++] = adv_data->uuid.type;
        memcpy(&app_adv_env.adv_data[pos], adv_data->uuid.uuid, adv_data->uuid.len);
        pos += adv_data->uuid.len;
    }

    // name
    if ((adv_data->include &APP_ADV_DATA_INCLUDE_NAME)==APP_ADV_DATA_INCLUDE_NAME) {
        uint8_t name[APP_ADV_MAX_LEN];
        len = appm_get_dev_name(name);
        if (pos+2+len > APP_ADV_MAX_LEN)
        {
            len = APP_ADV_MAX_LEN - pos - 2;
        }
        if (len > 0) {
            app_adv_env.adv_data[pos++] = len + 1;
            app_adv_env.adv_data[pos++] = adv_data->name_type;
            memcpy(&app_adv_env.adv_data[pos], name, len);
        }
        pos += len;
    }
 
    // APP_ADV_DATATYPE_MANUFACTURER_SPECIFIC_DATA
    if ((adv_data->include &APP_ADV_DATA_INCLUDE_SPEC_DATA)==APP_ADV_DATA_INCLUDE_SPEC_DATA) {
        if (adv_data->manuf_specific_data.len + pos +2 > APP_ADV_MAX_LEN) {
            len = APP_ADV_MAX_LEN - pos - 2;
        }
        else {
            len = adv_data->manuf_specific_data.len;
        }
        if (len >0) {
            app_adv_env.adv_data[pos++] = len + 1;
            app_adv_env.adv_data[pos++] = APP_ADV_DATATYPE_MANUFACTURER_SPECIFIC_DATA;
            memcpy(&app_adv_env.adv_data[pos], adv_data->manuf_specific_data.data, len);
        }
        pos += len;
    }
    app_adv_env.adv_data_len = pos;
}

static void appm_adv_start_advertising(void)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD,
                                                         TASK_GAPM, TASK_APP,
                                                         gapm_activity_start_cmd);

    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = app_adv_env.actv_idx;
    switch(app_adv_env.cur_mode)
    {
        case APP_ADV_MODE_DIRECTED:
            p_cmd->u_param.adv_add_param.duration = app_adv_env.config.directed_timeout*100;
            break;
        case APP_ADV_MODE_FAST:
            p_cmd->u_param.adv_add_param.duration = app_adv_env.config.fast_timeout*100;
            break;
        case APP_ADV_MODE_SLOW:
            p_cmd->u_param.adv_add_param.duration = app_adv_env.config.slow_timeout*100;
            break;
        default:
            break;
    }
    //log_debug("timeout = %d\n", p_cmd->u_param.adv_add_param.duration );
    p_cmd->u_param.adv_add_param.max_adv_evt = 0;

    // Send the message
    ke_msg_send(p_cmd);

    // Keep the current operation
    app_adv_env.state = APP_ADV_STATE_STARTING;
    // And the next expected operation code for the command completed event
    app_adv_env.op = GAPM_START_ACTIVITY;
}


static void appm_adv_stop_advertising(void)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    // Prepare the GAPM_ACTIVITY_STOP_CMD message
    struct gapm_activity_stop_cmd *cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_STOP_CMD,
                                                      TASK_GAPM, TASK_APP,
                                                      gapm_activity_stop_cmd);

    // Fill the allocated kernel message
    cmd->operation = GAPM_STOP_ACTIVITY;
    cmd->actv_idx = app_adv_env.actv_idx;

    // Send the message
    ke_msg_send(cmd);

    // Update advertising state
    app_adv_env.state = APP_ADV_STATE_STOPPING;
    // And the next expected operation code for the command completed event
    app_adv_env.op = GAPM_STOP_ACTIVITY;
}


static void appm_adv_set_adv_data_cmd(void)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    // Prepare the GAPM_SET_ADV_DATA_CMD message
    struct gapm_set_adv_data_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_SET_ADV_DATA_CMD,
                                                           TASK_GAPM, TASK_APP,
                                                           gapm_set_adv_data_cmd,
                                                           app_adv_env.adv_data_len);

    // Fill the allocated kernel message
    p_cmd->operation = GAPM_SET_ADV_DATA;
    p_cmd->actv_idx = app_adv_env.actv_idx;

    p_cmd->length = app_adv_env.adv_data_len;
    // GAP will use 3 bytes for the AD Type
    memcpy(p_cmd->data, app_adv_env.adv_data, app_adv_env.adv_data_len);

    // Send the message
    ke_msg_send(p_cmd);

    // Update advertising state
    app_adv_env.state = APP_ADV_STATE_SETTING_ADV_DATA;
    // And the next expected operation code for the command completed event
    app_adv_env.op = GAPM_SET_ADV_DATA;
}

static void appm_adv_set_scan_rsp_data_cmd(void)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    // Prepare the GAPM_SET_ADV_DATA_CMD message
    struct gapm_set_adv_data_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_SET_ADV_DATA_CMD,
                                                           TASK_GAPM, TASK_APP,
                                                           gapm_set_adv_data_cmd,
                                                           ADV_DATA_LEN);

    // Fill the allocated kernel message
    p_cmd->operation = GAPM_SET_SCAN_RSP_DATA;
    p_cmd->actv_idx = app_adv_env.actv_idx;

    p_cmd->length = app_adv_env.res_data_len;
    memcpy(&p_cmd->data[0], app_adv_env.res_data, app_adv_env.res_data_len);

    // Send the message
    ke_msg_send(p_cmd);

    // Update advertising state
    app_adv_env.state = APP_ADV_STATE_SETTING_SCAN_RSP_DATA;
    // And the next expected operation code for the command completed event
    app_adv_env.op = GAPM_SET_SCAN_RSP_DATA;
}


static void appm_adv_create_advertising(void)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    if (app_adv_env.state == APP_ADV_STATE_IDLE)
    {
        // Prepare the GAPM_ACTIVITY_CREATE_CMD message
        struct gapm_activity_create_adv_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,
                                                                  TASK_GAPM, TASK_APP,
                                                                  gapm_activity_create_adv_cmd);

        // Set operation code
        p_cmd->operation = GAPM_CREATE_ADV_ACTIVITY;

        // Fill the allocated kernel message
        p_cmd->own_addr_type = GAPM_STATIC_ADDR;
#if (BLE_APP_PRIVACY)
        if (app_sec_get_bond_status()) {
            p_cmd->own_addr_type = GAPM_GEN_RSLV_ADDR;
        }
#endif
        p_cmd->adv_param.type = GAPM_ADV_TYPE_LEGACY;
        p_cmd->adv_param.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK;
        p_cmd->adv_param.filter_pol = ADV_ALLOW_SCAN_ANY_CON_ANY;
#if (BLE_APP_WHITE_LIST)
        if (app_sec_get_bond_status()) {
          p_cmd->adv_param.filter_pol = ADV_ALLOW_SCAN_WLST_CON_WLST;
        }
#endif
        p_cmd->adv_param.prim_cfg.chnl_map = APP_ADV_CHMAP;
        p_cmd->adv_param.prim_cfg.phy = GAP_PHY_LE_1MBPS;
        #ifdef BLE_APP_ADV_DISC_MODE
        p_cmd->adv_param.disc_mode = BLE_APP_ADV_DISC_MODE;
        #else
        p_cmd->adv_param.disc_mode = GAPM_ADV_MODE_GEN_DISC;
        #endif
        #ifdef BLE_APP_ADV_TX_POWER
        p_cmd->adv_param.max_tx_pwr = BLE_APP_ADV_TX_POWER;
        #else
        p_cmd->adv_param.max_tx_pwr = 0;  // ¹ã²¥¹¦ÂÊ
        #endif

        if (app_adv_env.cur_mode == APP_ADV_MODE_DIRECTED) {
            if(app_adv_env.config.directed_enabled == false) {
                app_adv_env.cur_mode = APP_ADV_MODE_FAST;
            }
            else if (app_sec_get_bond_status() == false) {
                app_adv_env.cur_mode = APP_ADV_MODE_FAST;
            }
        }
        if (app_adv_env.cur_mode == APP_ADV_MODE_FAST && app_adv_env.config.fast_enabled == false)
        {
            app_adv_env.cur_mode = APP_ADV_MODE_SLOW;
        }
        app_adv_env.next_mode = app_adv_env.cur_mode;
        switch(app_adv_env.cur_mode)
        {
            case APP_ADV_MODE_DIRECTED:
                /*
                 * If the peripheral is already bonded with a central device, use the direct advertising
                 * procedure (BD Address of the peer device is stored in NVDS.
                 */
                if (app_sec_get_bond_status())
                {
                    // BD Address of the peer device
                    struct gap_bdaddr peer_bd_addr;
#if (NVDS_SUPPORT)
                    struct app_sec_conn_info_tag info;
                    nvds_tag_len_t length = sizeof(struct app_sec_conn_info_tag);
                    uint8_t index = 0;

                    uint8_t nvds_len = NVDS_TAG_BLE_LINK_KEY_LAST-NVDS_TAG_BLE_LINK_KEY_FIRST;

                    // Get bond status from NVDS
                    for(index = 0; index <=nvds_len; index++)
                    {
                        if (nvds_get(NVDS_TAG_BLE_LINK_KEY_LAST-index, &length, (uint8_t *)&info) == NVDS_OK)
                        {
#if 0//(BLE_APP_PRIVACY)
                            memcpy(&peer_bd_addr, &info.irk.addr, sizeof(struct gap_bdaddr));
#else
                            memcpy(&peer_bd_addr, &info.id_addr, sizeof(struct gap_bdaddr));
#endif
                            break;
                        }
                    }
#else
                    memset(&peer_bd_addr, 0, sizeof(struct gap_bdaddr));
#endif
                    // Set the DIRECT ADVERTISING mode
                    p_cmd->adv_param.disc_mode = GAPM_ADV_MODE_NON_DISC;
                    if (app_adv_env.config.directed_timeout == 0) {
                    p_cmd->adv_param.prop = GAPM_ADV_PROP_DIR_CONN_HDC_MASK;
                    }
                    else {
                        p_cmd->adv_param.prop = GAPM_ADV_PROP_DIR_CONN_MASK;
                    }
                    // Copy the BD address of the peer device and the type of address
                    memcpy(&p_cmd->adv_param.peer_addr, &peer_bd_addr, sizeof(struct gap_bdaddr));
                }
                p_cmd->adv_param.prim_cfg.adv_intv_min = APP_ADV_DIRECT_INTERVAL;
                p_cmd->adv_param.prim_cfg.adv_intv_max = APP_ADV_DIRECT_INTERVAL;
                break;
            case APP_ADV_MODE_FAST:
                p_cmd->adv_param.prim_cfg.adv_intv_min = app_adv_env.config.fast_interval;
                p_cmd->adv_param.prim_cfg.adv_intv_max = app_adv_env.config.fast_interval;
                break;
            case APP_ADV_MODE_SLOW:
                p_cmd->adv_param.prim_cfg.adv_intv_min = app_adv_env.config.slow_interval;
                p_cmd->adv_param.prim_cfg.adv_intv_max = app_adv_env.config.slow_interval;
                break;
            default:
                break;
        }
        
        // Send the message
        ke_msg_send(p_cmd);

        // Keep the current operation
        app_adv_env.state = APP_ADV_STATE_CREATING;
        // And the next expected operation code for the command completed event
        app_adv_env.op = GAPM_CREATE_ADV_ACTIVITY;
    }
}


void appm_adv_delete_advertising(void)
{
    //log_debug("%s@%d\n", __func__, __LINE__);
    // Prepare the GAPM_ACTIVITY_CREATE_CMD message
    struct gapm_activity_delete_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_DELETE_CMD,
                                                              TASK_GAPM, TASK_APP,
                                                              gapm_activity_delete_cmd);

    // Set operation code
    p_cmd->operation = GAPM_DELETE_ACTIVITY;
    p_cmd->actv_idx = app_adv_env.actv_idx;

    // Send the message
    ke_msg_send(p_cmd);

    // Keep the current operation
    app_adv_env.state = APP_ADV_STATE_DELETING;
    // And the next expected operation code for the command completed event
    app_adv_env.op = GAPM_DELETE_ACTIVITY;
}

static void appm_adv_fsm_next(void)
{
    //log_debug("%s@%d, mode=%x, state=%x, next event=%x\r\n", __func__, __LINE__, 
    //                                                    app_adv_env.cur_mode,
    //                                                    app_adv_env.state,
    //                                                    app_adv_env.next_event);
    switch (app_adv_env.state)
    {
        case (APP_ADV_STATE_IDLE):
        {
            //log_debug("idle\n");
            // Create advertising
            appm_adv_create_advertising();
        } break;

        case (APP_ADV_STATE_CREATING):
        {
            //log_debug("creating\n");
            if (app_adv_env.cur_mode == APP_ADV_MODE_DIRECTED)
            {
                // Start advertising activity
                if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_START)
                {
                    appm_adv_start_advertising();
                }
            }
            else {
                // Set advertising data
                appm_adv_set_adv_data_cmd();
            }
        } break;

        case (APP_ADV_STATE_SETTING_ADV_DATA):
        {
            //log_debug("setting adv data\n");
            if (app_adv_env.res_data_len > 0 ) {
                // Set scan response data
                appm_adv_set_scan_rsp_data_cmd();
            }
            else if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_START)
            {
                appm_adv_start_advertising();
            }
        } break;

        case (APP_ADV_STATE_CREATED):
        case (APP_ADV_STATE_SETTING_SCAN_RSP_DATA):
        {
            if (app_adv_env.state == APP_ADV_STATE_CREATED) {
                //log_debug("created\n");
            }
            else {
                //log_debug("setting scan res data\n");
            }
            // Start advertising activity
            if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_START)
            {
                appm_adv_start_advertising();
            }
        } break;

        case (APP_ADV_STATE_STARTING):
        {
            //log_debug("starting\n");
            #if (DISPLAY_SUPPORT)
            // Update advertising state screen
            app_display_set_adv(true);
            #endif //(DISPLAY_SUPPORT)

            // Go to started state
            app_adv_env.state = APP_ADV_STATE_STARTED;
            if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_START)
            {
                app_adv_env.next_event &= (~APP_ADV_NEXT_EVENT_START);
            }
            else if ((app_adv_env.next_event & APP_ADV_NEXT_EVENT_STOP) || (app_adv_env.next_event & APP_ADV_NEXT_EVENT_DEL))
            {
                appm_adv_stop_advertising();
            }
        } break;

        case (APP_ADV_STATE_STARTED):
        {
            //log_debug("started\n");

            // Stop advertising activity
            appm_adv_stop_advertising();
        } break;

        case (APP_ADV_STATE_STOPPING):
        {
            //log_debug("stopping\n");

            #if (DISPLAY_SUPPORT)
            // Update advertising state screen
            app_adv_display_set_adv(false);
            #endif //(DISPLAY_SUPPORT)

            // Go created state
            app_adv_env.state = APP_ADV_STATE_CREATED;
            if(app_adv_env.cur_mode != app_adv_env.next_mode) {
                appm_adv_delete_advertising();
            }
            else
            {
                if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_STOP)
                {
                    app_adv_env.next_event &= (~APP_ADV_NEXT_EVENT_STOP);
                }
                
                if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_DEL)
                {
                    app_adv_env.next_event &= (~APP_ADV_NEXT_EVENT_DEL);
                    appm_adv_delete_advertising();
                }
                else if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_START)
                {
                    app_adv_env.next_event &= (~APP_ADV_NEXT_EVENT_START);
                    appm_adv_start_advertising();
                }
            }
        } break;

        case (APP_ADV_STATE_DELETING):
        {
            //log_debug("deleting\n");
            app_adv_env.state = APP_ADV_STATE_IDLE;
            if(app_adv_env.cur_mode != app_adv_env.next_mode) {
                app_adv_env.cur_mode = app_adv_env.next_mode;
                if (app_adv_env.cur_mode != APP_ADV_MODE_IDLE) {
                    app_adv_env.next_event |= APP_ADV_NEXT_EVENT_START;
                    appm_adv_create_advertising();
                }
                else
                {
                     if (app_adv_env.config.directed_enabled == true)
                    {
                        app_adv_env.cur_mode = APP_ADV_MODE_DIRECTED;
                    }
                    else if (app_adv_env.config.fast_enabled == true)
                    {
                        app_adv_env.cur_mode = APP_ADV_MODE_FAST;
                    }
                    else if (app_adv_env.config.slow_enabled == true)
                    {
                        app_adv_env.cur_mode = APP_ADV_MODE_SLOW;
                    }
                    if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_START)
                    {
                        appm_adv_create_advertising();
                    }
                }
            }
            else
            {
                if (app_adv_env.next_event & APP_ADV_NEXT_EVENT_START)
                {
                    appm_adv_create_advertising();
                }
            }
        }break;

        default:
        {
            ASSERT_ERR(0);
        } break;
    }
}

void appm_adv_update_state(bool start)
{
    // TODO [LT] - Check current advertising state

    // Start or stop advertising
    if (start)
    {
        appm_adv_start();
    }
    else {
        appm_adv_fsm_next();
    }
}


void appm_adv_init(struct app_adv_data_tag* adv_data, struct app_adv_modes_config_tag *config)
{
    memset(&app_adv_env, 0, sizeof(struct app_adv_env_tag));
    if (adv_data != NULL)
    {
        appm_adv_build_adv_data(adv_data);
    }
    if (config != NULL)
    {
        memcpy(&app_adv_env.config, config, sizeof(struct app_adv_modes_config_tag));
        
        #if (BLE_APP_WHITE_LIST)
        appm_whl_init(app_adv_env.config.whitelist_enabled);
        #endif
        #if (BLE_APP_PRIVACY)
        appm_privacy_init(1);
        #endif
        if (config->fast_interval == 0)
        {
            app_adv_env.config.fast_enabled = false;
        }
        if (config->slow_interval == 0)
        {
            app_adv_env.config.slow_enabled = false;
        }

        if (app_adv_env.config.directed_enabled == true)
        {
            app_adv_env.cur_mode = APP_ADV_MODE_DIRECTED;
        }
        else if (app_adv_env.config.fast_enabled == true)
        {
            app_adv_env.cur_mode = APP_ADV_MODE_FAST;
        }
        else if (app_adv_env.config.slow_enabled == true)
        {
            app_adv_env.cur_mode = APP_ADV_MODE_SLOW;
        }
        else
        {
            ASSERT_ERR(0);
        }
    }
}

void appm_adv_set_adv_data(uint8_t* adv_data, uint8_t len)
{
    if (adv_data != NULL && len < APP_ADV_MAX_LEN)
    {
        memcpy(app_adv_env.adv_data, adv_data, len);
        app_adv_env.adv_data_len = len;
    }
}

void appm_adv_set_res_data(uint8_t* res_data, uint8_t len)
{
    if (res_data != NULL && len < APP_ADV_MAX_LEN)
    {
        memcpy(app_adv_env.res_data, res_data, len);
        app_adv_env.res_data_len = len;
    }
}


void appm_adv_create_ind_handler(void *p_param)
{
    struct gapm_activity_created_ind *param = (struct gapm_activity_created_ind *)p_param;

    //log_debug("%s@%d,%x\n", __func__, __LINE__, param->actv_idx);

    if (param->actv_type == GAPM_ACTV_TYPE_ADV) {
        if (app_adv_env.state == APP_ADV_STATE_CREATING)
        {
            // Store the advertising activity index
            app_adv_env.actv_idx = param->actv_idx;
        }
    }
}

void appm_adv_stopped_ind_handler(void *p_param)
{
    struct gapm_activity_stopped_ind *param = (struct gapm_activity_stopped_ind *)p_param;

    //log_debug("%s@%d,%x\n", __func__, __LINE__, param->reason);

    if (param->actv_type == GAPM_ACTV_TYPE_ADV) {

        if (app_adv_env.state == APP_ADV_STATE_STARTED && app_adv_env.actv_idx == param->actv_idx)
        {
            // Act as if activity had been stopped by the application
            app_adv_env.state = APP_ADV_STATE_STOPPING;
            if (param->reason == GAP_ERR_TIMEOUT) {
                app_adv_env.next_mode = app_adv_env.cur_mode+1;
                //log_debug("next_mode=%d\n", app_adv_env.next_mode);
                if(app_adv_env.next_mode > APP_ADV_MODE_SLOW)
                {
                    app_adv_env.next_mode = APP_ADV_MODE_IDLE;
                }
            }
            else if (param->reason == 0) // connect success
            {
                // adv mode >= 2, and mode == last mode. need del            

                uint8_t adv_type_num = 0;
                uint8_t last_mode = APP_ADV_MODE_IDLE;
                if (app_adv_env.config.directed_enabled) adv_type_num++;
                if (app_adv_env.config.fast_enabled) {
                    adv_type_num++;
                    last_mode = APP_ADV_MODE_FAST;
                 }
                if (app_adv_env.config.slow_enabled) {
                    adv_type_num++;
                    last_mode = APP_ADV_MODE_SLOW;
                }
                if (adv_type_num >=2 && app_adv_env.cur_mode == last_mode) {
                    app_adv_env.next_mode = app_adv_env.cur_mode+1;
                    if(app_adv_env.next_mode > APP_ADV_MODE_SLOW)
                    {
                        app_adv_env.next_mode = APP_ADV_MODE_IDLE;
                    }
                }
            }
            // Perform next operation
            appm_adv_fsm_next();
        }
    }
}

/**
 ****************************************************************************************
 * @brief complete event handler
 *
 * @param[in] param: the complete param.
 * ****************************************************************************************
 */
void appm_adv_cmp_evt_handler(void *param)
{
    struct gapm_cmp_evt *p_param = (struct gapm_cmp_evt *)param;

    //log_debug("%s@%d,%x,%x\n", __func__, __LINE__, p_param->operation, p_param->status);

    switch(p_param->operation) {
        case (GAPM_CREATE_ADV_ACTIVITY):
        case (GAPM_STOP_ACTIVITY):
        case (GAPM_START_ACTIVITY):
        case (GAPM_DELETE_ACTIVITY):
        case (GAPM_SET_ADV_DATA):
        case (GAPM_SET_SCAN_RSP_DATA):
            if(app_adv_env.op == p_param->operation)
            {
                app_adv_env.op = 0;
                appm_adv_fsm_next();
            }
            break;
        case (GAPM_DELETE_ALL_ACTIVITIES) :
            {
                // Re-Invoke Advertising
                if(app_adv_env.op == p_param->operation) {
                    app_adv_env.state = APP_ADV_STATE_IDLE;
                    app_adv_env.op = 0;
                    appm_adv_fsm_next();
                }
            } break;
        default:
            break;
    }
}

void appm_adv_start(void)
{
    switch (app_adv_env.state)
    {
        case (APP_ADV_STATE_IDLE):
            // Create advertising
            appm_adv_create_advertising();
            app_adv_env.next_event |= APP_ADV_NEXT_EVENT_START;
            break;

        case (APP_ADV_STATE_CREATING):
        case (APP_ADV_STATE_SETTING_ADV_DATA):
            break;

        case (APP_ADV_STATE_CREATED):
            appm_adv_start_advertising();
            break;
        case (APP_ADV_STATE_SETTING_SCAN_RSP_DATA):
        case (APP_ADV_STATE_STARTING):
        case (APP_ADV_STATE_STARTED):
            break;
        case (APP_ADV_STATE_STOPPING):
        case (APP_ADV_STATE_DELETING):
            app_adv_env.next_event |= APP_ADV_NEXT_EVENT_START;
            break;

        default:
            break;
    }
}

void appm_adv_stop(uint8_t is_del)
{
    switch (app_adv_env.state)
    {
        case (APP_ADV_STATE_IDLE):
            break;

        case (APP_ADV_STATE_CREATING):
        case (APP_ADV_STATE_SETTING_ADV_DATA):
        case (APP_ADV_STATE_CREATED):
        case (APP_ADV_STATE_SETTING_SCAN_RSP_DATA):
        case (APP_ADV_STATE_STARTING):
            if (is_del)
                app_adv_env.next_event |= APP_ADV_NEXT_EVENT_DEL;
            else
                app_adv_env.next_event |= APP_ADV_NEXT_EVENT_STOP;
            break;
        case (APP_ADV_STATE_STARTED):
            if (is_del)
                app_adv_env.next_event |= APP_ADV_NEXT_EVENT_DEL;
            else
                app_adv_env.next_event |= APP_ADV_NEXT_EVENT_STOP;
            // Stop advertising activity
            appm_adv_stop_advertising();
            break;

        case (APP_ADV_STATE_STOPPING):
        case (APP_ADV_STATE_DELETING):
            break;

        default:
            break;
    }
}


void appm_adv_set_mode(uint8_t mode)
{
    if(mode >APP_ADV_MODE_SLOW) return;
    app_adv_env.cur_mode = mode;
}

uint8_t appm_adv_get_state(void)
{
    return app_adv_env.state;
}
#endif //(BLE_APP_PRESENT)

/// @} APP
