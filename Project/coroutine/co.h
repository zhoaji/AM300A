/**
 * @file co.h
 * @brief library
 * @date Sat 17 Jan 2015 01:52:06 PM CST
 * @author liqiang
 *
 * @defgroup COROUTINE Library
 * @ingroup HS662X
 * @brief Basic Library
 * @details
 *
 * Provide some basic functions and structures, such as malloc, event, fifo, list, queue and so on
 *
 * @{
 */

#ifndef __CO_H__
#define __CO_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "features.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "co_debug.h"
#include "co_util.h"
//#include "co_list.h"
//#include "co_q.h"
#include "co_fifo.h"
#include "co_power.h"

#if defined(CONFIG_HS6621) || defined(CONFIG_HS6621C)
#include "co_port_hs6621.h"
#elif defined(HS6601) || defined(HS6601c)
#include "co_port_hs660x.h"
#else
#error Invalid Hardware Platform
#endif

#include "co_allocator.h"
#include "co_stack.h"
#include "co_timer.h"
#include "co_time.h"
#include "co_delay.h"
#include "co_fault.h"
#include "co_errno.h"

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

/// @cond
/**
 * @brief co_init()
 *
 * @return 
 **/
void co_init(void);

/**
 * @brief co_sche_once()
 *
 * @return 
 **/
void co_sche_once(void);

/// @endcond

#ifdef __cplusplus
}
#endif

#endif

/** @} */

