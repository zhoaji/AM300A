/**
 ****************************************************************************************
 *
 * @file app_dis.h
 *
 * @brief Device Information Application Module Entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef APP_DIS_H_
#define APP_DIS_H_

/**
 ****************************************************************************************
 * @addtogroup APP_COMM_DIS_H app_dis.h
 * @ingroup APP_COMMON
 *
 * @brief Device Information Application Module Entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW Configuration

#if (BLE_APP_DIS)

#include <stdint.h>

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */

/// Table of message handlers
extern const struct app_subtask_handlers app_dis_handlers;

/*
 * GLOBAL FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Device Information Service Application
 ****************************************************************************************
 */
void app_dis_init(void);

/**
 ****************************************************************************************
 * @brief Add a Device Information Service instance in the DB
 ****************************************************************************************
 */
void app_dis_add_dis(void);

/**
 ****************************************************************************************
 * @brief Enable the Device Information Service
 ****************************************************************************************
 */
void app_dis_enable_prf(uint16_t conhdl);

#endif //BLE_APP_DIS

/// @} APP

#endif //APP_DIS_H_
