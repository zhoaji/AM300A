/**
 ****************************************************************************************
 *
 * @file mesh_config.h
 *
 * @brief Header file for Mesh Stack Configuration
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MESH_CONFIG_
#define MESH_CONFIG_

/**
 ****************************************************************************************
 * @defgroup MESH_CONFIG Mesh Stack Configuration
 * @ingroup MESH
 * @brief  Mesh Stack Configuration
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwip_task.h"

/*
 * COMPILATION FLAGS
 ****************************************************************************************
 */
/// Flag indicating that Mesh Debug is enabled
#if (defined(CFG_BLE_MESH_DBG))
#define BLE_MESH_DBG                         (1)
#else
#define BLE_MESH_DBG                         (0)
#endif //(defined(CFG_BLE_MESH_DBG))

/// Flag indicating that Mesh Message API is enabled
#if (defined(CFG_BLE_MESH_MSG_API))
#define BLE_MESH_MSG_API                     (1)
#else
#define BLE_MESH_MSG_API                     (0)
#endif //(defined(CFG_BLE_MESH_MSG_API))

#if (defined(CFG_BLE_MESH_STORAGE_NONE))
#define BLE_MESH_STORAGE_NONE                (1)
#else
#define BLE_MESH_STORAGE_NONE                (0)
#endif //(BLE_MESH_STORAGE_NONE)

#if (defined(CFG_BLE_MESH_STORAGE_WVT))
#define BLE_MESH_STORAGE_WVT                 (1)
#else
#define BLE_MESH_STORAGE_WVT                 (0)
#endif //(BLE_MESH_STORAGE_WVT)

#if (defined(CFG_BLE_MESH_STORAGE_NVDS))
#define BLE_MESH_STORAGE_NVDS                (1)
#else
#define BLE_MESH_STORAGE_NVDS                (0)
#endif //(CFG_BLE_MESH_STORAGE_NVDS)

/// Support of GATT Proxy feature
#if defined(CFG_BLE_MESH_GATT_PROXY)
#define BLE_MESH_GATT_PROXY                  (1)
#else
#define BLE_MESH_GATT_PROXY                  (0)
#endif //defined(CFG_BLE_MESH_GATT_PROXY)

/// Support of GATT Provisioning feature
#if defined(CFG_BLE_MESH_GATT_PROV)
#define BLE_MESH_GATT_PROV                   (1)
#else
#define BLE_MESH_GATT_PROV                   (0)
#endif //defined(CFG_BLE_MESH_GATT_PROV)

/// Used to know if GATT Bearer is present
#define BLE_MESH_GATT_BEARER                 (BLE_MESH_GATT_PROXY || BLE_MESH_GATT_PROV)

/// Support of Relay feature
#if defined(CFG_BLE_MESH_RELAY)
#define BLE_MESH_RELAY                       (1)
#else
#define BLE_MESH_RELAY                       (0)
#endif //defined(CFG_BLE_MESH_RELAY)

/// Support of Friend feature
#if defined(CFG_BLE_MESH_FRIEND)
#define BLE_MESH_FRIEND                      (1)
#else
#define BLE_MESH_FRIEND                      (0)
#endif //defined(CFG_BLE_MESH_FRIEND)

/// Support of Low Power Node feature
#if defined(CFG_BLE_MESH_LPN)
#define BLE_MESH_LPN                         (1)
#else
#define BLE_MESH_LPN                         (0)
#endif //defined(CFG_BLE_MESH_LPN)

/// Supported feature mask
#define BLE_MESH_FEAT_MASK   (  (BLE_MESH_MSG_API    * M_FEAT_MSG_API_SUP        )\
                              | (BLE_MESH_RELAY      * M_FEAT_RELAY_NODE_SUP     )\
                              | (BLE_MESH_GATT_PROXY * M_FEAT_PROXY_NODE_SUP     )\
                              | (BLE_MESH_GATT_PROV  * M_FEAT_PB_GATT_SUP        )\
                              | (BLE_MESH_FRIEND     * M_FEAT_FRIEND_NODE_SUP    )\
                              | (BLE_MESH_LPN        * M_FEAT_LOW_POWER_NODE_SUP ))

/*
 * DEFINES
 ****************************************************************************************
 */
#if APP_MESH_TMALL

/// Maximum number of subnets the node can belong to
#define MESH_SUBNET_NB_MAX                   (5)//(5)
/// Maximum number of models that can be registered on the node
/// Shall be at least 2 (for Configuration Server Model and Health Server Model)
#define MESH_MODEL_NB_MAX                    (15)//(6)

#else

#define MESH_SUBNET_NB_MAX                   (2)
#define MESH_MODEL_NB_MAX                    (10)

#endif
/// Maximum number of buffer block that can be allocated by the Buffer Manager
#define MESH_BUF_BLOCK_NB_MAX                (5)
/// Size of data part of "long" buffers
/// Value must be a multiple of 4
#define MESH_BUF_LONG_SIZE                   (152)
/// Size of data part of "small" buffers
/// Value must be a multiple of 4
#define MESH_BUF_SMALL_SIZE                  (32)
#if (BLE_MESH_FRIEND)
/// Number of "small" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_LONG_NB                (8)
/// Number of "long" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_SMALL_NB               (32)
#else
/// Number of "small" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_LONG_NB                (2)
/// Number of "long" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_SMALL_NB               (32)
#endif //(BLE_MESH_FRIEND)
/// Maximum number of buffers that can be dynamically allocated
#define MESH_BUF_DYN_NB_MAX                  (4)

/// @} MESH_CONFIG

#endif /* MESH_CONFIG_ */
