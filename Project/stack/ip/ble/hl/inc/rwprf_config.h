/**
 ****************************************************************************************
 *
 * @file rwprf_config.h
 *
 * @brief Header file - Profile Configuration
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 ****************************************************************************************
 */


#ifndef _RWPRF_CONFIG_H_
#define _RWPRF_CONFIG_H_


/**
 ****************************************************************************************
 * @addtogroup PRF_CONFIG
 * @ingroup PROFILE
 * @brief Profile configuration
 *
 * @{
 ****************************************************************************************
 */

#define BLE_CLIENT_PRF          1
#define BLE_SERVER_PRF          1

//Force ATT parts depending on profile roles or compile options
/// Attribute Client
#if (BLE_CLIENT_PRF)
#define BLE_ATTC                    1
#endif //(BLE_CLIENT_PRF)

/// Attribute Server
#if (BLE_SERVER_PRF)
#define BLE_ATTS                    1
#endif //(BLE_SERVER_PRF)


/// @} PRF_CONFIG

#endif /* _RWPRF_CONFIG_H_ */
