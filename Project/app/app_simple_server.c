/**
 ****************************************************************************************
 *
 * @file app_simple_server.c
 *
 * @brief Application Module entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_SIMPLE_SERVER_PROFILE_C app_simple_server.c
 * @ingroup APP_SIMPLE_SERVER
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_simple_server.h"       // Application Module Definitions
#include "app.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "simple_server_task.h"               // health thermometer functions
#include "ke_timer.h"
#include "co_bt.h"
#include "co_utils.h"
#include "prf_types.h"               // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include <string.h>

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Module Environment Structure
struct app_simple_server_env_tag app_simple_server_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_simple_server_init(void)
{
    // Reset the environment
    memset(&app_simple_server_env, 0, sizeof(struct app_simple_server_env_tag));

    // TODO: Initial something
}

const struct prf_task_cbs* simple_server_prf_itf_get(void);
void app_simple_server_add_service(void)
{
//    struct simple_server_db_cfg* db_cfg;

//    struct prf_itf_pair itf_pair = {
//        TASK_ID_SIMPLE_SERVER, simple_server_prf_itf_get(),
//    };
//    prf_itf_register(&itf_pair, 1);
   
    // Allocate the CREATE_DB_REQ
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct simple_server_db_cfg));
    // Fill message
    req->operation   = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl     = PERM(SVC_AUTH, NO_AUTH);
    req->prf_task_id = TASK_ID_SIMPLE_SERVER;
    req->app_task    = TASK_APP;
    req->start_hdl   = 0;

    // Set parameters and add some parameter if needed
//    db_cfg = (struct simple_server_db_cfg* ) req->param;

    // Send the message
    ke_msg_send(req);
}

void app_simple_server_enable_prf(uint8_t conidx)
{
    app_simple_server_env.conidx = conidx;
    // Allocate the message
    struct simple_server_enable_req * req = KE_MSG_ALLOC(SIMPLE_SERVER_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_SIMPLE_SERVER),
                                                TASK_APP,
                                                simple_server_enable_req);

    // Fill in the parameter structure
    req->conidx             = conidx;

    // Send the message
    ke_msg_send(req);
}

void app_simple_server_disable_prf(uint8_t conidx)
{
    app_simple_server_env.conidx = GAP_INVALID_CONIDX;
    ke_timer_clear(SIMPLE_SERVER_TIMEOUT_TIMER, TASK_APP);
}

static int simple_server_enable_rsp_handler(ke_msg_id_t const msgid,
                                    struct simple_server_enable_rsp const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

static int simple_server_demo_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                    struct simple_server_demo_ntf_cfg_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if(param->ntf_cfg == PRF_CLI_START_NTF || param->ntf_cfg == PRF_CLI_START_IND)
		{
// Remove 20210610    ke_timer_set(SIMPLE_SERVER_TIMEOUT_TIMER, TASK_APP, 100);
    }
		else if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
		{
// Remove 20210610       ke_timer_clear(SIMPLE_SERVER_TIMEOUT_TIMER, TASK_APP);
    }
    return (KE_MSG_CONSUMED);
}

#include "main.h"
static int simple_server_timeout_timer_handler(ke_msg_id_t const msgid,
                                               void const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
	
	log_debug("%s@%d\n", __func__, __LINE__);
	uint8_t ntf_data[] = "Notification : Data Transfer Test!";  // 20210519 SIMPLE_SERVER_SEND_NTF_CMD
	
	// Allocate the message
	struct simple_server_send_ntf_cmd * cmd = KE_MSG_ALLOC_DYN(SIMPLE_SERVER_SEND_NTF_CMD,
																							prf_get_task_from_id(TASK_ID_SIMPLE_SERVER),
																							TASK_APP,
																							simple_server_send_ntf_cmd,
																							sizeof(ntf_data));
	
	
	cmd->conidx = app_simple_server_env.conidx;
	cmd->length = sizeof(ntf_data);
	memcpy(cmd->value, ntf_data, sizeof(ntf_data));
	// Send the message
	ke_msg_send(cmd);

	ke_timer_set(SIMPLE_SERVER_TIMEOUT_TIMER, TASK_APP, 500); // 500
	
	return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_simple_server_msg_dflt_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    // Drop the message

    return (KE_MSG_CONSUMED);
}

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct ke_msg_handler app_simple_server_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_simple_server_msg_dflt_handler},
    {SIMPLE_SERVER_ENABLE_REQ,              (ke_msg_func_t)simple_server_enable_rsp_handler},
    {SIMPLE_SERVER_NTF_CFG_IND,        (ke_msg_func_t)simple_server_demo_ntf_cfg_ind_handler},
    {SIMPLE_SERVER_TIMEOUT_TIMER,   (ke_msg_func_t)simple_server_timeout_timer_handler},

};

/// simple server handler
const struct app_subtask_handlers app_simple_server_handlers = APP_HANDLERS(app_simple_server);

#include "crc8.h"
void ble_send_data(uint8_t *buff, uint32_t length)
{
	uint8_t i,crc = 0;
	
//	uint32_t length = 0;
	
//	length = QUEUE_STOCK(BLE_Tx);
	
	struct simple_server_send_ntf_cmd * cmd = KE_MSG_ALLOC_DYN(SIMPLE_SERVER_SEND_NTF_CMD,
																						prf_get_task_from_id(TASK_ID_SIMPLE_SERVER),
																						TASK_APP,
																						simple_server_send_ntf_cmd,
																						length + 1);
		for(i = 0; i < length; i++)
		{
			cmd->value[i] = buff[i];//QUEUE_READ(BLE_Tx);;
			crc = CRC8(crc, buff[i]);
		}
		cmd->value[i] = crc;
		
		cmd->conidx = app_simple_server_env.conidx;
		cmd->length = (length + 1);

		// Send the message
		ke_msg_send(cmd);
}


/// @} APP
