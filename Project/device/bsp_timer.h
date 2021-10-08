
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_timer.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __DSP_TIMER_H__
#define __DSP_TIMRE_H__

#include <stdint.h>
#include "timer.h"

void TIM0_config( uint32_t us);
void TIM1_config( uint32_t us);
void tim_arr_set(HS_TIM_Type *TIMx, uint16_t value);

#endif
