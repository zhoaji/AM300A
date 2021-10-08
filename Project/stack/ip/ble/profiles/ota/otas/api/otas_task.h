/**
 ****************************************************************************************
 *
 * @file otas_task.h
 *
 * @brief Header file - Battery Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _OTAS_TASK_H_
#define _OTAS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup BAPSTASK Task
 * @ingroup BAPS
 * @brief Battery 'Profile' Task.
 *
 * The BAPS_TASK is responsible for handling the messages coming in and out of the
 * @ref BAPS block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "prf_types.h"
#include "rwip_task.h" // Task definitions
#include "gattc_task.h"
#include "attm.h"
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * APIs Structures
 ****************************************************************************************
 */

int otas_gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid, struct gattc_mtu_changed_ind *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
int otas_gapc_pkt_size_ind_handler(ke_msg_id_t const msgid,  struct gapc_le_pkt_size_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
int otas_gapc_param_update_ind_handler(ke_msg_id_t const msgid, struct gapc_param_updated_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
int otas_gapc_disconnect_ind_handler(ke_msg_id_t const msgid, struct gapc_disconnect_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);

bool otas_write(const uint8_t *pdata, uint16_t len);
void otas_reboot(void);

/// @} OTASTASK

#endif /* _OTAS_TASK_H_ */
