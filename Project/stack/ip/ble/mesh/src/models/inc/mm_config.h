/**
 ****************************************************************************************
 *
 * @file mm_config.h
 *
 * @brief Header file for Mesh Model Configuration Defines
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MM_CONFIG_
#define MM_CONFIG_

/**
 ****************************************************************************************
 * @defgroup MM_CONFIG Mesh Model Configuration Defines
 * @ingroup MESH_MDL
 * @brief  Mesh Model Configuration Defines
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_config.h"         // Mesh Stack Configuration

/*
 * COMPILATION FLAGS
 ****************************************************************************************
 */

/// Support of Generic Server Models
#if (defined(CFG_BLE_MESH_MDL_GENS))
#define BLE_MESH_MDL_GENS                    (1)
#else
#define BLE_MESH_MDL_GENS                    (0)
#endif //(MESH_MDL_GENS)

/// Support of Generic Client Models
#if (defined(CFG_BLE_MESH_MDL_GENC))
#define BLE_MESH_MDL_GENC                    (1)
#else
#define BLE_MESH_MDL_GENC                    (0)
#endif //(MESH_MDL_GENC)

/// Support of Sensor Server Models
#if (defined(CFG_BLE_MESH_MDL_SENSS))
#define BLE_MESH_MDL_SENSS                   (1)
#else
#define BLE_MESH_MDL_SENSS                   (0)
#endif //(BLE_MESH_MDL_SENSS)

/// Support of Sensor Client Models
#if (defined(CFG_BLE_MESH_MDL_SENSC))
#define BLE_MESH_MDL_SENSC                   (1)
#else
#define BLE_MESH_MDL_SENSC                   (0)
#endif //(BLE_MESH_MDL_SENSC)

/// Support of Time and Scenes Server Models
#if (defined(CFG_BLE_MESH_MDL_TSCNS))
#define BLE_MESH_MDL_TSCNS                   (1)
#else
#define BLE_MESH_MDL_TSCNS                   (0)
#endif //(BLE_MESH_MDL_TSCNS)

/// Support of Time and Scenes Client Models
#if (defined(CFG_BLE_MESH_MDL_TSCNC))
#define BLE_MESH_MDL_TSCNC                   (1)
#else
#define BLE_MESH_MDL_TSCNC                   (0)
#endif //(BLE_MESH_MDL_TSCNC)

/// Support of Light Server Models
#if (defined(CFG_BLE_MESH_MDL_LIGHTS))
#define BLE_MESH_MDL_LIGHTS                  (1)
#else
#define BLE_MESH_MDL_LIGHTS                  (0)
#endif //(BLE_MESH_MDL_LIGHTS)

/// Support of Light Client Models
#if (defined(CFG_BLE_MESH_MDL_LIGHTC))
#define BLE_MESH_MDL_LIGHTC                  (1)
#else
#define BLE_MESH_MDL_LIGHTC                  (0)
#endif //(BLE_MESH_MDL_LIGHTS)

/// @} MM_CONFIG

#endif /* MM_CONFIG_ */
