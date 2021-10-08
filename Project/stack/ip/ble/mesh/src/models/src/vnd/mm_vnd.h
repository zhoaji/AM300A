/**
****************************************************************************************
* @file mm_vnd.h
*
* @brief Header file for Mesh Vendor Model
*
* Copyright (C) HunterSun 2019
*
* @author liuqingtao
****************************************************************************************
*/

#ifndef _MM_VND_H_
#define _MM_VND_H_

/**
****************************************************************************************
* @defgroup MM_VND Mesh Vendor Model
* @ingroup MESH_MDL
* @brief Mesh Vendor Model
* @{
****************************************************************************************
*/

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mm_inc.h"       // Mesh Model Includes
#include "mesh_api.h"     // Mesh Model API Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// Support of Vendor Models --- Test
#define BLE_MESH_MDL_VND_TEST      (0)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
****************************************************************************************
* @brief Mesh Model Vendor Module initialization function
*
* @param[in] reset     True means SW reset, False means task initialization
* @param[in] p_env     Pointer to the environment structure
* @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
*
* @return Size of environment required for this module
****************************************************************************************
*/
uint16_t mm_vnd_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
/**
****************************************************************************************
* @brief Return memory size needed for environment allocation of Mesh Vendor Model Module
*
* @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
*
* @return Size of environment required for this module
****************************************************************************************
*/
uint16_t mm_vnd_get_env_size(const mm_cfg_t *p_cfg);

/**
****************************************************************************************
* @brief Mesh Model Vendor Test Module initialization function
*
* @param[in] reset     True means SW reset, False means task initialization
* @param[in] p_env     Pointer to the environment structure
* @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
*
* @return Size of environment required for this module
****************************************************************************************
*/
uint16_t mm_vnd_test_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_vnd_tmall_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
****************************************************************************************
* @brief Return memory size needed for environment allocation of Mesh Vendor Test Model Module
*
* @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
*
* @return Size of environment required for this module
****************************************************************************************
*/
uint16_t mm_vnd_test_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_vnd_tmall_get_env_size(const mm_cfg_t *p_cfg);

#if APP_MESH_TMALL
void mm_vnd_tmall_set_attr_direct(tmall_device_attr_type_t attr_type, uint8_t *p_attr_value);
void mm_vnd_tmall_get_attr_direct(tmall_device_attr_type_t attr_type, uint8_t *p_attr_value);
#endif


/// @} end of group

#endif //_MM_VND_
