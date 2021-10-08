/**
 * @file timer.c
 * @brief 
 * @date Wed 31 May 2017 07:16:01 PM CST
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

#define TIM_NUM                 3

#define TIM_ARR_MAX             0xFFFF
#define TIM_PSC_MAX             0xFFFF

#define TIM_STRUCT2INDEX(tim)   ((((uint32_t)tim) - HS_TIM0_BASE) / (HS_TIM1_BASE - HS_TIM0_BASE))
#define TIM_INDEX2STRUCT(i)     ((HS_TIM_Type *)(HS_TIM0_BASE + i * (HS_TIM1_BASE - HS_TIM0_BASE)))

typedef void (*tim_callback_t)(void);

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    struct
    {
        tim_mode_t mode;
        tim_callback_t callback;
    }tim[TIM_NUM];
}tim_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static tim_env_t tim_env;


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief tim_enable_irq()
 *
 * @param[in] index  
 *
 * @return 
 **/
__STATIC_INLINE void tim_enable_irq(int index)
{
    IRQn_Type irqn = (IRQn_Type)((int)TIM0_IRQn + index);

    NVIC_SetPriority(irqn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(irqn);
    NVIC_EnableIRQ(irqn);
}

/**
 * @brief tim_timer_irq_handler()
 *
 * @param[in] tim  
 *
 * @return 
 **/
static void tim_timer_irq_handler(HS_TIM_Type *tim)
{
    int index = TIM_STRUCT2INDEX(tim);
    uint32_t status = tim->SR;

    // clear irq
    tim->SR = 0;

    if(status & TIM_SR_UIF)
    {
        if(tim_env.tim[index].callback)
            tim_env.tim[index].callback();
    }
}

/**
 * @brief tim_timer_calc_psc_arr()
 *
 * @param[in] tim  
 * @param[in] period_us  
 *
 * @return 
 **/
static bool tim_timer_calc_psc_arr(HS_TIM_Type *tim, uint32_t period_us)
{
    int index = TIM_STRUCT2INDEX(tim);
    uint32_t clock_mhz = cpm_get_clock((cpm_clk_t)((int)CPM_TIM0_CLK + index)) / 1000000;
    uint32_t psc, arr, factor;

    factor = period_us / (TIM_ARR_MAX+1);
    psc = clock_mhz * (factor + 1);
    arr = period_us / (factor + 1); // arr = (delay_us * clock_mhz) / (psc + 1) - 1

    if(psc>TIM_PSC_MAX || arr>TIM_ARR_MAX)
        return false;

    tim->PSC = psc-1;
    tim->ARR = arr-1;

    return true;
}

/**
 * @brief tim_timer_config()
 *
 * @param[in] tim  
 * @param[in] config  
 *
 * @return 
 **/
static bool tim_timer_config(HS_TIM_Type *tim, const tim_timer_config_t *config)
{
    int index = TIM_STRUCT2INDEX(tim);
    bool psc_arr;

    psc_arr = tim_timer_calc_psc_arr(tim, config->period_us);
    if(!psc_arr)
        return false;

    tim->CR1 = TIM_CR1_ARPE;
    tim->DIER = 0;
    tim->RCR = 0;
    tim->CNT = 0;
    tim->SR = 0;

    tim_env.tim[index].callback = (tim_callback_t)config->callback;
    if(config->callback)
    {
        tim_enable_irq(index);
        tim->DIER = TIM_DIER_UIE;
    }

    return true;
}

/**
 * @brief tim_pwm_config()
 *
 * @param[in] tim  
 * @param[in] config  
 *
 * @return 
 **/
static bool tim_pwm_config(HS_TIM_Type *tim, const tim_pwm_config_t *config)
{
    int index = TIM_STRUCT2INDEX(tim);
    uint32_t clock = cpm_get_clock((cpm_clk_t)((int)CPM_TIM0_CLK + index));
    uint32_t psc, i, ccer;

    /* Configures the peripheral.*/
    psc = (clock / config->count_freq) - 1;

    if(psc > TIM_PSC_MAX)
        return false;

    tim->DIER = 0;
    tim->CR1 = TIM_CR1_ARPE;

    /* All channels configured in PWM1 mode with preload enabled and will
       stay that way until the driver is stopped.*/
    tim->CCMR1 = _TIM_CCMR1_OC1M(6) | TIM_CCMR1_OC1PE | _TIM_CCMR1_OC2M(6) | TIM_CCMR1_OC2PE;
    tim->CCMR2 = _TIM_CCMR2_OC3M(6) | TIM_CCMR2_OC3PE | _TIM_CCMR2_OC4M(6) | TIM_CCMR2_OC4PE;

    tim->PSC = psc;
    tim->ARR = config->period_count-1;

    /* Output enables and polarities setup.*/
    ccer = 0;
    for(i=0; i<TIM_PWM_CHANNEL_NUM; ++i)
    {
        if(config->channel[i].enable)
        {
            ccer |= TIM_CCER_CCE(i);
            if(config->channel[i].config.pol)
                ccer |= TIM_CCER_CCP(i);
            tim->CCR[i] = config->channel[i].config.pulse_count;
        }
    }
    tim->CCER = ccer;

    tim->SR = 0;
    tim->EGR |= TIM_EGR_UG;
    while(!(tim->SR & TIM_SR_UIF));

    tim->CNT = 0;
    tim->CR2 = 0;
    tim->SR = 0;
    tim->BDTR = TIM_BDTR_MOE;

    tim_env.tim[index].callback = (tim_callback_t)config->callback;
    if(config->callback)
    {
        tim_enable_irq(index);
        tim->DIER = TIM_DIER_UIE;
    }

    return true;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief tim initialize
 *
 * @return None
 **/
void tim_init(void)
{
}

/**
 * @brief tim configure
 *
 * @param[in] tim  tim object
 * @param[in] config  configuration
 *
 * @return None
 **/
void tim_config(HS_TIM_Type *tim, const tim_config_t *config)
{
    uint32_t index = TIM_STRUCT2INDEX(tim);

    // Reset and Open clock
    register_set1(&HS_PSO->TIM_CFG[index], CPM_TIMER_SOFT_RESET_MASK);
    register_set0(&HS_PSO->TIM_CFG[index], CPM_TIMER_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    switch(config->mode)
    {
        case TIM_TIMER_MODE:
            tim_timer_config(tim, &config->config.timer);
            break;

        case TIM_PWM_MODE:
            tim_pwm_config(tim, &config->config.pwm);
            break;
    }
}

/**
 * @brief Change PWM period count
 *
 * @param[in] tim  PWM object
 * @param[in] period_count  new period count, MAX:65535
 *
 * @return None
 **/
void tim_pwm_change_period_count(HS_TIM_Type *tim, uint16_t period_count)
{
    tim->ARR = period_count;
}

/**
 * @brief Change PWM channel pulse count
 *
 * @param[in] tim  PWM object
 * @param[in] channel  PWM channel index
 * @param[in] pulse_count  new pulse count, MAX 65535, should be less than period_count
 *
 * @return None
 **/
void tim_pwm_channel_change_pulse_count(HS_TIM_Type *tim, tim_pwm_channel_t channel, uint16_t pulse_count)
{
    tim->CCR[channel] = pulse_count;
}

/**
 * @brief Force PWM channel output
 *
 * @param[in] tim  PMW object
 * @param[in] channel  PWM channel index
 * @param[in] level  force output level
 *
 * @return None
 **/
void tim_pwm_channel_force_output(HS_TIM_Type *tim, tim_pwm_channel_t channel, tim_pwm_force_level_t level)
{
    uint32_t ccxm = (uint32_t)level;
    uint32_t pol = tim->CCER & TIM_CCER_CCP(channel);

    if(level!=TIM_PWM_FORCE_DISABLE && pol)
        ccxm ^= 1u;

    switch(channel)
    {
        case TIM_PWM_CHANNEL_0:
            tim->CCMR1 = (tim->CCMR1 & ~TIM_CCMR1_OC1M) | _TIM_CCMR1_OC1M(ccxm);
            break;
        case TIM_PWM_CHANNEL_1:
            tim->CCMR1 = (tim->CCMR1 & ~TIM_CCMR1_OC2M) | _TIM_CCMR1_OC2M(ccxm);
            break;
        case TIM_PWM_CHANNEL_2:
            tim->CCMR2 = (tim->CCMR2 & ~TIM_CCMR2_OC3M) | _TIM_CCMR2_OC3M(ccxm);
            break;
        case TIM_PWM_CHANNEL_3:
            tim->CCMR2 = (tim->CCMR2 & ~TIM_CCMR2_OC4M) | _TIM_CCMR2_OC4M(ccxm);
            break;
        default:
            break;
    }
}

/**
 * @brief tim start
 *
 * After config tim (timer, PWM), call this function to start
 *
 * @param[in] tim  tim object
 *
 * @return None
 **/
void tim_start(HS_TIM_Type *tim)
{
    int index = TIM_STRUCT2INDEX(tim);

    // start
    tim->CR1 |= TIM_CR1_CEN;/*timer starts running*/

    // Prevent sleep
    pmu_lowpower_prevent(PMU_LP_TIM0<<index);
}

/**
 * @brief tim stop
 *
 * @param[in] tim  tim object
 *
 * @return None
 **/
void tim_stop(HS_TIM_Type *tim)
{
    int index = TIM_STRUCT2INDEX(tim);

    // start
    tim->CR1 &= ~TIM_CR1_CEN;/*timer starts running*/

    // Alow sleep
    pmu_lowpower_allow(PMU_LP_TIM0<<index);
}

void TIM0_IRQHandler(void)
{
    tim_timer_irq_handler(HS_TIM0);
}

void TIM1_IRQHandler(void)
{
    tim_timer_irq_handler(HS_TIM1);
}

void TIM2_IRQHandler(void)
{
    tim_timer_irq_handler(HS_TIM2);
}

#ifndef CONFIG_HS6621
void tim_dma_config(HS_TIM_Type *tim, const tim_dma_config_t *config)
{
    tim->DCR = ((config->len_in_bytes/4 - 1)<<8) | ((uint32_t)config->tim_addr-(uint32_t)tim)/4;
    tim->DIER = config->req;

    dma_dev_t dma_tim = {
        .id         = (dma_id_t)(TIMER0_DMA_ID + TIM_STRUCT2INDEX(tim)),
        .addr       = (void *)&tim->DMAR,
        .addr_ctrl  = DMA_ADDR_CTRL_FIX,
        .bus_width  = DMA_SLAVE_BUSWIDTH_32BITS,
        .burst_size = DMA_BURST_LEN_1UNITS,
    };
    
    dma_dev_t dma_mem = {
        .id         = MEM_DMA_ID,
        .addr       = config->mem_addr,
        .addr_ctrl  = DMA_ADDR_CTRL_INC,
        .bus_width  = DMA_SLAVE_BUSWIDTH_32BITS,
        .burst_size = DMA_BURST_LEN_1UNITS,
    };
    
    dma_block_config_t dmabc = {
        .src                 = config->mem_to_tim ? &dma_mem : &dma_tim,
        .dst                 = config->mem_to_tim ? &dma_tim : &dma_mem,
        .block_size_in_bytes = config->len_in_bytes,
        .priority            = 0,
        .flow_controller     = DMA_FLOW_CONTROLLER_USE_NONE,
        .intr_en             = false,
    };
        
    dma_build_block(config->block, &dmabc);
}
#endif


/** @} */


