/*********************************************************************
 * @file gpio.c
 * @brief 
 * @version 1.0
 * @date Sun 14 Dec 2014 02:18:30 PM CST
 * @author liqiang
 *
 * @note 
 */

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"
#include "co.h"

/*********************************************************************
 * MACROS
 */
#ifdef __CC_ARM
#pragma Otime
#pragma O3
#endif

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
    gpio_callback_t callback;
    uint32_t pin_single_edge_flag;
    uint32_t pin_single_edge_level;

#ifndef CONFIG_GPIO1_ABSENT
    gpio_callback_t callback_ex;
    uint32_t pin_single_edge_flag_ex;
    uint32_t pin_single_edge_level_ex;
#endif

    struct
    {
        struct
        {
            struct
            {
                uint32_t DATAOUT;
                uint32_t OUTENSET;
                uint32_t INTENSET;
                uint32_t INTPOLSET;
                uint32_t INTTYPESET;
                uint32_t INTBOTHSET;
            }gpio0;
#ifndef CONFIG_GPIO1_ABSENT
            struct
            {
                uint8_t DATAOUT;
                uint8_t OUTENSET;
                uint8_t INTENSET;
                uint8_t INTPOLSET;
                uint8_t INTTYPESET;
                uint8_t INTBOTHSET;
            }gpio1;
#endif
        }store_reg;
    }lowpower;
}gpio_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static gpio_env_t gpio_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief gpio irq process function, for system calling
 *
 * @param[in] int_mask  gpio irq status
 * @param[in] level_mask
 *
 * @return None
 **/
