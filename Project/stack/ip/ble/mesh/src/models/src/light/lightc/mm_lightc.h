/**
 ****************************************************************************************
 *
 * @file mm_lightc.h
 *
 * @brief Header file for Mesh Model Lighting Client Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_LIGHTC_H_
#define _MM_LIGHTC_H_

/**
 ****************************************************************************************
 * @defgroup MM_LIGHTC Mesh Model Lighting Client Module
 * @ingroup MESH_MDL
 * @brief Mesh Model Lighting Client Module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mm_inc.h"       // Mesh Model Includes
#include "mesh_api.h"       // Mesh Model API Definitions

#if (BLE_MESH_MDL_LIGHTC)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Mesh Model Lighting Client Module initialization function
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_lightc_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return memory size needed for environment allocation of Mesh Model Lighting Client Module
 *
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_lightc_get_env_size(const mm_cfg_t *p_cfg);

#endif //(BLE_MESH_MDL_LIGHTC)

/// @} end of group

#endif //_MM_LIGHTC_
