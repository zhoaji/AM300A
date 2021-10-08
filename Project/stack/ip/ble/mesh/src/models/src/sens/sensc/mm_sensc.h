/**
 ****************************************************************************************
 *
 * @file mm_sensc.h
 *
 * @brief Header file for Mesh Model Sensor Client Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_SENSC_H_
#define _MM_SENSC_H_

/**
 ****************************************************************************************
 * @defgroup MM_SENSC Mesh Model Sensor Client Module
 * @ingroup MESH_MDL
 * @brief Mesh Model Sensor Client Module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mm_inc.h"       // Mesh Model Includes
#include "mesh_api.h"       // Mesh Model API Definitions

#if (BLE_MESH_MDL_SENSC)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Mesh Model Sensor Client Module initialization function
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_sensc_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return memory size needed for environment allocation of Mesh Model Sensor Client Module
 *
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_sensc_get_env_size(const mm_cfg_t *p_cfg);

#endif //(BLE_MESH_MDL_SENSC)

/// @} end of group

#endif //_MM_SENSC_
