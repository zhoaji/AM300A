
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_systick.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_systick.h"

volatile uint32_t systick_cnt = 0;

/************************************************
	@Function			: SysTick_Handler
	@Description	:	systick定时器中断处理函数
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void SysTick_Handler(void)
{
	TICK_INC;
}

/************************************************
	@Function			: delay_ms
	@Description	:	毫秒级延时
	@parameter		: nms , 延时时间，单位：毫秒
	@Return				: None
	@Remark				: None
*/
void delay_ms(uint16_t nms)
{
	uint32_t tick_old = TICK_NOW; 
	
	while(( TICK_NOW - tick_old) <= nms);
	
}


