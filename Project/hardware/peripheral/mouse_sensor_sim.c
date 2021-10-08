/**
 * @file mouse_sensor.c
 * @brief 
 * @date Mon 26 Jun 2017 03:35:15 PM CST
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
#include "mouse_sensor_sim.h"
#include "co.h"

/*********************************************************************
 * MACROS
 */

// Sensor Register
#define MS_SIM_REG_PRODUCT_ID1        0x00
#define MS_SIM_REG_PRODUCT_ID2        0x01
#define MS_SIM_REG_MOTION_STATUS      0x02
#define MS_SIM_REG_DELTA_X            0x03
#define MS_SIM_REG_DELTA_Y            0x04
#define MS_SIM_REG_OPERATION_MODE     0x05
#define MS_SIM_REG_CONFIGURATION      0x06
#define MS_SIM_REG_IMAGE_QUALITY      0x07
#define MS_SIM_REG_OPERATION_STATE    0x08
#define MS_SIM_REG_WRITE_PROTECT      0x09
#define MS_SIM_REG_SLEEP1_SETTING     0x0A
#define MS_SIM_REG_ENTER_TIME         0x0B
#define MS_SIM_REG_SLEEP2_SETTING     0x0C
#define MS_SIM_REG_THRESHOLD          0x0D
#define MS_SIM_REG_RECOGNITION        0x0E

// MS_SIM_REG_MOTION_STATUS: motion bit
#define MS_SIM_MOTION_STATUS_MASK     0x80

// Timer delay
#define MS_SIM_TIMER_DLEAY            8

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    ms_sim_config_t config;
    bool is_scanning;
    uint8_t no_motion_counter;
    co_timer_t timer;
}ms_sim_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static ms_sim_env_t ms_sim_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief ms_sim_sdio_set_direction()
 *
 * @param[in] dir  
 *
 * @return 
 **/
__STATIC_INLINE void ms_sim_sdio_set_direction(gpio_direction_t dir)
{
    gpio_set_direction(1<<ms_sim_env.config.sdio_pin, dir);
}

/**
 * @brief ms_sim_sdio_write()
 *
 * @param[in] level  
 *
 * @return 
 **/
__STATIC_INLINE void ms_sim_sdio_write(gpio_level_t level)
{
    gpio_write(1<<ms_sim_env.config.sdio_pin, level);
}

/**
 * @brief ms_sim_sdio_read()
 *
 * @return 
 **/
__STATIC_INLINE gpio_level_t ms_sim_sdio_read(void)
{
    return gpio_read(1<<ms_sim_env.config.sdio_pin) ? GPIO_HIGH : GPIO_LOW;
}

/**
 * @brief ms_sim_sclk_set_ouput()
 *
 * @return 
 **/
__STATIC_INLINE void ms_sim_sclk_set_ouput(void)
{
    gpio_set_direction(1<<ms_sim_env.config.sclk_pin, GPIO_OUTPUT);

}

/**
 * @brief ms_sim_sclk_write()
 *
 * @param[in] level  
 *
 * @return 
 **/
__STATIC_INLINE void ms_sim_sclk_write(gpio_level_t level)
{
    gpio_write(1<<ms_sim_env.config.sclk_pin, level);
}

/**
 * @brief ms_sim_out()
 *
 * @param[in] value  
 *
 * @return 
 **/
static void ms_sim_out(uint8_t value)
{
    int i;
    gpio_level_t sdio_bit;

    ms_sim_sdio_set_direction(GPIO_OUTPUT);

    for(i=0; i<8; ++i)
    {
        sdio_bit = (value & (1u<<(7-i))) ? GPIO_HIGH : GPIO_LOW;

        ms_sim_sclk_write(GPIO_LOW);
        co_delay_us(ms_sim_env.config.sclk_delay_us);

        ms_sim_sdio_write(sdio_bit);

        ms_sim_sclk_write(GPIO_HIGH);
        co_delay_us(ms_sim_env.config.sclk_delay_us);
    }

    ms_sim_sdio_set_direction(GPIO_INPUT);
}

/**
 * @brief ms_sim_in()
 *
 * @return 
 **/
static uint8_t ms_sim_in(void)
{
    uint8_t value = 0;;
    int i;

    for(i=0; i<8; ++i)
    {
        ms_sim_sclk_write(GPIO_LOW);
        co_delay_us(ms_sim_env.config.sclk_delay_us);

        ms_sim_sclk_write(GPIO_HIGH);
        co_delay_us(ms_sim_env.config.sclk_delay_us);

        value |= ((ms_sim_sdio_read()==GPIO_HIGH) ? 1 : 0)<<(7-i);
    }

    return value;
}

/**
 * @brief ms_sim_motion_clear()
 *
 * @return 
 **/
static void ms_sim_motion_clear(void)
{
    uint32_t motion_mask = 1u << ms_sim_env.config.motion_pin;

    // LOW level (Mot0Swk1 must be zero)
    while(gpio_read(motion_mask) == 0)
    {
        if(ms_sim_is_motion())
        {
            ms_sim_read_delta_x();
            ms_sim_read_delta_y();
        }
    }
}

/**
 * @brief ms_sim_timer_handler()
 *
 * @param[in] id  
 * @param[in] param  
 *
 * @return 
 **/
