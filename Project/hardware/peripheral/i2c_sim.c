/**
 * @file i2c_sim.c
 * @brief 
 * @date 2017/11/16 16:14:26
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"
#include "i2c_sim.h"

/*********************************************************************
 * MACROS
 */
#ifndef CONFIG_GPIO1_ABSENT
#define I2C_SIM_SCL_OUTPUT()    do{\
                                          if(config->scl_pin >= 32)\
                                          {\
                                              gpio_set_direction_ex(BIT_MASK(config->scl_pin-32), GPIO_OUTPUT);\
                                          }\
                                          else\
                                          {\
                                              gpio_set_direction(BIT_MASK(config->scl_pin), GPIO_OUTPUT);\
                                          }\
                                      }while(0)
    
#define I2C_SIM_SCL_HIGH()      do{\
                                          if(config->scl_pin >= 32)\
                                          {\
                                              gpio_write_ex(BIT_MASK(config->scl_pin-32), GPIO_HIGH);\
                                          }\
                                          else\
                                          {\
                                              gpio_write(BIT_MASK(config->scl_pin), GPIO_HIGH);\
                                          }\
                                      }while(0)
    
#define I2C_SIM_SCL_LOW()      do{\
                                          if(config->scl_pin >= 32)\
                                          {\
                                              gpio_write_ex(BIT_MASK(config->scl_pin-32), GPIO_LOW);\
                                          }\
                                          else\
                                          {\
                                              gpio_write(BIT_MASK(config->scl_pin), GPIO_LOW);\
                                          }\
                                      }while(0)
    
#define I2C_SIM_SDA_INPUT()     do{\
                                          if(config->sda_pin >= 32)\
                                          {\
                                              gpio_set_direction_ex(BIT_MASK(config->sda_pin-32), GPIO_INPUT);\
                                          }\
                                          else\
                                          {\
                                              gpio_set_direction(BIT_MASK(config->sda_pin), GPIO_INPUT);\
                                          }\
                                      }while(0)
    
#define I2C_SIM_SDA_OUTPUT()     do{\
                                          if(config->sda_pin >= 32)\
                                          {\
                                              gpio_set_direction_ex(BIT_MASK(config->sda_pin-32), GPIO_OUTPUT);\
                                          }\
                                          else\
                                          {\
                                              gpio_set_direction(BIT_MASK(config->sda_pin), GPIO_OUTPUT);\
                                          }\
                                      }while(0)
    
#define I2C_SIM_SDA_HIGH()      do{\
                                          if(config->sda_pin >= 32)\
                                          {\
                                              gpio_write_ex(BIT_MASK(config->sda_pin-32), GPIO_HIGH);\
                                          }\
                                          else\
                                          {\
                                              gpio_write(BIT_MASK(config->sda_pin), GPIO_HIGH);\
                                          }\
                                      }while(0)
    
#define I2C_SIM_SDA_LOW()      do{\
                                          if(config->sda_pin >= 32)\
                                          {\
                                              gpio_write_ex(BIT_MASK(config->sda_pin-32), GPIO_LOW);\
                                          }\
                                          else\
                                          {\
                                              gpio_write(BIT_MASK(config->sda_pin), GPIO_LOW);\
                                          }\
                                      }while(0)
    
#define I2C_SIM_SDA_READ()      ((config->sda_pin>=32)?gpio_read_ex(BIT_MASK(config->sda_pin-32)):gpio_read(BIT_MASK(config->sda_pin)))
    
#else

#define I2C_SIM_SCL_PIN_MASK    (1u<<config->scl_pin)
#define I2C_SIM_SDA_PIN_MASK    (1u<<config->sda_pin)

#define I2C_SIM_SCL_OUTPUT()    do{gpio_set_direction(I2C_SIM_SCL_PIN_MASK, GPIO_OUTPUT);}while(0)
#define I2C_SIM_SCL_HIGH()      do{gpio_write(I2C_SIM_SCL_PIN_MASK, GPIO_HIGH);}while(0)
#define I2C_SIM_SCL_LOW()       do{gpio_write(I2C_SIM_SCL_PIN_MASK, GPIO_LOW);}while(0)

