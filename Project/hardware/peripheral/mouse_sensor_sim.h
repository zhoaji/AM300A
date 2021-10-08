/**
 * @file mouse_sensor_sim.h
 * @brief 
 * @date Wed 28 Jun 2017 11:04:25 AM CST
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief
 * @details 
 *
 * @{
 */

#ifndef __MOUSE_SENSOR_SIM_H__
#define __MOUSE_SENSOR_SIM_H__

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

typedef void (*ms_sim_event_callback_t)(int8_t delta_x, int8_t delta_y);

typedef struct
{
    uint8_t sclk_pin;
    uint8_t sdio_pin;
    uint8_t motion_pin;
    uint8_t sclk_delay_us;
    ms_sim_event_callback_t callback;
}ms_sim_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief ms_sim_gpio_handler()
 *
 * @param[in] pin_mask  
 *
 * @return 
 **/
void ms_sim_gpio_handler(uint32_t pin_mask);

/**
 * @brief ms_init()
 *
 * @param[in] sclk  
 * @param[in] sdio  
 * @param[in] sclk_delay_us  
 *
 * @return 
 **/
void ms_sim_config(const ms_sim_config_t *config);

/**
 * @brief ms_read()
 *
 * @param[in] addr  
 *
 * @return 
 **/
uint8_t ms_sim_read(uint8_t addr);

/**
 * @brief ms_write()
 *
 * @param[in] addr  
 * @param[in] data  
 *
 * @return 
 **/
void ms_sim_write(uint8_t addr, uint8_t data);

/**
 * @brief ms_read_id1()
 *
 * @return 0x30
 **/
uint8_t ms_sim_read_id1(void);

/**
 * @brief ms_read_id2()
 *
 * @return 0x5x
 **/
uint8_t ms_sim_read_id2(void);

/**
 * @brief ms_sim_read_status()
 *
 * @return 
 **/
uint8_t ms_sim_read_status(void);

/**
 * @brief ms_sim_read_delta_x()
 *
 * @return 
 **/
int8_t ms_sim_read_delta_x(void);

/**
 * @brief ms_sim_read_delta_y()
 *
 * @return 
 **/
int8_t ms_sim_read_delta_y(void);

/**
 * @brief ms_sim_read_config()
 *
 * @return 
 **/
uint8_t ms_sim_read_config(void);

/**
 * @brief ms_sim_is_present()
 *
 * @return 
 **/
bool ms_sim_is_present(void);

/**
 * @brief ms_sim_is_motion()
 *
 * @return 
 **/
bool ms_sim_is_motion(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

