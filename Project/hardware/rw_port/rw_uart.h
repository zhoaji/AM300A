/**
 * @file rw_uart.h
 * @brief 
 * @date Thu, Aug 30, 2018  2:49:21 PM
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief 
 * @details 
 *
 * @{
 */

#ifndef __RW_UART_H__
#define __RW_UART_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "co.h"
#include "peripheral.h"
#include "rwip.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint8_t *tx_buf;
    unsigned tx_size;
    rwip_eif_callback tx_callback;
    void *tx_dummy;

    uint8_t *rx_buf;
    unsigned rx_size;
    rwip_eif_callback rx_callback;
    void *rx_dummy;
    co_fifo_t rx_fifo;
}rw_hci_uart_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 *************************************************************************************
 * @brief Starts a data reception.
 *
 * @param[out] bufptr      Pointer to the RX buffer
 * @param[in]  size        Size of the expected reception
 * @param[in]  callback    Pointer to the function called back when transfer finished
 * @param[in]  dummy       Dummy data pointer returned to callback when reception is finished
 *************************************************************************************
 */
void rw_uart_read(uint8_t *bufptr, uint32_t size, rwip_eif_callback callback, void* dummy);

/**
 *************************************************************************************
 * @brief Starts a data transmission.
 *
 * @param[in]  bufptr      Pointer to the TX buffer
 * @param[in]  size        Size of the transmission
 * @param[in]  callback    Pointer to the function called back when transfer finished
 * @param[in]  dummy       Dummy data pointer returned to callback when transmission is finished
 *************************************************************************************
 */
void rw_uart_write(uint8_t *bufptr, uint32_t size, rwip_eif_callback callback, void* dummy);

/**
 *************************************************************************************
 * @brief Enable Interface flow.
 *************************************************************************************
 */
void rw_uart_flow_on(void);

/**
 *************************************************************************************
 * @brief Disable Interface flow.
 *
 * @return True if flow has been disabled, False else.
 *************************************************************************************
 */
bool rw_uart_flow_off(void);

/**
 *************************************************************************************
 * @brief uart open (HS_UART1)
 *
 * @return None
 *************************************************************************************
 */
void rw_uart_open(uint32_t baud_rate, rw_hci_uart_t *rw, uint8_t *pool, uint32_t pool_size);


#ifdef __cplusplus
}
#endif

#endif

/** @} */

