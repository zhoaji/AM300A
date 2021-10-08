/**
 * @file peripheral.h
 * @brief peripheral header
 * @date Thu 27 Nov 2014 04:49:22 PM CST
 * @author liqiang
 *
 * @defgroup PERIPHERAL Peripheral
 * @ingroup HS662X
 * @brief Peripheral drivers
 * @details
 *
 * The Peripheral Driver provides an interface of abstraction between the physical hardware and the application.
 * System-level software developers can use the driver to do the fast application software development,
 * instead of using the register level programming, which can reduce the total development time significantly.
 *
 * ### Features
 * - four channels DMA
 * - Two UART interface, one share with 7816 interface
 * - I2S interface
 * - Up to 40 bits general-purpose I/O GPIO
 * - Three I2C master or slave interface
 * - Two SPI master or slave interface
 * - Watchdog to prevent system dead lock
 * - RTC
 * - Three 32bit timers
 * - Keyboard controller
 * - Three way QDEC
 * - Eight single-end or differential-end 12bits GP-ADC
 * - AES HW encryption
 * - Audio ADC
 * - PPG Heart rate detection
 * - Touch sensor controller
 *
 * @{
 */

#ifndef __PERIPHERAL_H__
#define __PERIPHERAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "hs66xx.h"

/*********************************************************************
 * MACROS
 */

/// @cond

// PSO and PMU update and wait update ready
#define HS_PSO_STATUS_CLR() \
    do { \
        HS_PSO->REG_UPD = CPM_REG_UPD_STATUS_CLR_MASK; \
    }while(HS_PSO->REG_UPD)
#define HS_PSO_UPD_RDY() \
    do { \
        HS_PSO_STATUS_CLR(); \
        HS_PSO->REG_UPD = CPM_REG_UPD_CPU_MASK;\
        while((HS_PSO->REG_UPD & CPM_REG_UPD_CPU_STATUS_MASK) == 0); \
    }while(0)
#define HS_PSO_32K_UPD_RDY() \
    do { \
        HS_PSO_STATUS_CLR(); \
        HS_PSO->REG_UPD = CPM_REG_UPD_CPU_MASK|CPM_REG_UPD_32K_MASK;\
        while((HS_PSO->REG_UPD & (CPM_REG_UPD_CPU_STATUS_MASK|CPM_REG_UPD_32K_STATUS_MASK)) == 0); \
    }while(0)
#define HS_PSO_XTAL32M_UPD_RDY() \
    do { \
        HS_PSO_STATUS_CLR(); \
        HS_PSO->REG_UPD = CPM_REG_UPD_CPU_MASK|CPM_REG_UPD_XTAL32M_MASK;\
        while((HS_PSO->REG_UPD & (CPM_REG_UPD_CPU_STATUS_MASK|CPM_REG_UPD_XTAL32M_STATUS_MASK)) == 0); \
    }while(0)
#define HS_PMU_UPD_RDY() \
    do { \
        register_set1(&HS_PMU->BASIC, PMU_BASIC_APB_BUS_UPD_REG_MASK); \
        while(HS_PMU->BASIC & PMU_BASIC_APB_BUS_UPD_REG_MASK); \
        while(HS_PMU->STATUS_READ & PMU_APB_BUS_UPD_DATA_MASK); \
    }while(0) // 3 32kHz

/// @endcond

/*********************************************************************
 * INCLUDES
 */

#include "co.h"
#include "gpio.h"
#include "uart.h"
#include "cpm.h"
#include "pmu.h"
#include "sf_base.h"
#include "sf.h"
#include "sf_sys.h"
#include "calib.h"
#include "encoder.h"
#include "encoder_sim.h"
#include "rtc.h"
#include "wdt.h"
#ifdef CONFIG_HS6621
#include "dma.h"
#else
#include "dma_nds.h"
#endif
#include "adc_ex.h"
#include "i2c.h"
#include "i2c_sim.h"
#include "spi.h"
#include "i2s.h"
#include "kpp.h"
#include "kpp_sim.h"
#include "timer.h"
#include "pinmux.h"
#include "lcd_serial.h"
#include "radio.h"
#include "audio.h"

#ifndef CONFIG_HS6621
#include "hib.h"
#endif

/*********************************************************************
 * MACROS
 */

/// @cond

// Calibarate in controller_test.c with keil -O3 (24MHz), result is 3, set to 4 more safe
#define TIMEOUT_COEFFICIENT     (2*4) // x2 for 48MHz

#define WAIT_TIMEOUT_EX(event, us) \
    do{register uint32_t timeout; for(timeout=(us)*TIMEOUT_COEFFICIENT; timeout && !(event); --timeout){__NOP();}}while(0)

#if defined(CONFIG_SIMULATION) || defined(CONFIG_DEBUG)
#define WAIT_TIMEOUT(event, us) do{while(!(event)){}}while(0)
#else
// Wait read !!!ONE!!! register status.
#define WAIT_TIMEOUT(event, us) WAIT_TIMEOUT_EX(event, us)
#endif

/// bit mask
#define BITMASK(n)              (1u<<(n))
#define BIT_MASK(n)             (1u<<(n))

/// IS PPGA
#define IS_FPGA()               (HS_SYS->REV_ID == 0x5555)

/// @endcond

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
 * @brief peripheral lowpower store
 *
 * @return None
 **/
void peripheral_lowpower_enter(co_power_status_t power_status);

/**
 * @brief peripheral lowpower restore
 *
 * @return None
 **/
void peripheral_lowpower_leave(co_power_status_t power_status);

/// @endcond

#ifdef __cplusplus
}
#endif

#endif

/** @} */

