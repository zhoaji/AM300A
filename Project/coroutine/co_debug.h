/*********************************************************************
 * @file co_debug.h
 * @brief 
 * @version 1.0
 * @date 14/11/3 19:28:20
 * @author liqiang
 *
 * @note 
 */

#ifndef __CO_DEBUG_H__
#define __CO_DEBUG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "features.h"
#include <stdint.h>
#include <stdio.h>

/*********************************************************************
 * MACROS
 */

#ifdef CONFIG_LOG_OUTPUT
#ifndef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL                        4
#endif
// debug output
#if CONFIG_LOG_LEVEL >= 1
#define log_error(format, ...)                  printf(format,  ## __VA_ARGS__) // level-1 Error conditions
#define log_error_array(array, len)             do{int __i; for(__i=0;__i<(len);++__i)log_error("%02X ",((uint8_t *)(array))[__i]);}while(0)
#define log_error_array_ex(note, array, len)    do{log_error("%s: ",note); log_error_array(array,len); log_error("[%dbytes]\n",len);}while(0)
#endif
#if CONFIG_LOG_LEVEL >= 2
#define log_warn(format, ...)                   printf(format,  ## __VA_ARGS__) // level-2 Warning conditions
#define log_warn_array(array, len)              do{int __i; for(__i=0;__i<(len);++__i)log_warn("%02X ",((uint8_t *)(array))[__i]);}while(0)
#define log_warn_array_ex(note, array, len)     do{log_warn("%s: ",note); log_warn_array(array,len); log_warn("[%dbytes]\n",len);}while(0)
#endif
#if CONFIG_LOG_LEVEL >= 3
#define log_info(format, ...)                   printf(format,  ## __VA_ARGS__) // level-3 informational
#define log_info_array(array, len)              do{int __i; for(__i=0;__i<(len);++__i)log_info("%02X ",((uint8_t *)(array))[__i]);}while(0)
#define log_info_array_ex(note, array, len)     do{log_info("%s: ",note); log_info_array(array,len); log_info("[%dbytes]\n",len);}while(0)
#endif
#if CONFIG_LOG_LEVEL >= 4
#define log_debug(format, ...)                  printf(format,  ## __VA_ARGS__) // level-4 debug level message
#define log_debug_array(array, len)             do{int __i; for(__i=0;__i<(len);++__i)log_debug("%02X ",((uint8_t *)(array))[__i]);}while(0)
#define log_debug_array_ex(note, array, len)    do{log_debug("%s: ",note); log_debug_array(array,len); log_debug("[%dbytes]\n",len);}while(0)
#endif
#else
#define log_error(format, ...)
#define log_warn(format, ...)
#define log_info(format, ...)
#define log_debug(format, ...)
#define log_error_array(array, len)
#define log_error_array_ex(note, array, len)
#define log_warn_array(array, len)
#define log_warn_array_ex(note, array, len)
#define log_info_array(array, len)
#define log_info_array_ex(note, array, len)
#define log_debug_array(array, len)
#define log_debug_array_ex(note, array, len)
#endif

#ifdef CONFIG_ASSERT
// Run Time Asserts, It is important for DEBUG
#define co_assert(exp)                  ((void)( (exp) || (__co_assert(#exp, __FILE__, __FUNCTION__, __LINE__), 0)))
#define co_verify(exp)                  ((void)( (exp) || (__co_assert(#exp, __FILE__, __FUNCTION__, __LINE__), 0)))
#else
#define co_assert(exp)
#define co_verify(exp)                  (exp)
#endif

// Compile Time Asserts
#define co_static_assert(exp)           extern char co_static_assert_failed[(exp)?1:-1]

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief __co_assert()
 *
 * @param[in] exp  
 * @param[in] file  
 * @param[in] func  
 * @param[in] line  
 *
 * @return 
 **/
int __co_assert(const char *exp, const char *file, const char *func, int line);

#ifdef __cplusplus
}
#endif

#endif

