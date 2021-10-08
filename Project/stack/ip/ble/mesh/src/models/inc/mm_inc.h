/**
 ****************************************************************************************
 *
 * @file mm_inc.h
 *
 * @brief Header file for Mesh Model Includes
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MM_INC_
#define MM_INC_

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

#include "mal.h"                        // Mesh Abstraction Layer Definitions
#include "mesh_api.h"                   // Mesh Stack Application Programming Interface Definitions
#include "mesh_api_msg.h"               // Mesh Stack Message Application Programming Interface Definitions
#include "mesh_defines.h"               // Mesh Stack Definitions
#include "mm_config.h"                  // Mesh Model Configuration Defines
#include "mm_defines.h"                 // Mesh Model Defines
#include "mm_tb.h"                      // Mesh Model Toolbox Defines
#include "mm_state.h"                   // Mesh Model State Defines
#include "../../prf/inc/m_tb.h"         // Mesh Toolbox Defines

#if (BLE_MESH_MDL_GENS)
#include "gen/gens/mm_gens.h"           // Generic Server Model Definitions
#endif //(BLE_MESH_MDL_GENS)
#if (BLE_MESH_MDL_GENC)
#include "gen/genc/mm_genc.h"           // Generic Client Model Definitions
#endif //(BLE_MESH_MDL_GENC)
#if (BLE_MESH_MDL_LIGHTS)
#include "light/lights/mm_lights.h"     // Lighting Server Model Definitions
#endif //(BLE_MESH_MDL_LIGHTS)
#if (BLE_MESH_MDL_LIGHTC)
#include "light/lightc/mm_lightc.h"     // Lighting Client Model Definitions
#endif //(BLE_MESH_MDL_LIGHTC)
#if (BLE_MESH_MDL_SENSS)
#include "sens/senss/mm_senss.h"        // Sensor Server Model Definitions
#endif //(BLE_MESH_MDL_SENSS)
#if (BLE_MESH_MDL_SENSC)
#include "sens/sensc/mm_sensc.h"        // Sensor Client Model Definitions
#endif //(BLE_MESH_MDL_SENSC)
#if (BLE_MESH_MDL_TSCNS)
#include "tscn/tscns/mm_tscns.h"        // Time and Scene Server Model Definitions
#endif //(BLE_MESH_MDL_TSCNS)
#if (BLE_MESH_MDL_TSCNC)
#include "tscn/tscnc/mm_tscnc.h"        // Time and Scene Client Model Definitions
#endif //(BLE_MESH_MDL_TSCNC)

#include "vnd/mm_vnd.h"                 // Vendor Model Definitions

/// @} MM_INC

#endif /* MM_INC_ */