static void ms_sim_timer_handler(co_timer_t *timer, void *param)
{
    int8_t x_delta;
    int8_t y_delta;
    uint32_t motion_mask = 1u << ms_sim_env.config.motion_pin;

    // LOW level (Mot0Swk1 must be zero)
    if(gpio_read(motion_mask))
    {
        ++ms_sim_env.no_motion_counter;

        if(ms_sim_env.no_motion_counter > 10)
        {
            CO_DISABLE_IRQ();
            if(gpio_read(motion_mask))
            {
                co_timer_del(&ms_sim_env.timer);
                pmu_wakeup_pin_set(motion_mask, PMU_PIN_WAKEUP_LOW_LEVEL);
                ms_sim_env.no_motion_counter = 0;
                ms_sim_env.is_scanning = false;
            }
            CO_RESTORE_IRQ();
        }
    }
    else
    {
        ms_sim_env.no_motion_counter = 0;

        if(ms_sim_is_motion())
        {
            x_delta = ms_sim_read_delta_x();
            y_delta = ms_sim_read_delta_y();

            if((x_delta || y_delta) && ms_sim_env.config.callback)
                ms_sim_env.config.callback(x_delta, y_delta);
        }
    }
}

/*********************************************************************
 * PUBLIC FUNCTIONS (Basic function)
 */

/**
 * @brief ms_sim_gpio_handler()
 *
 * @param[in] pin_mask  
 *
 * @return 
 **/
void ms_sim_gpio_handler(uint32_t pin_mask)
{
    uint32_t motion_mask = 1u << ms_sim_env.config.motion_pin;

    if((pin_mask & motion_mask) && !ms_sim_env.is_scanning)
    {
        // LOW level (Mot0Swk1 must be zero)
        if(gpio_read(motion_mask) == 0)
        {
            co_timer_set(&ms_sim_env.timer, MS_SIM_TIMER_DLEAY,
                    TIMER_REPEAT, ms_sim_timer_handler, NULL);

            pmu_wakeup_pin_set(motion_mask, PMU_PIN_WAKEUP_DISABLE);
            ms_sim_env.is_scanning = true;
        }
    }
}

/**
 * @brief ms_sim_config()
 *
 * @param[in] config  
 *
 * @return 
 **/
void ms_sim_config(const ms_sim_config_t *config)
{
    uint32_t motion_mask = 1u << config->motion_pin;

    ms_sim_env.config = *config;
    ms_sim_env.is_scanning = false;
    ms_sim_env.no_motion_counter = 0;

    // IO
    ms_sim_sdio_set_direction(GPIO_INPUT);
    ms_sim_sclk_set_ouput();
    ms_sim_sclk_write(GPIO_HIGH);

    // Input
    gpio_set_direction(motion_mask, GPIO_INPUT);
    gpio_set_interrupt(motion_mask, GPIO_FALLING_EDGE);

    // wakeup
    pmu_wakeup_pin_set(motion_mask, PMU_PIN_WAKEUP_LOW_LEVEL);

    // clear motion
    ms_sim_motion_clear();
}

/**
 * @brief ms_sim_read()
 *
 * @param[in] addr  
 *
 * @return 
 **/
uint8_t ms_sim_read(uint8_t addr)
{
    ms_sim_out(addr & 0x7F);
    return ms_sim_in();
}

/**
 * @brief ms_sim_write()
 *
 * @param[in] addr  
 * @param[in] data  
 *
 * @return 
 **/
void ms_sim_write(uint8_t addr, uint8_t data)
{
    ms_sim_out(addr | 0x80);
    ms_sim_out(data);
}

/*********************************************************************
 * PUBLIC FUNCTIONS (Specified Fucntion)
 */

/**
 * @brief ms_read_id1()
 *
 * @return 0x30
 **/
uint8_t ms_sim_read_id1(void)
{
    return ms_sim_read(MS_SIM_REG_PRODUCT_ID1);
}

/**
 * @brief ms_read_id2()
 *
 * @return 0x5x
 **/
uint8_t ms_sim_read_id2(void)
{
    return ms_sim_read(MS_SIM_REG_PRODUCT_ID2);
}

/**
 * @brief ms_sim_read_status()
 *
 * @return 
 **/
uint8_t ms_sim_read_status(void)
{
    return ms_sim_read(MS_SIM_REG_MOTION_STATUS);
}

/**
 * @brief ms_sim_read_delta_x()
 *
 * @return 
 **/
int8_t ms_sim_read_delta_x(void)
{
    return (int8_t)ms_sim_read(MS_SIM_REG_DELTA_X);
}

/**
 * @brief ms_sim_read_delta_y()
 *
 * @return 
 **/
int8_t ms_sim_read_delta_y(void)
{
    return (int8_t)ms_sim_read(MS_SIM_REG_DELTA_Y);
}

/**
 * @brief ms_sim_read_config()
 *
 * @return 
 **/
uint8_t ms_sim_read_config(void)
{
    return ms_sim_read(MS_SIM_REG_CONFIGURATION);
}

/**
 * @brief ms_sim_is_present()
 *
 * @return 
 **/
bool ms_sim_is_present(void)
{
    uint8_t id1 = ms_sim_read_id1();
    uint8_t id2 = ms_sim_read_id2() & 0xF0;

    return id1==0x30 && id2==0x50;
}

/**
 * @brief ms_sim_is_motion()
 *
 * @return 
 **/
bool ms_sim_is_motion(void)
{
    uint8_t status = ms_sim_read_status();

    return status & MS_SIM_MOTION_STATUS_MASK ? true : false;
}

/** @} */


