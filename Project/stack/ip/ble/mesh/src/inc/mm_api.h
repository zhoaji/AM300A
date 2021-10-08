/**
 ****************************************************************************************
 *
 * @file mm_api.h
 *
 * @brief Header file for Mesh Model API Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_API_H_
#define _MM_API_H_

/**
 ****************************************************************************************
 * @defgroup MM_API Mesh Model API Module
 * @ingroup MESH_MDL
 * @brief Mesh Model API Module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_defines.h"     // Mesh Stack Definitions
#include "mesh_api.h"         // Mesh Stack API Definitions

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Mesh Model API Module initialization function
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_api_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return memory size needed for environment allocation of Mesh Model API Module
 *
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_api_get_env_size(const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief TODO [LT]
 ****************************************************************************************
 */
uint8_t mm_api_handler(uint16_t msg_id, uint16_t src_id, const void *p_param);

/// @} end of group

#endif //_MM_API_INT_