#define I2C_SIM_SDA_INPUT()     do{gpio_set_direction(I2C_SIM_SDA_PIN_MASK, GPIO_INPUT);}while(0)
#define I2C_SIM_SDA_OUTPUT()    do{gpio_set_direction(I2C_SIM_SDA_PIN_MASK, GPIO_OUTPUT);}while(0)
#define I2C_SIM_SDA_HIGH()      do{gpio_write(I2C_SIM_SDA_PIN_MASK, GPIO_HIGH);}while(0)
#define I2C_SIM_SDA_LOW()       do{gpio_write(I2C_SIM_SDA_PIN_MASK, GPIO_LOW);}while(0)

#define I2C_SIM_SDA_READ()      gpio_read(I2C_SIM_SDA_PIN_MASK)
#endif

#define I2C_SIM_DELAY()         do{int i; for(i=0;i<config->delay_counter;i++)__NOP();}while(0)

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief i2c_sim_start()
 *
 * @param[in] config  
 *
 * @return 
 **/
static void i2c_sim_start(const i2c_sim_config_t *config)
{
    I2C_SIM_SDA_HIGH();
    I2C_SIM_DELAY();

    I2C_SIM_SCL_HIGH();
    I2C_SIM_DELAY();

    I2C_SIM_SDA_LOW();
    I2C_SIM_DELAY();

    I2C_SIM_SCL_LOW();
    I2C_SIM_DELAY();
}

/**
 * @brief i2c_sim_stop()
 *
 * @param[in] config  
 *
 * @return 
 **/
static void i2c_sim_stop(const i2c_sim_config_t *config)
{
    I2C_SIM_SDA_LOW();
    I2C_SIM_DELAY();

    I2C_SIM_SCL_HIGH();
    I2C_SIM_DELAY();

    I2C_SIM_SDA_HIGH();
    I2C_SIM_DELAY();

}

/**
 * @brief i2c_sim_nack()
 *
 * @param[in] config  
 *
 * @return 
 **/
static void i2c_sim_nack(const i2c_sim_config_t *config)
{
    I2C_SIM_DELAY();
    I2C_SIM_SDA_HIGH();
    I2C_SIM_DELAY();

    I2C_SIM_SCL_HIGH();
    I2C_SIM_DELAY();

    I2C_SIM_SCL_LOW();
    I2C_SIM_DELAY();

    I2C_SIM_SDA_LOW();
    I2C_SIM_DELAY();
}

/**
 * @brief i2c_sim_ack()
 *
 * @param[in] config  
 *
 * @return 
 **/
static void i2c_sim_ack(const i2c_sim_config_t *config)
{
    I2C_SIM_DELAY();
    I2C_SIM_SDA_LOW();
    I2C_SIM_DELAY();

    I2C_SIM_SCL_HIGH();
    I2C_SIM_DELAY();

    I2C_SIM_SCL_LOW();
    I2C_SIM_DELAY();

    I2C_SIM_SDA_LOW();
    I2C_SIM_DELAY();
}

/**
 * @brief i2c_sim_write()
 *
 * @param[in] config  
 * @param[in] data  
 *
 * @return 
 **/
static bool i2c_sim_write(const i2c_sim_config_t *config, uint8_t data)
{
    int j;
    bool result;

    for(j=0; j<8; ++j)
    {
        if(data & (0x80u>>j))
            I2C_SIM_SDA_HIGH();
        else
            I2C_SIM_SDA_LOW();

        I2C_SIM_DELAY();
        I2C_SIM_SCL_HIGH();
        I2C_SIM_DELAY();
        I2C_SIM_SCL_LOW();
        I2C_SIM_DELAY();
    }
    I2C_SIM_DELAY();

    I2C_SIM_SDA_INPUT();
    I2C_SIM_DELAY();
    I2C_SIM_SCL_HIGH();
    I2C_SIM_DELAY();
    I2C_SIM_DELAY();
    I2C_SIM_DELAY();

    if(I2C_SIM_SDA_READ())
        result = false;
    else
        result = true;

    I2C_SIM_SCL_LOW();
    I2C_SIM_DELAY();

    I2C_SIM_SDA_OUTPUT();

    return result;
}

