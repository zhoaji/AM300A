/**
 * @file co_fault.h
 * @brief Software Fault Module
 * @date Mon 23 May 2016 05:13:39 PM CST
 * @author liqiang
 *
 * @defgroup CO_SOFTWARE_FAULT Software Fault
 * @ingroup COROUTINE
 * @brief Software fault module
 * @details 
 *
 * @{
 */

#ifndef __CO_FAULT_H__
#define __CO_FAULT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>

/*********************************************************************
 * MACROS
 */
/// @cond
// Use Cortex reserve fault handler
#define SOFT_FAULT_HANDLER(code) ((void (*)(int))(*(uint32_t *)0x0000001C))(code)
#define SOFT_FAULT_ASSERT_HANDLER(exp, file, line, func) ((void (*)(int, const char *, const char *, int, const char *))(*(uint32_t *)0x0000001C))(CO_FAULT_ASSERT_FAIL, exp, file, line, func)
/// @endcond

/*********************************************************************
 * TYPEDEFS
 */
/// Software fault code
typedef enum
{
    CO_FAULT_ASSERT_FAIL        = 0,  // General co_assert() fail
    CO_FAULT_PATCH_ERROR        = 1,  // Patch error
    CO_FAULT_HCI_ERROR          = 2,  // HCI error. The reason maybe: 1.the UART Priority is too low 2.the invalid HCI data
    CO_FAULT_HEAP_CRASH         = 3,  // Heap: crash
    CO_FAULT_HEAP_ERROR_POINTER = 4,  // Heap: fee invalid pointer
    CO_FAULT_HEAP_NO_SPACE      = 5,  // Heap: no space
    CO_FAULT_STACK_OVERFLOW     = 6,  // stack: overflow
    CO_FAULT_HCLK_CRASH         = 7,  // High frequency clock crash
    CO_FAULT_LCLK_CRASH         = 8,  // Low frequency clock crash
    CO_FAULT_BASEBAND_CRASH     = 9,  // Baseband crash
    CO_FAULT_CALIB_CRASH        = 10, // Calib crash
    CO_FAULT_SRAM_CRASH         = 11, // SRAM crash
}co_fault_code_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief set software fault
 *
 * @param[in] code  software fault code
 *
 * @return None
 **/
__STATIC_INLINE void co_fault_set(co_fault_code_t code)
{
    SOFT_FAULT_HANDLER((int)code);
}

/**
 * @brief  co fault assert set
 *
 * @param[in] exp  exp
 * @param[in] file  file
 * @param[in] func  func
 * @param[in] line  line
 **/
__STATIC_INLINE void co_fault_assert_set(const char *exp, const char *file, const char *func, int line)
{
    SOFT_FAULT_ASSERT_HANDLER(exp, file, line, func);
}


#ifdef __cplusplus
}
#endif

#endif

/** @} */

