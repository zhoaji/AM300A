/**
 * @file i2c_sim.h
 * @brief 
 * @date 2017/11/19 23:02:18
 * @author liqiang
 *
 * @defgroup I2C_SIM I2C GPIO Simulation
 * @ingroup PERIPHERAL
 * @brief I2C GPIO simulation driver
 * @details I2C GPIO simulation driver
 *
 * @{
 *
 */

#ifndef __I2C_SIM_H__
#define __I2C_SIM_H__

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
/// I2C simulation config
typedef struct
{
    /// clock pin number
    uint8_t scl_pin;
    /// sda pin number
    uint8_t sda_pin;
    /// eche gpio simulate clock delay counter
    uint16_t delay_counter;
}i2c_sim_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief i2c config
 *
 * @param[in] config  
 **/
void i2c_sim_config(const i2c_sim_config_t *config);

/**
 * @brief i2c master write
 *
 * @param[in] config  config object @ref i2c_sim_config
 * @param[in] slave_addr  slave device address
 * @param[in] offset  memery offset
 * @param[in] offset_length  memery offset length
 * @param[in] write_data  write data
 * @param[in] write_length  write data length
 *
 * @return true: ok
 **/
bool i2c_sim_master_write_mem(const i2c_sim_config_t *config, uint8_t slave_addr,
                              uint32_t offset, uint32_t offset_length,
                              const uint8_t *write_data, uint32_t write_length);

/**
 * @brief i2c master read
 *
 * @param[in] config  config object @ref i2c_sim_config
 * @param[in] slave_addr  slave device address
 * @param[in] offset  memery offset
 * @param[in] offset_length  memery offset legnth
 * @param[in] read_data  read buffer
 * @param[in] read_length  read data length
 *
 * @return true: ok
 **/
bool i2c_sim_master_read_mem(const i2c_sim_config_t *config, uint8_t slave_addr,
                             uint32_t offset, uint32_t offset_length,
                             uint8_t *read_data, uint32_t read_length);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

