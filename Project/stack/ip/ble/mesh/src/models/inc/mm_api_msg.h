/**
 ****************************************************************************************
 *
 * @file mm_api_msg.h
 *
 * @brief Header file for Mesh Model Message Application Programming Interface
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_API_MSG_H_
#define _MM_API_MSG_H_

/**
 ****************************************************************************************
 * @defgroup MM_API_MSG Mesh Model Message Application Programming Interface
 * @ingroup MESH_MDL
 * @brief Mesh Model Message Application Programming Interface
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mm_inc.h"        // Mesh Model Include Files

#if (BLE_MESH_MSG_API)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Send a basic command complete message to the application
 *
 * @param[in] mdl_apid      Model API Identifier
 * @param[in] cmd_code      Command Code
 * @param[in] src_id        Source identifier of task that request command execution
 * @param[in] status        Status error code of the command execution (@see enum hl_err)
 ****************************************************************************************
 */
void mm_api_msg_send_basic_cmp_evt(uint16_t mdl_apid, uint16_t cmd_code, uint16_t src_id,
                                   uint16_t status);

#endif //(BLE_MESH_MSG_API)

/// @} end of group

#endif //_MM_API_MSG_
