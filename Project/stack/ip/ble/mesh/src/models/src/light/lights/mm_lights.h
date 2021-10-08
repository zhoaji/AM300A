/**
 ****************************************************************************************
 *
 * @file mm_lights.h
 *
 * @brief Header file for Mesh Model Lighting Server Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_LIGHTS_H_
#define _MM_LIGHTS_H_

/**
 ****************************************************************************************
 * @defgroup MM_LIGHTS Mesh Model Lighting Server Module
 * @ingroup MESH_MDL
 * @brief Mesh Model Lighting Server Module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mm_inc.h"       // Mesh Model Includes
#include "mesh_api.h"       // Mesh Model API Definitions

/*
 * DEFINES
 ****************************************************************************************
 */
/// Head length for buffers allocated by the SIG Light Model
#define MM_LIGHT_BUF_HEAD_LEN         (2)

#if (APP_MESH_TMALL)
/// Support of Lighting Server Model --- Lightness
#define BLE_MESH_MDL_LIGHTS_LN        (1)
/// Support of Lighting Server Model --- Lightness Setup
#define BLE_MESH_MDL_LIGHTS_LNS       (1)
/// Support of Lighting Server Model --- CTL
#define BLE_MESH_MDL_LIGHTS_CTL       (1)
/// Support of Lighting Server Model --- CTL Temperature
#define BLE_MESH_MDL_LIGHTS_CTLT      (1)
/// Support of Lighting Server Model --- CTL Setup
#define BLE_MESH_MDL_LIGHTS_CTLS      (1)

#else

/// Support of Lighting Server Model --- Lightness
#define BLE_MESH_MDL_LIGHTS_LN        (0)
/// Support of Lighting Server Model --- Lightness Setup
#define BLE_MESH_MDL_LIGHTS_LNS       (0)
/// Support of Lighting Server Model --- CTL
#define BLE_MESH_MDL_LIGHTS_CTL       (0)
/// Support of Lighting Server Model --- CTL Temperature
#define BLE_MESH_MDL_LIGHTS_CTLT      (0)
/// Support of Lighting Server Model --- CTL Setup
#define BLE_MESH_MDL_LIGHTS_CTLS      (0)

#endif

/// Support of Lighting Server Model --- HSL
#define BLE_MESH_MDL_LIGHTS_HSL       (0)
/// Support of Lighting Server Model --- HSL Hue
#define BLE_MESH_MDL_LIGHTS_HSLH      (0)
/// Support of Lighting Server Model --- HSL Saturation
#define BLE_MESH_MDL_LIGHTS_HSLSAT    (0)
/// Support of Lighting Server Model --- HSL Setup
#define BLE_MESH_MDL_LIGHTS_HSLS      (0)
/// Support of Lighting Server Model --- xyL
#define BLE_MESH_MDL_LIGHTS_XYL       (0)
/// Support of Lighting Server Model --- xyL Setup
#define BLE_MESH_MDL_LIGHTS_XYLS      (0)
/// Support of Lighting Server Model --- LC
#define BLE_MESH_MDL_LIGHTS_LC        (0)
/// Support of Lighting Server Model --- LC Setup
#define BLE_MESH_MDL_LIGHTS_LCS       (0)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Mesh Model Lighting Server Module initialization function
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_lights_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return memory size needed for environment allocation of Mesh Model Lighting Server Module
 *
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_lights_get_env_size(const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Mesh Model Light Lightness Server Module initialization function
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_lights_ln_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_lights_lns_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_lights_ctl_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_lights_ctlt_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_lights_ctls_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return memory size needed for environment allocation of Mesh Model Light Lightness Server Module
 *
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_lights_ln_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_lights_lns_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_lights_ctl_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_lights_ctlt_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_lights_ctls_get_env_size(const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return address size needed for environment allocation of Mesh Model Light Lightness Server Module
 *
 *
 * @return Address of environment required for this module
 ****************************************************************************************
 */
void * mm_lights_ln_get_env_addr(void);
void * mm_lights_lns_get_env_addr(void);
void * mm_lights_ctl_get_env_addr(void);
void * mm_lights_ctlt_get_env_addr(void);
void * mm_lights_ctls_get_env_addr(void);



void mm_lights_ln_publish_state(void);
void mm_lights_ln_lir_publish_state(void);
void mm_light_ln_poweron_publish(void);

void mm_lights_ctl_publish_state(void);
void mm_lights_ctlt_publish_state(void);

/// @} end of group

#endif //_MM_LIGHTS_
