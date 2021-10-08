/**
 * @file encoder_sim.c
 * @brief GPIO simulation for encoder driver
 * @date Thu 31 Aug 2017 03:14:17 PM CST
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
#include "encoder_sim.h"
#include "co.h"

/*********************************************************************
 * MACROS
 */
#define ENABLE_A_RISING_EDGE_COUNT
#define ENABLE_A_FALLING_EDGE_COUNT
//#define ENABLE_B_RISING_EDGE_COUNT
//#define ENABLE_B_FALLING_EDGE_COUNT

#define ENCODER_SIM_TIMER_DLEAY 8

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint32_t prev_level_mask;
    int counter;
    int pre_counter;
    uint8_t no_count_counter;
    bool is_scanning;

    encoder_sim_config_t config;

    co_timer_t timer;
}encoder_sim_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
encoder_sim_env_t es_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief encoder_sim_timer_handler()
 *
 * @param[in] id  
 * @param[in] param  
 *
 * @return 
 **/
 //   TYPE 1
 //        T1  T2
 //        !___!__       ______
 //     A _|      |_____|      |_
 // P         ______       ______
 //     B ___|      |_____|      |_ 
 //____________________________________________
 //           ______       ______
 //     A ___|      |_____|      |_
 // N       ______       ______
 //     B _|      |_____|      |_  
 
 
 //   TYPE 2
 //        T1  T2
 //        !___!__       ______
 //     A _|      |_____|      |_
 // P         ______       ______
 //     B ___|      |_____|      |_ 
 //____________________________________________
 //         ________       ______
 //     A _|        |_____|      |_
 // N       ______         _____
 //     B _|      |_______|     |_   
static void encoder_sim_timer_handler(co_timer_t *timer, void *param)
{
    if(es_env.pre_counter == es_env.counter)
    {
        ++es_env.no_count_counter;
        if(es_env.no_count_counter > 10)
        {
            pmu_lowpower_allow(PMU_LP_ENCODER2);

            co_timer_del(&es_env.timer);

            es_env.no_count_counter = 0;

            CO_DISABLE_IRQ();
            es_env.is_scanning = false;
            CO_RESTORE_IRQ();
        }
    }
    else
    {
        es_env.no_count_counter = 0;
        es_env.pre_counter = es_env.counter;
        if(es_env.config.callback)
            es_env.config.callback(es_env.counter);
    }
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief encoder gpio IRQ handler, PUT THIS TO GPIO IRQ HANDLER
 *
 * @param[in] pin_mask  pin mask
 *
 * @return None
 **/
void encoder_sim_gpio_handler(uint32_t pin_mask)
{
    uint32_t a_mask = 1u << es_env.config.a_pin;
    uint32_t b_mask = 1u << es_env.config.b_pin;
    uint32_t cur_level_mask = gpio_read(a_mask | b_mask);
    uint32_t a_level = cur_level_mask & a_mask;
    uint32_t b_level = cur_level_mask & b_mask;
    int save_counter = es_env.counter;

#if defined(ENABLE_A_RISING_EDGE_COUNT) || defined(ENABLE_A_FALLING_EDGE_COUNT)
    // A edge
    if((pin_mask & a_mask) && ((es_env.prev_level_mask&a_mask) != a_level))
    {
        // Rising edge
        if(a_level)
        {
#ifdef ENABLE_A_RISING_EDGE_COUNT
            if(b_level)
                --es_env.counter;
            else
                ++es_env.counter;
            pmu_wakeup_pin_set(a_mask, PMU_PIN_WAKEUP_LOW_LEVEL);
#endif
        }
        // Falling edge
        else
        {
#ifdef ENABLE_A_FALLING_EDGE_COUNT
            if(b_level)
                ++es_env.counter;
            else
                --es_env.counter;
            pmu_wakeup_pin_set(a_mask, PMU_PIN_WAKEUP_HIGH_LEVEL);
#endif
        }
    }
#endif

#if defined(ENABLE_B_RISING_EDGE_COUNT) || defined(ENABLE_B_FALLING_EDGE_COUNT)
    // B edge
    if((pin_mask & b_mask) && ((es_env.prev_level_mask&b_mask) != b_level))
    {
        // Rising edge
        if(b_level)
        {
#ifdef ENABLE_B_RISING_EDGE_COUNT
            if(a_level)
                ++es_env.counter;
            else
                --es_env.counter;
            pmu_wakeup_pin_set(b_mask, PMU_PIN_WAKEUP_LOW_LEVEL);
#endif
        }
        // Falling edge
        else
        {
#ifdef ENABLE_B_FALLING_EDGE_COUNT
            if(a_level)
                --es_env.counter;
            else
                ++es_env.counter;
            pmu_wakeup_pin_set(b_mask, PMU_PIN_WAKEUP_HIGH_LEVEL);
#endif
        }
    }
#endif

    // Prevent some jitter for no edge interruot.
    es_env.prev_level_mask = cur_level_mask;

    // Prevent sleep
    if(save_counter!=es_env.counter && !es_env.is_scanning)
    {
        pmu_lowpower_prevent(PMU_LP_ENCODER2);

        co_timer_set(&es_env.timer, ENCODER_SIM_TIMER_DLEAY,
                TIMER_REPEAT, encoder_sim_timer_handler, NULL);

        es_env.is_scanning = true;
    }
}

/**
 * @brief encoder_sim_config()
 *
 * @param[in] config  
 *
 * @return 
 **/
void encoder_sim_config(const encoder_sim_config_t *config)
{
    uint32_t a_mask, b_mask, cur_level, int_mask = 0;

    es_env.config = *config;
    es_env.prev_level_mask = 0;
    es_env.counter = 0;
    es_env.pre_counter = 0;
    es_env.no_count_counter = 0;
    es_env.is_scanning = false;

    // mask
    a_mask = 1u << es_env.config.a_pin;
    b_mask = 1u << es_env.config.b_pin;

#if defined(ENABLE_A_RISING_EDGE_COUNT) || defined(ENABLE_A_FALLING_EDGE_COUNT)
    int_mask |= a_mask;
#endif

#if defined(ENABLE_B_RISING_EDGE_COUNT) || defined(ENABLE_B_FALLING_EDGE_COUNT)
    int_mask |= b_mask;
#endif

    // Set input GPIO
    gpio_set_direction(a_mask | b_mask, GPIO_INPUT);
    gpio_set_interrupt(int_mask, GPIO_BOTH_EDGE);

    // Read current level
    cur_level = gpio_read(a_mask | b_mask);

    // Setup wakeup pin
#if defined(ENABLE_A_RISING_EDGE_COUNT) || defined(ENABLE_A_FALLING_EDGE_COUNT)
    if(cur_level & a_mask)
        pmu_wakeup_pin_set(a_mask, PMU_PIN_WAKEUP_LOW_LEVEL);
    else
        pmu_wakeup_pin_set(a_mask, PMU_PIN_WAKEUP_HIGH_LEVEL);
#endif

#if defined(ENABLE_B_RISING_EDGE_COUNT) || defined(ENABLE_B_FALLING_EDGE_COUNT)
    if(cur_level & b_mask)
        pmu_wakeup_pin_set(b_mask, PMU_PIN_WAKEUP_LOW_LEVEL);
    else
        pmu_wakeup_pin_set(b_mask, PMU_PIN_WAKEUP_HIGH_LEVEL);
#endif
}

/** @} */


