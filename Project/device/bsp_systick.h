
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_systick.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __BSP_SYSTICK_H__
#define __BSP_SYSTICK_H__

#include <stdint.h>

extern volatile uint32_t systick_cnt;
#define TICK_OUT			(1000* 60 * 30)   // max count 30min
#define TICK_NOW			(systick_cnt)
#define TICK_INC			(systick_cnt > TICK_OUT ? systick_cnt = 0 : systick_cnt++)

#define TICK_X10MS(_x10ms)		(((uint32_t)_x10ms) * 100)
#define TICK_nS(_ns)					((uint32_t)_ns * 1000)

#define TICK_PASSED(_now, _old)		((((uint32_t)_now + TICK_OUT) - (uint32_t)_old)%TICK_OUT)

void delay_ms(uint16_t ms);

#endif
