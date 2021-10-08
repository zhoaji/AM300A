/**
 ****************************************************************************************
 *
 * @file mesh_dbg.h
 *
 * @brief Header file for Mesh Stack Debug Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MESH_DBG_
#define MESH_DBG_

/**
 ****************************************************************************************
 * @defgroup MESH_DBG Mesh Stack Debug Module
 * @ingroup MESH
 * @brief  Mesh Stack Debug Module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_inc.h"    // Mesh Stack Includes

#if (BLE_MESH_DBG)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Mesh Stack Debug Module initialization
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to mesh stack initialization parameters
 *
 * @return Size of environment required for this module.
 ****************************************************************************************
 */
uint16_t mesh_dbg_init(bool reset, void *p_env, const mesh_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Get size needed for environments used by Mesh Stack Debug Module
 *
 * @param[in] p_cfg    Pointer to mesh stack initialization parameters
 *
 * @return Size of environment required for this module.
 ****************************************************************************************
 */
uint16_t mesh_dbg_get_env_size(const mesh_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Handler for Mesh Stack Debug messages
 *
 * @param[in] msg_id        Identifier of the message received
 * @param[in] src_id        Identifier of the task that issue message
 * @param[in] p_param       Message parameters
 *
 * @return Status of message after handling (@see enum mal_msg_status)
 ****************************************************************************************
 */
uint8_t mesh_dbg_handler(uint16_t msg_id, uint16_t src_id, const void *p_param);

#endif //(BLE_MESH_DBG)

/// @} MESH_DBG

#endif /* MESH_DBG_ */