static void gpio0_irq_process(uint32_t int_mask, uint32_t level_mask)
{
    // calculate unexpect single edge irq
    uint32_t cur_single_edge_mask = gpio_env.pin_single_edge_flag & int_mask;
    uint32_t cur_single_level_mask = level_mask & cur_single_edge_mask;
    uint32_t exp_single_level_mask = gpio_env.pin_single_edge_level & cur_single_edge_mask;
    uint32_t unexp_single_level_mask = cur_single_level_mask ^ exp_single_level_mask;

    // remove unexpect single edge irq
    int_mask &= ~unexp_single_level_mask;

    // notify
    if(int_mask && gpio_env.callback)
        gpio_env.callback(int_mask);

    // disable wakeup irq event
    pmu_pin_wakeup_out_of_date();
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio irq process function, for system calling
 *
 * @param[in] int_mask  gpio irq status
 * @param[in] level_mask
 *
 * @return None
 **/
static void gpio1_irq_process(uint32_t int_mask, uint32_t level_mask)
{
    // calculate unexpect single edge irq
    uint32_t cur_single_edge_mask = gpio_env.pin_single_edge_flag_ex & int_mask;
    uint32_t cur_single_level_mask = level_mask & cur_single_edge_mask;
    uint32_t exp_single_level_mask = gpio_env.pin_single_edge_level_ex & cur_single_edge_mask;
    uint32_t unexp_single_level_mask = cur_single_level_mask ^ exp_single_level_mask;

    // remove unexpect single edge irq
    int_mask &= ~unexp_single_level_mask;

    // notify
    if(int_mask && gpio_env.callback_ex)
        gpio_env.callback_ex(int_mask);

    // disable wakeup irq event
    pmu_pin_wakeup_out_of_date();
}
#endif

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief GPIO0_IRQHandler()
 *
 * @return 
 **/
void GPIO0_IRQHandler(void)
{
    uint32_t level_mask = HS_GPIO0->DATA;
    uint32_t int_status = HS_GPIO0->INTSTATUS;

    // clear
    HS_GPIO0->INTSTATUS = int_status;

    if(int_status)
        gpio0_irq_process(int_status, level_mask);
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief GPIO1_IRQHandler()
 *
 * @return 
 **/
void GPIO1_IRQHandler(void)
{
    uint32_t level_mask = HS_GPIO1->DATA;
    uint32_t int_status = HS_GPIO1->INTSTATUS;

    // clear
    HS_GPIO1->INTSTATUS = int_status;

    if(int_status)
        gpio1_irq_process(int_status, level_mask);
}
#endif

/**
 * @brief gpio initialize
 *
 * @return None
 **/
void gpio_open(void)
{
    gpio_env.callback = NULL;
    gpio_env.pin_single_edge_flag = 0;
    gpio_env.pin_single_edge_level = 0;
#ifndef CONFIG_GPIO1_ABSENT
    gpio_env.callback_ex = NULL;
    gpio_env.pin_single_edge_flag_ex = 0;
    gpio_env.pin_single_edge_level_ex = 0;
#endif

    // GPIO reset may be generate a invalid jitter when debuging-reset, so set all pin as input
    HS_GPIO0->OUTENCLR = 0xFFFFFFFF;
#ifndef CONFIG_GPIO1_ABSENT
    HS_GPIO1->OUTENCLR = 0xFFFFFFFF;
#endif

    // Reset and clock on
    register_set1(&HS_PSO->GPIO_CFG, CPM_GPIO_SOFT_RESET_MASK);
    register_set0(&HS_PSO->GPIO_CFG, CPM_GPIO_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
}

/**
 * @brief  gpio open only clock
 **/
void gpio_open_clock(void)
{
    register_set0(&HS_PSO->GPIO_CFG, CPM_GPIO_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
}

/**
 * @brief gpio set interrupt type
 *
 * @param[in] pin_mask  pin mask
 * @param[in] trigger_type  gpio trigger type
 *
 * @return None
 **/
void gpio_set_interrupt(uint32_t pin_mask, gpio_trigger_type_t trigger_type)
{
    /*
     * Single-Edge-Trigger is replaced as Both-Edge-Trigger.
     * Single-Edge-Trigger may cause the chip not go to sleep.
     * Software walkarround it.
     */

    CO_DISABLE_IRQ();

    switch(trigger_type)
    {
        case GPIO_FALLING_EDGE:
            HS_GPIO0->INTTYPESET = pin_mask;
            HS_GPIO0->INTBOTHSET = pin_mask;
            gpio_env.pin_single_edge_flag |= pin_mask;
            gpio_env.pin_single_edge_level &= ~pin_mask;
            break;

        case GPIO_RISING_EDGE:
            HS_GPIO0->INTTYPESET = pin_mask;
            HS_GPIO0->INTBOTHSET = pin_mask;
            gpio_env.pin_single_edge_flag |= pin_mask;
            gpio_env.pin_single_edge_level |= pin_mask;
            break;

        case GPIO_BOTH_EDGE:
            HS_GPIO0->INTTYPESET = pin_mask;
            HS_GPIO0->INTBOTHSET = pin_mask;
            gpio_env.pin_single_edge_flag &= ~pin_mask;
            break;

        case GPIO_LOW_LEVEL:
            HS_GPIO0->INTBOTHCLR = pin_mask;
            HS_GPIO0->INTTYPECLR = pin_mask;
            HS_GPIO0->INTPOLCLR  = pin_mask;
            gpio_env.pin_single_edge_flag &= ~pin_mask;
            break;

        case GPIO_HIGH_LEVEL:
            HS_GPIO0->INTBOTHCLR = pin_mask;
            HS_GPIO0->INTTYPECLR = pin_mask;
            HS_GPIO0->INTPOLSET  = pin_mask;
            gpio_env.pin_single_edge_flag &= ~pin_mask;
            break;

        default:
            break;
    }

    if(trigger_type == GPIO_TRIGGER_DISABLE)
    {
        HS_GPIO0->INTENCLR  = pin_mask;
        HS_GPIO0->INTSTATUS = pin_mask;
    }
    else
    {
        HS_GPIO0->INTSTATUS = pin_mask;
        HS_GPIO0->INTENSET  = pin_mask;
    }

    if(HS_GPIO0->INTENSET)
    {
        NVIC_ClearPendingIRQ(GPIO0_IRQn);
        NVIC_SetPriority(GPIO0_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(GPIO0_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(GPIO0_IRQn);
    }

    CO_RESTORE_IRQ();
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio set interrupt type
 *
 * @param[in] pin_mask  pin mask
 * @param[in] trigger_type  gpio trigger type
 *
 * @return None
 **/
void gpio_set_interrupt_ex(uint32_t pin_mask, gpio_trigger_type_t trigger_type)
{
    /*
     * Single-Edge-Trigger is replaced as Both-Edge-Trigger.
     * Single-Edge-Trigger may cause the chip not go to sleep.
     * Software walkarround it.
     */

    CO_DISABLE_IRQ();

    switch(trigger_type)
    {
        case GPIO_FALLING_EDGE:
            HS_GPIO1->INTTYPESET = pin_mask;
            HS_GPIO1->INTBOTHSET = pin_mask;
            gpio_env.pin_single_edge_flag_ex |= pin_mask;
            gpio_env.pin_single_edge_level_ex &= ~pin_mask;
            break;

        case GPIO_RISING_EDGE:
            HS_GPIO1->INTTYPESET = pin_mask;
            HS_GPIO1->INTBOTHSET = pin_mask;
            gpio_env.pin_single_edge_flag_ex |= pin_mask;
            gpio_env.pin_single_edge_level_ex |= pin_mask;
            break;

        case GPIO_BOTH_EDGE:
            HS_GPIO1->INTTYPESET = pin_mask;
            HS_GPIO1->INTBOTHSET = pin_mask;
            gpio_env.pin_single_edge_flag_ex &= ~pin_mask;
            break;

        case GPIO_LOW_LEVEL:
            HS_GPIO1->INTBOTHCLR = pin_mask;
            HS_GPIO1->INTTYPECLR = pin_mask;
            HS_GPIO1->INTPOLCLR  = pin_mask;
            gpio_env.pin_single_edge_flag_ex &= ~pin_mask;
            break;

        case GPIO_HIGH_LEVEL:
            HS_GPIO1->INTBOTHCLR = pin_mask;
            HS_GPIO1->INTTYPECLR = pin_mask;
            HS_GPIO1->INTPOLSET  = pin_mask;
            gpio_env.pin_single_edge_flag_ex &= ~pin_mask;
            break;

        default:
            break;
    }

    if(trigger_type == GPIO_TRIGGER_DISABLE)
    {
        HS_GPIO1->INTENCLR  = pin_mask;
        HS_GPIO1->INTSTATUS = pin_mask;
    }
    else
    {
        HS_GPIO1->INTSTATUS = pin_mask;
        HS_GPIO1->INTENSET  = pin_mask;
    }

    if(HS_GPIO1->INTENSET)
    {
        NVIC_ClearPendingIRQ(GPIO1_IRQn);
        NVIC_SetPriority(GPIO1_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(GPIO1_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(GPIO1_IRQn);
    }

    CO_RESTORE_IRQ();
}
#endif

/**
 * @brief gpio set interrupt callback
 *
 * @param[in] callback  gpio event callback
 *
 * @return None
 **/
void gpio_set_interrupt_callback(gpio_callback_t callback)
{
    gpio_env.callback = callback;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio set interrupt callback
 *
 * @param[in] callback  gpio event callback
 *
 * @return None
 **/
void gpio_set_interrupt_callback_ex(gpio_callback_t callback)
{
    gpio_env.callback_ex = callback;
}
#endif

/**
 * @brief gpio set direction
 *
 * @param[in] pin_mask  pin mask
 * @param[in] dir  gpio direction
 *
 * @return None
 **/
void gpio_set_direction(uint32_t pin_mask, gpio_direction_t dir)
{
    if(dir == GPIO_INPUT)
        HS_GPIO0->OUTENCLR = pin_mask;
    else
        HS_GPIO0->OUTENSET = pin_mask;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio set direction
 *
 * @param[in] pin_mask  pin mask
 * @param[in] dir  gpio direction
 *
 * @return None
 **/
void gpio_set_direction_ex(uint32_t pin_mask, gpio_direction_t dir)
{
    if(dir == GPIO_INPUT)
        HS_GPIO1->OUTENCLR = pin_mask;
    else
        HS_GPIO1->OUTENSET = pin_mask;
}
#endif

/**
 * @brief gpio read input value
 *
 * @param[in] pin_mask  pin mask
 *
 * @return gpio value with mask
 **/
uint32_t gpio_read(uint32_t pin_mask)
{
    return HS_GPIO0->DATA & pin_mask;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio read input value
 *
 * @param[in] pin_mask  pin mask
 *
 * @return gpio value with mask
 **/
uint32_t gpio_read_ex(uint32_t pin_mask)
{
    return HS_GPIO1->DATA & pin_mask;
}
#endif

/**
 * @brief gpio read current output status
 *
 * @param[in] pin_mask  pin mask
 *
 * @return gpio value with mask
 **/
uint32_t gpio_read_output_status(uint32_t pin_mask)
{
    return HS_GPIO0->DATAOUT & pin_mask;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio read current output status
 *
 * @param[in] pin_mask  pin mask
 *
 * @return gpio value with mask
 **/
uint32_t gpio_read_output_status_ex(uint32_t pin_mask)
{
    return HS_GPIO1->DATAOUT & pin_mask;
}
#endif

/**
 * @brief gpio write value
 *
 * @param[in] pin_mask  pin mask
 * @param[in] level  gpio value with mask, @ref gpio_level_t
 *
 * @return None
 **/
void gpio_write(uint32_t pin_mask, uint32_t level)
{
    uint8_t *p = (uint8_t *)&pin_mask;

    if(p[0]) HS_GPIO0->MASK_0_7  [p[0]] = level;
    if(p[1]) HS_GPIO0->MASK_8_15 [p[1]] = level;
    if(p[2]) HS_GPIO0->MASK_16_23[p[2]] = level;
    if(p[3]) HS_GPIO0->MASK_24_31[p[3]] = level;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio write value
 *
 * @param[in] pin_mask  pin mask
 * @param[in] level  gpio value with mask, @ref gpio_level_t
 *
 * @return None
 **/
void gpio_write_ex(uint32_t pin_mask, uint32_t level)
{
    uint8_t *p = (uint8_t *)&pin_mask;

    if(p[0]) HS_GPIO1->MASK_0_7  [p[0]] = level;
}
#endif

/**
 * @brief gpio toggle
 *
 * @param[in] pin_mask  pin mask
 *
 * @return None
 **/
void gpio_toggle(uint32_t pin_mask)
{
    gpio_write(pin_mask, gpio_read_output_status(pin_mask) ^ pin_mask);
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio toggle
 *
 * @param[in] pin_mask  pin mask
 *
 * @return None
 **/
void gpio_toggle_ex(uint32_t pin_mask)
{
    gpio_write_ex(pin_mask, gpio_read_output_status_ex(pin_mask) ^ pin_mask);
}
#endif

/**
 * @brief gpio_clear_interrupt_pending()
 *
 * @param[in] pin_mask  
 *
 * @return 
 **/
void gpio_clear_interrupt_pending(uint32_t pin_mask)
{
    HS_GPIO0->INTSTATUS = pin_mask;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief gpio_clear_interrupt_pending()
 *
 * @param[in] pin_mask  
 *
 * @return 
 **/
void gpio_clear_interrupt_pending_ex(uint32_t pin_mask)
{
    HS_GPIO1->INTSTATUS = pin_mask;
}
#endif

/**
 * @brief gpio_store()
 *
 * @return 
 **/
void gpio_store(void)
{
    if(HS_PSO->GPIO_CFG & CPM_GPIO_GATE_EN_MASK)
    {
        // Force open GPIO clock
        register_set0(&HS_PSO->GPIO_CFG, CPM_GPIO_GATE_EN_MASK);
        HS_PSO_UPD_RDY();
    }

    // store gpio status
    gpio_env.lowpower.store_reg.gpio0.DATAOUT    = HS_GPIO0->DATAOUT;
    gpio_env.lowpower.store_reg.gpio0.OUTENSET   = HS_GPIO0->OUTENSET;
    gpio_env.lowpower.store_reg.gpio0.INTENSET   = HS_GPIO0->INTENSET;
    gpio_env.lowpower.store_reg.gpio0.INTPOLSET  = HS_GPIO0->INTPOLSET;
    gpio_env.lowpower.store_reg.gpio0.INTTYPESET = HS_GPIO0->INTTYPESET;
    gpio_env.lowpower.store_reg.gpio0.INTBOTHSET = HS_GPIO0->INTBOTHSET;
#ifndef CONFIG_GPIO1_ABSENT
    gpio_env.lowpower.store_reg.gpio1.DATAOUT    = HS_GPIO1->DATAOUT;
    gpio_env.lowpower.store_reg.gpio1.OUTENSET   = HS_GPIO1->OUTENSET;
    gpio_env.lowpower.store_reg.gpio1.INTENSET   = HS_GPIO1->INTENSET;
    gpio_env.lowpower.store_reg.gpio1.INTPOLSET  = HS_GPIO1->INTPOLSET;
    gpio_env.lowpower.store_reg.gpio1.INTTYPESET = HS_GPIO1->INTTYPESET;
    gpio_env.lowpower.store_reg.gpio1.INTBOTHSET = HS_GPIO1->INTBOTHSET;
#endif

    // Disable not-wakeup-IO's IRQ
    HS_GPIO0->INTENCLR = ~pmu_wakeup_pin_get();
#ifndef CONFIG_GPIO1_ABSENT
    HS_GPIO1->INTENCLR = ~pmu_wakeup_pin_get_ex();
#endif
}

/**
 * @brief gpio_restore()
 *
 * @return 
 **/
void gpio_restore(void)
{
    if(HS_PSO->GPIO_CFG & CPM_GPIO_GATE_EN_MASK)
    {
        register_set1(&HS_PSO->GPIO_CFG, CPM_GPIO_SOFT_RESET_MASK);
        register_set0(&HS_PSO->GPIO_CFG, CPM_GPIO_GATE_EN_MASK);
        HS_PSO_UPD_RDY();

        // Output level
        HS_GPIO0->DATAOUT    = gpio_env.lowpower.store_reg.gpio0.DATAOUT;
        // Direction
        HS_GPIO0->OUTENSET   = gpio_env.lowpower.store_reg.gpio0.OUTENSET;
        // Interrupt
        HS_GPIO0->INTPOLSET  = gpio_env.lowpower.store_reg.gpio0.INTPOLSET;
        HS_GPIO0->INTTYPESET = gpio_env.lowpower.store_reg.gpio0.INTTYPESET;
        HS_GPIO0->INTBOTHSET = gpio_env.lowpower.store_reg.gpio0.INTBOTHSET;
        // Enable IRQ
        HS_GPIO0->INTENSET   = gpio_env.lowpower.store_reg.gpio0.INTENSET;

        // Enable CPU GPIO NVIC
        if(HS_GPIO0->INTENSET)
        {
            NVIC_SetPriority(GPIO0_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_EnableIRQ(GPIO0_IRQn);
        }

#ifndef CONFIG_GPIO1_ABSENT
        // Output level
        HS_GPIO1->DATAOUT    = gpio_env.lowpower.store_reg.gpio1.DATAOUT;
        // Direction
        HS_GPIO1->OUTENSET   = gpio_env.lowpower.store_reg.gpio1.OUTENSET;
        // Interrupt
        HS_GPIO1->INTPOLSET  = gpio_env.lowpower.store_reg.gpio1.INTPOLSET;
        HS_GPIO1->INTTYPESET = gpio_env.lowpower.store_reg.gpio1.INTTYPESET;
        HS_GPIO1->INTBOTHSET = gpio_env.lowpower.store_reg.gpio1.INTBOTHSET;
        // Enable IRQ
        HS_GPIO1->INTENSET   = gpio_env.lowpower.store_reg.gpio1.INTENSET;

        if(HS_GPIO1->INTENSET)
        {
            NVIC_SetPriority(GPIO1_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_EnableIRQ(GPIO1_IRQn);
        }
#endif

    }
    else
    {
        // Enable IRQ
        HS_GPIO0->INTENSET = gpio_env.lowpower.store_reg.gpio0.INTENSET;
#ifndef CONFIG_GPIO1_ABSENT
        HS_GPIO1->INTENSET = gpio_env.lowpower.store_reg.gpio1.INTENSET;
#endif
    }
}

