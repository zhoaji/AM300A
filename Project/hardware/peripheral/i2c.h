/**
 * @file i2c.h
 * @brief i2c driver
 * @date Wed 31 May 2017 07:15:14 PM CST
 * @author liqiang
 *
 * @defgroup I2C I2C
 * @ingroup PERIPHERAL
 * @brief I2C driver
 * @details I2C driver
 *
 * The I2C is a master or slave interface. It supports 100, 400 and 800 KHz clock rates
 * for controlling EEPROM and etc. The I2C interface provides several data formats and
 * can fit various I2C peripherals. Sequential read and write are supported to improve
 * throughputs. The I2C support DMA operation for extra MCU free data transfer. The I2C
 * work as ether master or slave, but cannot change the working mode after configuration.
 *
 * @{
 *
 * @example example_i2c.c
 * This is an example of how to use the i2c
 *
 */

#ifndef __I2C_H__
#define __I2C_H__

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
/// I2C mode
typedef enum
{
    /// Master mode
    I2C_MODE_MASTER,
    /// Slave mode
    I2C_MODE_SLAVE,
    /// SMBUS device mode
    I2C_MODE_SMBUS_DEVICE,
    /// SMBUS host mode
    I2C_MODE_SMBUS_HOST,
} i2c_mode_t;


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief i2c initialize
 *
 * @param[in] mode  mode
 * @param[in] speed  speed
 *
 * @return None
 **/
void i2c_open(i2c_mode_t mode, uint32_t speed);

/**
 * @brief i2c close
 *
 * @return None
 **/
void i2c_close(void);

/**
 * @brief i2c master write data
 *
 * @param[in] addr  slave address
 * @param[in] tx_buf  transmit data buffer
 * @param[in] tx_len  transmit data length
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_write(uint16_t addr, uint8_t *tx_buf, uint32_t tx_len);

/**
 * @brief i2c master read data
 *
 * @param[in] addr  slave address
 * @param[in] rx_buf  receive data buffer
 * @param[in] rx_len  receive buffer length
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_read(uint16_t addr, uint8_t *rx_buf, uint32_t rx_len);

/**
 * @brief i2c master read memery (EEPROM)
 *
 * @param[in] addr  I2C address
 * @param[in] offset  memery offset
 * @param[in] alen  memery offset bytes
 * @param[in] rx_buf  receive data buffer
 * @param[in] rx_len  receive data lenght
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_read_mem(uint16_t addr, uint32_t offset, uint32_t alen, uint8_t *rx_buf, uint32_t rx_len);

/**
 * @brief i2c master write memery (EEPROM)
 *
 * @param[in] addr  I2C address
 * @param[in] offset  memery offset
 * @param[in] alen  memery offset bytes
 * @param[in] tx_buf  transmit data buffer
 * @param[in] tx_len  transmit data length
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_write_mem(uint16_t addr, uint32_t offset, uint32_t alen, uint8_t *tx_buf, uint32_t tx_len);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

