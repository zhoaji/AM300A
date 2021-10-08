/**
 ****************************************************************************************
 *
 * @file app_simple_server.h
 *
 * @brief Application simple server profile entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef APP_SIMPLE_SERVER_H_
#define APP_SIMPLE_SERVER_H_

/**
 ****************************************************************************************
 * @addtogroup APP_SIMPLE_SERVER_PROFILE_H app_simple_server.h
 * @ingroup APP_SIMPLE_SERVER
 *
 * @brief Application simple server profile entry point
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration
#include <stdint.h>          // Standard Integer Definition
#include "ke_task.h"         // Kernel Task Definition
#include "co_debug.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

///struct app_simple_server_env_tag
/// Application Module Environment Structure
struct app_simple_server_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Some other parameters
};

/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

extern struct app_simple_server_env_tag app_simple_server_env; /// Application environment

extern const struct app_subtask_handlers app_simple_server_handlers; /// Table of message handlers

/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * Health Thermometer Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Application Module
 * @return void.
 ****************************************************************************************
 */
void app_simple_server_init(void);

/**
 ****************************************************************************************
 * @brief Add a Service instance in the DB
 * @return void.
 ****************************************************************************************
 */
void app_simple_server_add_service(void);

/**
 ****************************************************************************************
 * @brief Enable the Service
 * @param[in] conidx: connect index.
 * @return void.
 ****************************************************************************************
 */
void app_simple_server_enable_prf(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Disable the Service
 * @param[in] conidx: connect index.
 * @return void.
 ****************************************************************************************
 */
void app_simple_server_disable_prf(uint8_t conidx);

// Some other functions


/// @} APP

#endif // APP_SIMPLE_SERVER_H_
