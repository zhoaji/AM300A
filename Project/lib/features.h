/**
 * @file features.h
 * @brief 
 * @date Sat 14 Feb 2015 09:55:02 AM CST
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

#ifndef __FEATURES_H__
#define __FEATURES_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#ifdef CONFIG_AUTOCONFG_VERSION
#include "autoconf_version.h"
#endif

// HS6621
#if defined(CONFIG_HS6621A1_RELEASE)
#include "autoconf_release_hs6621_a1.h"
#elif defined(CONFIG_HS6621A2_RELEASE)
#include "autoconf_release_hs6621_a2.h"
#elif defined(CONFIG_HS6621A3_RELEASE)
#include "autoconf_release_hs6621_a3.h"
#elif defined(CONFIG_HS6621B1_RELEASE)
#include "autoconf_release_hs6621_b1.h"
#elif defined(CONFIG_HS6621B2_RELEASE)
#include "autoconf_release_hs6621_b2.h"

// HS6621C
#elif defined(CONFIG_HS6621CA1_RELEASE)
#ifdef CONFIG_BLE_ALLROLES
#include "autoconf_release_hs6621c_allroles_a1.h"
#else
#include "autoconf_release_hs6621c_a1.h"
#endif
#elif defined(CONFIG_HS6621CA2_RELEASE)
#ifdef CONFIG_BLE_ALLROLES
#include "autoconf_release_hs6621c_allroles_a2.h"
#else
#include "autoconf_release_hs6621c_a2.h"
#endif
#elif defined(CONFIG_HS6621CB1_RELEASE)
#ifdef CONFIG_BLE_ALLROLES
#include "autoconf_release_hs6621c_allroles_b1.h"
#else
#include "autoconf_release_hs6621c_b1.h"
#endif
#elif defined(CONFIG_HS6621CB2_RELEASE)
#include "autoconf_release_hs6621c_b2.h"

// HS6621P
#elif defined(CONFIG_HS6621P1_RELEASE)
#include "autoconf_release_hs6621p_a1.h"
#elif defined(CONFIG_HS6621P2_RELEASE)
#include "autoconf_release_hs6621p_a2.h"

// default
#elif defined(CONFIG_HS6621_USER_APP)
#include "autoconf_release_hs6621_a2.h"
#elif defined(CONFIG_HS6621C_USER_APP)
#include "autoconf_release_hs6621c_a1.h"

// DEBUG
#else
#include "autoconf.h"
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifdef CONFIG_USE_BASE_COMPONENT_SYMBOL
#include "bc.h"
#endif

/*********************************************************************
 * MACROS
 */

#define HARDWARE_VERSION_A(n)           ((n) + 0)
#define HARDWARE_VERSION_B(n)           ((n) + 5)
#define CONFIG_HARDWARE_BASE_VERSION    ((CONFIG_HARDWARE_VERSION>5) ? (CONFIG_HARDWARE_VERSION-5) : CONFIG_HARDWARE_VERSION)

// ROM_ID and app magic
#if defined(CONFIG_HS6621)
#define CONFIG_ROM_ID (0x66210000 | (CONFIG_HARDWARE_BASE_VERSION << 8) | (CONFIG_MINOR_VERSION << 0))

#elif defined(CONFIG_HS6621P)
#define CONFIG_ROM_ID (0x6621F000 | (CONFIG_HARDWARE_BASE_VERSION << 8) | (CONFIG_MINOR_VERSION << 0))

#elif defined(CONFIG_HS6621C)
#if CONFIG_HARDWARE_VERSION >= HARDWARE_VERSION_B(1)
#define CONFIG_ROM_ID (0x6621CB00 | (CONFIG_HARDWARE_BASE_VERSION << 4) | (CONFIG_MINOR_VERSION << 0))
#else
#define CONFIG_ROM_ID (0x6621C000 | (CONFIG_HARDWARE_BASE_VERSION << 8) | (CONFIG_MINOR_VERSION << 0))
#endif

#else
#error "Unknown chip"
#endif

#define BLE_VERSION_50     9
#define BLE_VERSION_51     10
#define BLE_VERSION_52     11

// lagacy design
#if (defined(CONFIG_HS6621) && CONFIG_HARDWARE_VERSION<=3) || (defined(CONFIG_HS6621C) && CONFIG_HARDWARE_VERSION<=2)
#define CONFIG_LAGACY_DESIGN
#endif

/*********************************************************************
 * REDEFINE MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */


#ifdef __cplusplus
}
#endif

#endif

/** @} */

