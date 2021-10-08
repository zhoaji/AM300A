
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
	@Description	:	systick��ʱ���жϴ�����
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
	@Description	:	���뼶��ʱ
	@parameter		: nms , ��ʱʱ�䣬��λ������
	@Return				: None
	@Remark				: None
*/
void delay_ms(uint16_t nms)
{
	uint32_t tick_old = TICK_NOW; 
	
	while(( TICK_NOW - tick_old) <= nms);
	
}


