
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_timer.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_timer.h"
#include "bsp_spi.h"
#include "emg_wave.h"
#include "stim_control.h"

#include "bsp_gpio.h"
#include "ll.h"
/************************************************
	@Function			: tim_timer_handler
	@Description	:	定时器0计数中断处理
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void tim_timer_handler(void)
{
//	static uint8_t flag = 0;
//	if(flag) gpio_write(BITMASK(PIN_OFF_CHC), GPIO_HIGH);
//	else gpio_write(BITMASK(PIN_OFF_CHC), GPIO_LOW);
//	flag = ~flag;
	
//	get_emg_raw_adc_value();
	
	GLOBAL_INT_STOP();
	stim_50us_server();
	GLOBAL_INT_START();
}

/************************************************
	@Function			: tim_timer_handler1
	@Description	:	定时器1计数中断处理
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void tim_timer_handler1(void)
{
	get_emg_raw_adc_value();
	
//	GLOBAL_INT_STOP();
//	stim_50us_server();
//	GLOBAL_INT_START();
}

/************************************************
	@Function			: TIM0_config
	@Description	:	定时器0配置
	@parameter		: us , 微秒级定时
	@Return				: None
	@Remark				: None
*/
void TIM0_config( uint32_t us)
{
    const tim_config_t timer_cfg =
    {
        .mode = TIM_TIMER_MODE,
        .config.timer =
        {
            .period_us = us,//  1000*1000,// = 1s
            .callback = tim_timer_handler,
        },
    };

    // Timer init
    tim_config(HS_TIM0, &timer_cfg);
//    tim_start(HS_TIM0);  //20210604

}


/************************************************
	@Function			: TIM0_config
	@Description	:	定时器0配置
	@parameter		: us , 微秒级定时
	@Return				: None
	@Remark				: None
*/
void TIM1_config( uint32_t us)
{
    const tim_config_t timer1_cfg =
    {
        .mode = TIM_TIMER_MODE,
        .config.timer =
        {
            .period_us = us,//  1000*1000,// = 1s
            .callback = tim_timer_handler1,
        },
    };

    // Timer init
    tim_config(HS_TIM1, &timer1_cfg);
//    tim_start(HS_TIM1);  //20210604
		
}

/************************************************
	@Function			: tim_arr_set
	@Description	:	重设TIM的ARR寄存器
	@parameter		: *TIMx , 定时器
									value , 重设值
	@Return				: None
	@Remark				: 复位定时器计数
*/
void tim_arr_set(HS_TIM_Type *TIMx, uint16_t value)
{
//	TIMx->CNT = 0;
	TIMx->ARR = value;
}
