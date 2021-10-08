/**
 ****************************************************************************************
 *
 * @file mm_tscns.h
 *
 * @brief Header file for Mesh Model Time and Scene Server Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_TSCNS_H_
#define _MM_TSCNS_H_

/**
 ****************************************************************************************
 * @defgroup MM_TSCNS Mesh Model Time and Scene Server Module
 * @ingroup MESH_MDL
 * @brief Mesh Model Time and Scene Server Module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mm_inc.h"       // Mesh Model Includes
#include "mesh_api.h"       // Mesh Model API Definitions

#if (BLE_MESH_MDL_TSCNS)

/*
 * DEFINES
 ****************************************************************************************
 */
 /// Head length for buffers allocated by the SIG Scene Server Model
 #define MM_TSCNS_BUF_HEAD_LEN          (2)

#if (APP_MESH_TMALL)
  /// Support of Time Server Mode --- Time Server
 #define BLE_MESH_MDL_TSCNS_TS          (0)
  /// Support of Time Setup Server Mode --- Time Setup Server
 #define BLE_MESH_MDL_TSCNS_TSS         (0)
  /// Support of Scene Server Model --- Scene Server
 #define BLE_MESH_MDL_TSCNS_SS          (0)
  /// Support of Scene Setup Server Model --- Scene Setup Server
 #define BLE_MESH_MDL_TSCNS_SSS         (0)
  /// Support of Scheduler Server Model --- Schedular Server
 #define BLE_MESH_MDL_TSCNS_SCH         (0)
 /// Support of Scheduler Setup Server Model --- Schedular Setup Server
 #define BLE_MESH_MDL_TSCNS_SCHS        (0)

#else

 /// Support of Time Server Mode --- Time Server
 #define BLE_MESH_MDL_TSCNS_TS          (0)
 /// Support of Time Setup Server Mode --- Time Setup Server
 #define BLE_MESH_MDL_TSCNS_TSS         (0)
 /// Support of Scene Server Model --- Scene Server
 #define BLE_MESH_MDL_TSCNS_SS          (0)
 /// Support of Scene Setup Server Model --- Scene Setup Server
 #define BLE_MESH_MDL_TSCNS_SSS         (0)
 /// Support of Scheduler Server Model --- Schedular Server
 #define BLE_MESH_MDL_TSCNS_SCH         (0)
 /// Support of Scheduler Setup Server Model --- Schedular Setup Server
 #define BLE_MESH_MDL_TSCNS_SCHS        (0)
#endif

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Mesh Model Time and Scene Server Module initialization function
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_tscns_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return memory size needed for environment allocation of Mesh Model Sensor Server Module
 *
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_tscns_get_env_size(const mm_cfg_t *p_cfg);

uint16_t mm_tscns_scene_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_tscns_scene_get_env_size(const mm_cfg_t *p_cfg);
void *mm_tscns_scene_get_env_addr(void);

#endif //(BLE_MESH_MDL_TSCNS)

/// @} end of group

#endif //_MM_TSCNS_
