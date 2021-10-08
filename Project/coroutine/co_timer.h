/**
 * @file co_timer.h
 * @brief 
 * @date Tue, Apr 30, 2019 11:26:04 AM
 * @author liqiang
 *
 * @defgroup CO_TIMER Software Timer
 * @ingroup COROUTINE
 * @brief Software Timer Module
 * @details The software timer
 *
 * @{
 */

#ifndef __CO_TIMER_H__
#define __CO_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "co.h"

#ifdef CONFIG_USE_RWIP_CO_TIMER
#include "co_timer_rwip.h"
#else
#include "co_timer_pmu.h"
#endif

/*********************************************************************
 * MACROS
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

