/**
 * @file hib.h
 * @brief 
 * @date Thu, Jan  9, 2020  5:34:35 PM
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief 
 * @details 
 *
 * @{
 */

#ifndef __HIB_H__
#define __HIB_H__

#ifdef __cplusplus
extern "C"
{ /*}*/
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */
#define HREGW   hib_register_spi_write
#define HREGWA  hib_register_spi_write_raw
#define HREGW1  hib_register_spi_write1
#define HREGW0  hib_register_spi_write0
#define HREGR   hib_register_spi_read
#define HREGRA  hib_register_spi_read_raw

// only pin0-pin12
#define HIB_WAKEUP_PIN_MASK 0x00001FFF

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * PUBLIC FUNCTIONS (base REG read/write)
 */

/**
 * @brief  hib register spi read raw
 *
 * @param[in] addr  addr
 *
 * @return
 **/
uint32_t hib_register_spi_read_raw(volatile uint8_t *reg);

/**
 * @brief  hib register spi write raw
 *
 * @param[in] addr  addr
 * @param[in] wdata  wdata
 **/
void hib_register_spi_write_raw(volatile uint8_t *reg, uint32_t wdata);

/**
 * @brief  hib register spi write
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 * @param[in] value  value
 **/
void hib_register_spi_write(volatile uint8_t *reg, uint32_t mask, uint32_t value);

/**
 * @brief  hib register spi write0
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 **/
void hib_register_spi_write0(volatile uint8_t *reg, uint32_t mask);

/**
 * @brief  hib register spi write1
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 **/
void hib_register_spi_write1(volatile uint8_t *reg, uint32_t mask);

/**
 * @brief  hib register spi read
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 * @param[in] pos  pos
 *
 * @return
 **/
uint32_t hib_register_spi_read(volatile uint8_t *reg, uint32_t mask, uint32_t pos);


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief  hib wakeup pin set
 *
 * @param[in] pin_mask  pin mask (only for PIN0~PIN12)
 * @param[in] trigger_type  trigger type
 **/
void hib_wakeup_pin_set(uint32_t pin_mask, pmu_pin_wakeup_type_t trigger_type);

/**
 * @brief  hib wakeup pin get
 *
 * @return wakeup pin mask
 **/
uint32_t hib_wakeup_pin_get(void);

/**
 * @brief  hib timer start
 *
 * @param[in] delay_32k  delay 32k
 **/
void hib_timer_start(uint32_t delay_32k);

/**
 * @brief  hib timer stop
 **/
void hib_timer_stop(void);

/**
 * @brief  hib wakeup pin get
 **/
uint32_t hib_is_pin_wakeup(void);

/**
 * @brief  hib is timer wakeup
 *
 * @return is timer
 **/
bool hib_is_timer_wakeup(void);

#ifdef __cplusplus
/*{*/ }
#endif

#endif

/** @} */

