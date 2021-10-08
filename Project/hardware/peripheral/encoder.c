/**
 * @file encoder.c
 * @brief 
 * @date Fri 28 Apr 2017 11:11:34 AM CST
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

/*********************************************************************
 * MACROS
 */
#define ENCODER_STRUCT2INDEX(e)    ((((uint32_t)e) - HS_ENCODER0_BASE) / (HS_ENCODER1_BASE - HS_ENCODER0_BASE))


/*********************************************************************
 * TYPEDEFS
 */
//{BASE_COMPONENT_BEGIN
typedef struct
{
    uint32_t pin_mask;
    encoder_clk_t clk;
    encoder_event_callback_t callback;
}encoder_env_t;
//}BASE_COMPONENT_END

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
static encoder_env_t encoder_env = {0};
#else //}BASE_COMPONENT_END
extern encoder_env_t encoder_env;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void encoder_irq_handler(HS_ENCODER_Type *encoder)
{
    uint32_t of = encoder->OF & (ENCODER_OF_CNT_MOVE_MASK | ENCODER_OF_OVERFLOW_MASK | ENCODER_OF_UNDERFLOW_MASK);

    if (of)
    {
        // Clear irq
        encoder->OF = ENCODER_OF_CNT_MOVE_MASK | ENCODER_OF_OVERFLOW_MASK | ENCODER_OF_UNDERFLOW_MASK;

        if(of & ENCODER_OF_CNT_MOVE_MASK)
        {
            if(encoder_env.callback)
                encoder_env.callback(encoder, encoder->CNT);
        }

        // Wait clear finished
        while(encoder->OF & ENCODER_OF_CLR_MASK);
    }
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief encoder interrupt handler.
 *
 * @isr
 */
void QDEC_IRQHandler(void)
{
    encoder_irq_handler(HS_ENCODER0);
    encoder_irq_handler(HS_ENCODER1);
    encoder_irq_handler(HS_ENCODER2);
}

/**
 * @brief Encoder config
 *
 * @param[in] encoder  Encoder object
 * @param[in] config  configuration
 *
 * @return None
 **/
void encoder_config(HS_ENCODER_Type *encoder, const encoder_config_t *config)
{
    // Power on
    register_set1(&HS_PMU->BASEBAND_PM, PMU_PM_QDEC_POWER_ON_MASK);
    while(!(HS_PMU->BASEBAND_PM & PMU_PM_QDEC_POWER_STATUS_MASK));

    // Open clock
    register_set(&HS_PMU->MISC_CTRL_1, MASK_2REG(
            PMU_MISC_QDEC_CLK_EN, 1,
            PMU_MISC_QDEC_CLK_SEL, config->clk==ENCODER_CLK_32K? 0 : 1)); // 0:32K 1:CPU-CLK

    // Open clock
    register_set(&HS_PSO->QDEC_CFG, MASK_2REG(CPM_QDEC_DIV_SEL, 0,
                                              CPM_QDEC_GATE_EN, 0));
    HS_PSO_UPD_RDY();

    // reset QDEC
    register_set1(&HS_PMU->MISC_CTRL_1, PMU_MISC_QDEC_APB_SOFT_RESET_MASK|PMU_MISC_QDEC_SOFT_RESET_MASK);
    register_set0(&HS_PMU->MISC_CTRL_1, PMU_MISC_QDEC_APB_SOFT_RESET_MASK);

    // pin and default pinmux as input
    register_set(&encoder->PMUX, MASK_2REG(ENCODER_OF_A_SEL, config->a_pin,
                                           ENCODER_OF_B_SEL, config->b_pin));
//    pinmux_config(config->a_pin, PINMUX_QDEC_MODE_CFG);
//    pinmux_config(config->b_pin, PINMUX_QDEC_MODE_CFG);

    // enable clock
    register_set0(&HS_PMU->MISC_CTRL_1, PMU_MISC_QDEC_SOFT_RESET_MASK);

    // disable
    encoder->EN = 0;

    // enable sleep irq
    register_set(&HS_PMU->BASIC, MASK_1REG(PMU_BASIC_QDEC_INT_PMU_EN, (config->callback && (config->clk==ENCODER_CLK_32K)) ? 1 : 0));
    HS_PMU_UPD_RDY();

    // Auto reload
    encoder->ARR = config->counter_max;

    // Mode
    register_set(&encoder->MODE, MASK_3REG(ENCODER_MODE_SMS, config->mode,
                                           ENCODER_MODE_CC1P, config->pol_a,
                                           ENCODER_MODE_CC2P, config->pol_b));

    encoder_env.clk = config->clk;
    encoder_env.pin_mask |= (1u<<config->a_pin) | (1u<<config->b_pin);
    encoder_env.callback = config->callback;

    if(config->callback)
    {
        register_set1(&encoder->EN, ENCODER_EN_IEN_MOVE_MASK);
        NVIC_ClearPendingIRQ(QDEC_IRQn);
        NVIC_SetPriority(QDEC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(QDEC_IRQn);
    }
}

/**
 * @brief Encoder start
 *
 * @param[in] encoder  Encoder object
 *
 * @return None
 **/
void encoder_start(HS_ENCODER_Type *encoder)
{
    int index = ENCODER_STRUCT2INDEX(encoder);

    // enable
    register_set1(&encoder->EN, ENCODER_EN_CEN_MASK);

    if (encoder_env.clk != ENCODER_CLK_32K)
        pmu_lowpower_prevent(PMU_LP_ENCODER0<<index);
}

/**
 * @brief Encoder stop
 *
 * @param[in] encoder  Encoder object
 *
 * @return None
 **/
void encoder_stop(HS_ENCODER_Type *encoder)
{
    int index = ENCODER_STRUCT2INDEX(encoder);

    // disable
    register_set0(&encoder->EN, ENCODER_EN_CEN_MASK);

    if (encoder_env.clk != ENCODER_CLK_32K)
        pmu_lowpower_allow(PMU_LP_ENCODER0<<index);
}

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief encoder_pin_mask()
 *
 * @return 
 **/
uint32_t encoder_pin_mask(void)
{
    return encoder_env.pin_mask;
}

/**
 * @brief encoder_restore()
 *
 * @return 
 **/
void encoder_restore(void)
{
    if(encoder_env.callback && encoder_env.clk == ENCODER_CLK_32K)
    {
        register_set(&HS_PSO->QDEC_CFG, MASK_2REG(CPM_QDEC_DIV_SEL, 0,
                                                  CPM_QDEC_GATE_EN, 0));
        HS_PSO_UPD_RDY();

        NVIC_SetPriority(QDEC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(QDEC_IRQn);
    }
}
#endif //}BASE_COMPONENT_END

/** @} */


