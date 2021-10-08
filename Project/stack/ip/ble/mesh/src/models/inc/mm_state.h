/**
 ****************************************************************************************
 *
 * @file mm_state.h
 *
 * @brief Header file for Mesh Model State Manager Module
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef _MM_STATE_H_
#define _MM_STATE_H_

/**
 ****************************************************************************************
 * @defgroup MM_API Mesh Model State Manager Module
 * @ingroup MESH_MDL
 * @brief Mesh Model State Manager Module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mm_inc.h"       // Mesh Model Include Files

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Some model states is bound to an instance of the model state source.
typedef enum
{
    MM_STATE_GEN_ON_POWERUP,
    MM_STATE_GEN_ONOFF,
    MM_STATE_GEN_LEVEL,
    MM_STATE_GEN_POWER_ACTUAL,
    MM_STATE_GEN_POWER_SETUP,
    MM_STATE_LIGHT_LN_ACTUAL,
    MM_STATE_LIGHT_LN_LINEAR,
    MM_STATE_LIGHT_LN_SETUP,
    MM_STATE_LIGHT_CCT,
    MM_STATE_LIGHT_CCT_SETUP,
    MM_STATE_TSCNS_SCENE_STATUS,
} mm_state_binding_src_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Mesh Model State Manager Module initialization function
 *
 * @param[in] reset     True means SW reset, False means task initialization
 * @param[in] p_env     Pointer to the environment structure
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_state_init(bool reset, void *p_env, const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Return memory size needed for environment allocation of Mesh State Manager Module
 *
 * @param[in] p_cfg     Pointer to Mesh Model Configuration Structure provided by the Application
 *
 * @return Size of environment required for this module
 ****************************************************************************************
 */
uint16_t mm_state_get_env_size(const mm_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Inform the Model State Manager about registration of a new model
 *
 * @param[in] mdl_id        Model Identifier
 * @param[in] elmt_id       Index of element the model belongs to
 * @param[in] mdl_lid       Local index allocated by the mesh profile for the model
 * @param[in] env_size      Size of environment to allocate for this model
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t mm_state_register_ind(uint32_t mdl_id, uint16_t elmt_id, m_lid_t mdl_lid, uint16_t env_size);

void *mm_state_get_env_addr(uint16_t elmt_id);
void mm_state_update_binding(uint16_t elmt_id, mm_state_binding_src_t src);

/// @} end of group

#endif //_MM_STATE_
