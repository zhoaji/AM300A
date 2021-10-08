/**
 * @file radio.h
 * @brief 
 * @date Tue, Sep 24, 2019  5:33:45 PM
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief 
 * @details 
 *
 * @{
 */

#ifndef __RADIO_H__
#define __RADIO_H__

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
#define RF_TX_POWER_NORMAL  0
#define RF_TX_POWER_MAX     7
#define RF_TX_POWER_MIN     -30


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
 * @brief  rf txrx pin enable
 *
 * @param[in] enable  enable
 * @param[in] tx_pin  tx pin
 * @param[in] rx_pin  rx pin
 * @param[in] pol  polarity, 0 or 1
 **/
void rf_txrx_pin_enable(bool enable, int tx_pin, int rx_pin, int pol);

/**
 * @brief  rf carrier enable
 *
 * @param[in] enable  enable
 * @param[in] freq  2402MHz ... 2480MHz
 **/
void rf_carrier_enable(bool enable, uint32_t freq);

/**
 * @brief  rf full rx enable
 *
 * @param[in] enable  enable
 * @param[in] freq  2402MHz ... 2480MHz
 **/
void rf_full_rx_enable(bool enable, uint32_t freq);

/**
 * @brief  rf tx power set
 *
 * @param[in] auto_ctrl  false: control by power param; true: auto control by STACK
 * @param[in] power  power
 **/
void rf_tx_power_set(bool auto_ctrl, int power);

#if (CONFIG_DIRECTION_FINDING)
/**
 * @brief  rf cte pattern set
 *
 * @param[in] ptn:    antena switch pattern
 * @param[in] ptn_len: antena switch pattern length
 * @param[in] smp_win :   switch antena frequency
 * @param[in] smp_point : sample point in 1/16us accurate in the 1us sample window
 *
 **/
void rf_set_cte(uint8_t* ptn, uint8_t ptn_len, uint8_t smp_win);
#endif  // (CONFIG_DIRECTION_FINDING)

#ifdef __cplusplus
/*{*/ }
#endif

#endif

/** @} */