/**
 * @brief i2c_sim_read()
 *
 * @param[in] config  
 *
 * @return 
 **/
static uint8_t i2c_sim_read(const i2c_sim_config_t *config)
{
    uint8_t data = 0;
    int j;

    I2C_SIM_SDA_INPUT();

    for(j=0; j<8; ++j)
    {
        I2C_SIM_DELAY();
        I2C_SIM_SCL_HIGH();
        I2C_SIM_DELAY();
        I2C_SIM_DELAY();
        I2C_SIM_DELAY();
        if(I2C_SIM_SDA_READ())
            data |= 0x80u>>j;
        I2C_SIM_SCL_LOW();
    }

    I2C_SIM_DELAY();
    I2C_SIM_SDA_OUTPUT();

    return data;
}


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief i2c_sim_config()
 *
 * @param[in] config  
 *
 * @return 
 **/
void i2c_sim_config(const i2c_sim_config_t *config)
{
    // Must be set LEVEL firstly, and then set the direction

    I2C_SIM_SDA_HIGH();
    I2C_SIM_SDA_OUTPUT();

    I2C_SIM_SCL_HIGH();
    I2C_SIM_SCL_OUTPUT();
}

/**
 * @brief i2c_sim_master_write_mem()
 *
 * @param[in] config  
 * @param[in] slave_addr  
 * @param[in] offset  
 * @param[in] offset_length  
 * @param[in] write_data  
 * @param[in] write_length  
 *
 * @return 
 **/
bool i2c_sim_master_write_mem(const i2c_sim_config_t *config, uint8_t slave_addr,
                              uint32_t offset, uint32_t offset_length,
                              const uint8_t *write_data, uint32_t write_length)
{
    bool result = false;
    uint8_t *poffset = (uint8_t *)&offset;

    co_assert(write_data != NULL);

    // Start
    i2c_sim_start(config);

    // Write address (WRITE flag)
    if(!i2c_sim_write(config, (slave_addr << 1) & 0xFE))
        goto I2C_STOP;

    // Write offset
    while(offset_length--)
    {
        I2C_SIM_DELAY();
        if(!i2c_sim_write(config, *poffset++))
            goto I2C_STOP;
    }

    // Write data
    while(write_length--)
    {
        I2C_SIM_DELAY();
        if(!i2c_sim_write(config, *write_data++))
            goto I2C_STOP;
    }

    result = true;

I2C_STOP:

    // Stop
    i2c_sim_stop(config);

    return result;
}

/**
 * @brief i2c_sim_master_read_mem()
 *
 * @param[in] config  
 * @param[in] slave_addr  
 * @param[in] offset  
 * @param[in] offset_length  
 * @param[in] read_data  
 * @param[in] read_length  
 *
 * @return 
 **/
bool i2c_sim_master_read_mem(const i2c_sim_config_t *config, uint8_t slave_addr,
                             uint32_t offset, uint32_t offset_length,
                             uint8_t *read_data, uint32_t read_length)
{
    bool result = false;
    uint8_t *poffset = (uint8_t *)&offset;

    co_assert(read_data != NULL);

    // Start
    i2c_sim_start(config);

    // Wirte address (WRITE flag)
    if(!i2c_sim_write(config, (slave_addr<<1) & 0xFE))
        goto I2C_STOP;

    // Write offset
    while(offset_length--)
    {
        I2C_SIM_DELAY();
        if(!i2c_sim_write(config, *poffset++))
            goto I2C_STOP;
    }

    //i2c_sim_stop(config);

    I2C_SIM_DELAY();

    // restart
    i2c_sim_start(config);

    // Write address (READ flag)
    if(!i2c_sim_write(config, (slave_addr<<1) | 0x01))
        goto I2C_STOP;

    // Read data
    while(read_length--)
    {
        *read_data++ = i2c_sim_read(config);
        if(read_length)
            i2c_sim_ack(config);
    }
    i2c_sim_nack(config);

    result = true;

I2C_STOP:

    // Stop
    i2c_sim_stop(config);

    return result;
}

/** @} */

