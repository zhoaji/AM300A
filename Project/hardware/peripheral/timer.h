/**
 * @file timer.h
 * @brief timer driver
 * @date Wed 31 May 2017 07:16:05 PM CST
 * @author liqiang
 *
 * @defgroup Timer Timer
 * @ingroup PERIPHERAL
 * @brief timer/PWM driver
 * @details Timer driver
 *
 * The Timer includes three identical 32-bit Timer Counter channels.
 * Each channel can be independently programmed to perform a wide
 * range of functions including frequency measurement, event counting,
 * interval measurement, pulse generation, delay timing and pulse
 * width modulation. Each channel drives an internal interrupt signal
 * which can be programmed to generate processor interrupts.
 *
 * @{
 *
 * @example example_timer.c
 * This is an example of how to use the timer
 *
 * @example example_pwm.c
 * This is an example of how to use the pwm
 *
 */

#ifndef __TIMER_H__
#define __TIMER_H__

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

/// timer event callback
typedef void (*tim_timer_callback_t)(void);

/// PWM event callback
typedef void (*tim_pwm_callback_t)(void);

/// TIM DMA
typedef enum
{
    TIM_DMA_BURST_LEN_1UNIT = 0,
    TIM_DMA_BURST_LEN_2UNIT,
    TIM_DMA_BURST_LEN_3UNIT,
    TIM_DMA_BURST_LEN_4UNIT,
    TIM_DMA_BURST_LEN_5UNIT,
    TIM_DMA_BURST_LEN_6UNIT,
    TIM_DMA_BURST_LEN_7UNIT,
    TIM_DMA_BURST_LEN_8UNIT,
    TIM_DMA_BURST_LEN_9UNIT,
    TIM_DMA_BURST_LEN_10UNIT,
    TIM_DMA_BURST_LEN_11UNIT,
    TIM_DMA_BURST_LEN_12UNIT,
    TIM_DMA_BURST_LEN_13UNIT,
    TIM_DMA_BURST_LEN_14UNIT,
    TIM_DMA_BURST_LEN_15UNIT,
    TIM_DMA_BURST_LEN_16UNIT,
    TIM_DMA_BURST_LEN_17UNIT,
    TIM_DMA_BURST_LEN_18UNIT,
    
    TIM_DMA_BURST_LEN_RESERVED,
    
} tim_dma_burstlen_t;
    
/// TIM mode
typedef enum
{
    /// Work as timer mode
    TIM_TIMER_MODE,
    /// Work as PWM mode
    TIM_PWM_MODE,
}tim_mode_t;

/// PWM polarity
typedef enum
{
    TIM_PWM_POL_HIGH2LOW,
    TIM_PWM_POL_LOW2HIGH,
}tim_pwm_pol_t;

/// PWM force output level
typedef enum
{
    TIM_PWM_FORCE_LOW     = 4,
    TIM_PWM_FORCE_HIGH    = 5,
    TIM_PWM_FORCE_DISABLE = 6,
}tim_pwm_force_level_t;

/// PWM channels
typedef enum
{
    TIM_PWM_CHANNEL_0,
    TIM_PWM_CHANNEL_1,
    TIM_PWM_CHANNEL_2,
    TIM_PWM_CHANNEL_3,
    TIM_PWM_CHANNEL_NUM,
}tim_pwm_channel_t;

/// timer configuration
typedef struct
{
    /// delay or period, unit is us
    uint32_t period_us;
    /// event callback
    tim_timer_callback_t callback;
}tim_timer_config_t;

/// PWM channel configuration
typedef struct
{
    /// PWM polarity
    tim_pwm_pol_t pol;
    /// pulse counter value
    uint16_t pulse_count;
}tim_pwm_channel_config_t;

/// PWM configuration
typedef struct
{
    /// Frequency for every count
    uint32_t count_freq;
    /// Period counter
    uint16_t period_count;
    /// Channel configuration
    struct
    {
        /// channel enable
        bool enable;
        /// channel configuration
        tim_pwm_channel_config_t config;
    }channel[TIM_PWM_CHANNEL_NUM];
    /// Event callback
    tim_pwm_callback_t callback;
}tim_pwm_config_t;

/// TIM configuration
typedef struct
{
    /// TIM mode
    tim_mode_t mode;
    /// TIM config
    union
    {
        /// Use as timer
        tim_timer_config_t timer;
        /// Use as PWM
        tim_pwm_config_t pwm;
    }config;
}tim_config_t;

#ifndef CONFIG_HS6621
/// TIM DMA config
typedef struct
{
    /// dma direction: true for mem to tim, false for tim to mem.
    bool mem_to_tim;
    /// Absolute address of the TIM register to access
    __IO uint32_t *tim_addr;
    /// address of the memory buffer
    uint32_t *mem_addr;
    /// length of the DMA transfer in bytes
    uint32_t len_in_bytes;
    /// TIM DMA request source: TIM_DIER_UDE, TIM_DIER_CC1DE, ..., TIM_DIER_TDE
    uint32_t req;
    /// DMA block transfer to build.
    dma_block_t *block;
} tim_dma_config_t;
#endif
/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief tim initialize
 *
 * @return None
 **/
void tim_init(void);

/**
 * @brief tim configure
 *
 * @param[in] tim  tim object
 * @param[in] config  configuration
 *
 * @return None
 **/
void tim_config(HS_TIM_Type *tim, const tim_config_t *config);

/**
 * @brief Change PWM period count
 *
 * @param[in] tim  PWM object
 * @param[in] period_count  new period count, MAX:65535
 *
 * @return None
 **/
void tim_pwm_change_period_count(HS_TIM_Type *tim, uint16_t period_count);

/**
 * @brief Change PWM channel pulse count
 *
 * @param[in] tim  PWM object
 * @param[in] channel  PWM channel index
 * @param[in] pulse_count  new pulse count, MAX 65535, should be less than period_count
 *
 * @return None
 **/
void tim_pwm_channel_change_pulse_count(HS_TIM_Type *tim, tim_pwm_channel_t channel, uint16_t pulse_count);

/**
 * @brief Force PWM channel output
 *
 * @param[in] tim  PMW object
 * @param[in] channel  PWM channel index
 * @param[in] level  force output level
 *
 * @return None
 **/
void tim_pwm_channel_force_output(HS_TIM_Type *tim, tim_pwm_channel_t channel, tim_pwm_force_level_t level);

/**
 * @brief tim start
 *
 * After config tim (timer, PWM), call this function to start
 *
 * @param[in] tim  tim object
 *
 * @return None
 **/
void tim_start(HS_TIM_Type *tim);

/**
 * @brief tim stop
 *
 * @param[in] tim  tim object
 *
 * @return None
 **/
void tim_stop(HS_TIM_Type *tim);

#ifndef CONFIG_HS6621
/**
 * @brief tim dma configure and build dma block transfer.
 *
 * @param[in] tim  tim object
 * @param[in] config  tim dma configuration
 *
 * @return None
 **/
void tim_dma_config(HS_TIM_Type *tim, const tim_dma_config_t *config);
#endif
#ifdef __cplusplus
}
#endif

#endif

/** @} */

