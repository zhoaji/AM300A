/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_adc.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "peripheral.h"



void bsp_adc_config(void);
int16_t Get_sample_adc(adc_channel_t ch_p, adc_callback_t cb);

void adc_sample_one_channel_irq(adc_channel_t ch_p, adc_callback_t cb);

#endif

