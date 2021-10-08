/**
 * @file spi.h
 * @brief SPI Driver
 * @date Wed 31 May 2017 07:15:52 PM CST
 * @author liqiang
 *
 * @defgroup SPI SPI
 * @ingroup PERIPHERAL
 * @brief SPI driver
 * @details SPI Driver
 *
 * The Serial Peripheral Interface (SPI) bus is a synchronous serial communication
 * interface specification used for short distance communication, primarily in
 * embedded systems. The HS6620 integrate 2 SPI interfaces, they can work in either
 * master or slave mode and also support DMA or software mode to transfer data.
 *
 * The master or slave controller only support point to point connection by hardware,
 * that is, both the SPI interface has only one CS pin. The connection is shown below the figure:
 *
 * The SPI Interface provides much flexibility that can fit most SPI slave devices.
 * The polarity and phase of SCK can be both programmed and results in four combinations.
 * The CS to SCK delay, the SCK to NCS delay, and SCK period are also programmed.
 * The timing relationships of SPI Interface are illustrated below.
 *
 * @{
 *
 * @example example_spi.c
 * This is an example of how to use the spi
 *
 */

#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
/// SPI work mode
typedef enum
{
    SPI_MODE_MASTER = 0,
    SPI_MODE_SLAVE  = 1,
} spi_mode_t;

/// spi transfor mode
typedef enum
{
    /// Mode 0: CPOL=0 CPHA=0
    SPI_TRANS_MODE_0 = 0,
    /// Mode 1: CPOL=0 CPHA=1
    SPI_TRANS_MODE_1 = 1,
    /// Mode 2: CPOL=1 CPHA=0
    SPI_TRANS_MODE_2 = 2,
    /// Mode 3: CPOL=1 CPHA=1
    SPI_TRANS_MODE_3 = 3,
} spi_transmode_t;

/// @cond
// SPI DMA object
typedef struct
{
    HS_DMA_CH_Type *tx_dma;
    HS_DMA_CH_Type *rx_dma;
    dma_callback_t callback;
} spi_dma_t;
/// @endcond

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/**
 * @brief spi initialize
 *
 * @param[in] spi  spi object
 * @param[in] mode  work mode
 * @param[in] transmode  transfer mode
 * @param[in] speed  speed in Hz
 *
 * @return None
 **/
void spi_open(HS_SPI_Type *spi, spi_mode_t mode, spi_transmode_t transmode, uint32_t speed);

/**
 * @brief spi close
 *
 * @param[in] spi  spi object
 **/
void spi_close(HS_SPI_Type *spi);

/**
 * @brief spi master exchange data with block
 *
 * @param[in] spi  spi object
 * @param[in] txbuf  transmit data buffer, NULL: ignore transmit data
 * @param[in] rxbuf  receive data buffer, NULL: ignore received data
 * @param[in] len  transmit and receive buffer size
 *
 * @return None
 **/
void spi_master_exchange(HS_SPI_Type *spi, const uint8_t *txbuf, uint8_t *rxbuf, uint32_t len);

/**
 * @brief spi master cs to low
 *
 * @param[in] spi  spi object
 *
 * @return None
 **/
void spi_master_cs_low(HS_SPI_Type *spi);

/**
 * @brief spi master cs to high
 *
 * @param[in] spi  spi object
 *
 * @return None
 **/
void spi_master_cs_high(HS_SPI_Type *spi);

/**
 * @brief spi dma config
 *
 * @param[in] spi  
 * @param[in] dma  
 * @param[in] callback  DMA transfer complete callback
 *
 * @note
 * if callback==NULL, the spi_master_exchange_dma() will return until data transfer complete.
 * otherwise, the spi_master_exchange_dma() will return immediately, and the callback is called after data transfer complete.
 *
 * @return None
 **/
void spi_dma_config(HS_SPI_Type *spi, spi_dma_t *dma, dma_callback_t callback);

/**
 * @brief spi master exchange with dma
 *
 * @param[in] spi  HS_SPI0/HS_SPI1
 * @param[in] dma  create by spi_dma_config()
 * @param[in] txbuf  transmit data buffer, NULL: ignore transmit data (!!! must be 4bytes align !!!)
 * @param[in] rxbuf  receive data buffer, NULL: ignore received data (!!! must be 4bytes align !!!)
 * @param[in] len  exchange length
 *
 * @note function blocking mode see spi_dma_config()
 *
 * @return None
 **/
void spi_master_exchange_dma(HS_SPI_Type *spi, spi_dma_t *dma, const uint8_t *txbuf, uint8_t *rxbuf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

