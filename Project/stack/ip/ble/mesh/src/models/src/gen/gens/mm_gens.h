/**
 ****************************************************************************************
 * @file mm_gens.h
 *
 * @brief Header file for Mesh Model Generic Server Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_GENS_H_
#define _MM_GENS_H_

/**
 ****************************************************************************************
 * @defgroup MM_GENS Mesh Model Generic Server Module
 * @ingroup MESH_MDL
 * @brief Mesh Model Generic Server Module
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
/// Head length for buffers allocated by the SIG Generic Model
#define MM_GENS_BUF_HEAD_LEN         (2)


#if (APP_MESH_TMALL)
/// Support of Generic Server Models --- Onoff
#define BLE_MESH_MDL_GENS_ONOFF      (1)
/// Support of Generic Server Models --- Level
#define BLE_MESH_MDL_GENS_LEVEL      (1)
/// Support of Generic Server Models --- Default Transition Time
#define BLE_MESH_MDL_GENS_DTT        (1)
/// Support of Generic Server Models --- Power ON/OFF
#define BLE_MESH_MDL_GENS_POO        (1)
/// Support of Generic Server Models --- Power ON/OFF Setup
#define BLE_MESH_MDL_GENS_POOS       (1)

#else

/// Support of Generic Server Models --- Onoff
#define BLE_MESH_MDL_GENS_ONOFF      (0)
/// Support of Generic Server Models --- Level
#define BLE_MESH_MDL_GENS_LEVEL      (0)
/// Support of Generic Server Models --- Default Transition Time
#define BLE_MESH_MDL_GENS_DTT        (0)
/// Support of Generic Server Models --- Power ON/OFF
#define BLE_MESH_MDL_GENS_POO        (0)
/// Support of Generic Server Models --- Power ON/OFF Setup
#define BLE_MESH_MDL_GENS_POOS       (0)

#endif
/// Support of Generic Server Models --- Power Level
#define BLE_MESH_MDL_GENS_PLVL       (0)
/// Support of Generic Server Models --- Power Level Setup
#define BLE_MESH_MDL_GENS_PLVLS      (0)
/// Support of Generic Server Models --- Battery
#define BLE_MESH_MDL_GENS_BAT        (0)
/// Support of Generic Server Models --- Location
#define BLE_MESH_MDL_GENS_LOC        (0)
/// Support of Generic Server Models --- Location Setup
#define BLE_MESH_MDL_GENS_LOCS       (0)
/// Support of Generic Server Models --- User Property
#define BLE_MESH_MDL_GENS_UPROP      (0)
/// Support of Generic Server Models --- Admin Property
#define BLE_MESH_MDL_GENS_APROP      (0)
/// Support of Generic Server Models --- Manufacturer Property
#define BLE_MESH_MDL_GENS_MPROP      (0)
/// Support of Generic Server Models --- Client Property
#define BLE_MESH_MDL_GENS_CPROP      (0)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

uint16_t mm_gens_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

uint16_t mm_gens_get_env_size(const mm_cfg_t *p_cfg);

uint16_t mm_gens_api_msg_handler_cmd(uint16_t cmd_code, const void *p_param);

uint16_t mm_gens_api_msg_handler_cfm(const void *p_param);

uint16_t mm_gens_get_model_info(uint32_t mdl_id, const m_api_model_cb_t **pp_mdl_cb,
                                uint16_t *p_env_size);

uint16_t mm_gens_onoff_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_gens_level_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_gens_dtt_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_gens_poo_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);
uint16_t mm_gens_poos_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

uint16_t mm_gens_onoff_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_gens_level_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_gens_dtt_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_gens_poo_get_env_size(const mm_cfg_t *p_cfg);
uint16_t mm_gens_poos_get_env_size(const mm_cfg_t *p_cfg);

void *mm_gens_onoff_get_env_addr(void);
void *mm_gens_level_get_env_addr(void);
void *mm_gens_dtt_get_env_addr(void);
void *mm_gens_poo_get_env_addr(void);
void *mm_gens_poos_get_env_addr(void);

void mm_gens_onoff_publish_state(void);
void mm_gens_level_publish_state(void);

uint8_t mm_gens_dtt_get_cur(void);

void mm_gens_level_sync_element(int16_t val);

/// @} end of group

#endif //_MM_GENS_
