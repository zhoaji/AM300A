/**
 ****************************************************************************************
 *
 * @file gnuarm/compiler.h
 *
 * @brief Definitions of compiler specific directives.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "hs66xx.h"

// Compliler (GCC or ARMCC)
#if defined ( __GNUC__ )
/*********************************
 * GCC compiler
 */

/// define the static keyword for this compiler
#define __STATIC static

/// define the force inlining attribute for this compiler
#undef __INLINE
#define __INLINE __STATIC_FORCEINLINE

/// define the IRQ handler attribute for this compiler
#define __IRQ __attribute__((__interrupt__("IRQ")))

/// define the BLE IRQ handler attribute for this compiler
#define __BTIRQ

/// define the BLE IRQ handler attribute for this compiler
#define __BLEIRQ

/// define the FIQ handler attribute for this compiler
#define __FIQ __attribute__((__interrupt__("FIQ")))

/// define size of an empty array (used to declare structure with an array size not defined)
#define __ARRAY_EMPTY

/// Function returns struct in registers (4 in rvds, var with gnuarm).
/// With Gnuarm, feature depends on command line options and
/// impacts ALL functions returning 2-words max structs
/// (check -freg-struct-return and -mabi=xxx)
#define __VIR

/// function has no side effect and return depends only on arguments
#define __PURE __attribute__((const))

/// Align instantiated lvalue or struct member on 4 bytes
#define __ALIGN4 __attribute__((aligned(4)))

/// __MODULE__ comes from the RVDS compiler that supports it
#define __MODULE__ __BASE_FILE__

/// Pack a structure field
#undef __PACKED
#define __PACKED __attribute__ ((__packed__))

/// Put a variable in a memory maintained during deep sleep
#define __LOWPOWER_SAVED

/// time-critical components in on-chip RAM
#define __ONCHIP_CODE__       __attribute__ ((section (".ramtext"), optimize(s), noinline))

/**
  \brief   Get Program Counter
  \details Returns the current value of the Program Counter (PC).
  \return               PC Register value
 */
__STATIC_FORCEINLINE uint32_t __get_PC(void)
{
  uint32_t result;

  __ASM volatile ("mov %0, pc\n\t" : "=r" (result) );
  return(result);
}


#elif defined ( __CC_ARM )
/*********************************
 * ARMCC compiler
 */

/// define the static keyword for this compiler
#define __STATIC static

/// define the force inlining attribute for this compiler
#undef __INLINE
#define __INLINE __forceinline static

/// define the IRQ handler attribute for this compiler
#define __IRQ __irq

/// define the BLE IRQ handler attribute for this compiler
#define __BTIRQ

/// define the BLE IRQ handler attribute for this compiler
#define __BLEIRQ

/// define the FIQ handler attribute for this compiler
#define __FIQ __irq

/// define size of an empty array (used to declare structure with an array size not defined)
#define __ARRAY_EMPTY 1

/// Put a variable in a memory maintained during deep sleep
#define __LOWPOWER_SAVED

#else

#error "File only included with RVDS or GCC!"

#endif

#endif // _COMPILER_H_
