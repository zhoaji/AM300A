/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: fifter.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __FIFTER_H__
#define __FIFTER_H__

#include <stdint.h>

#define	  UINT16_middle_value		0x8000

uint16_t Filter_Bandstop_50_100_150Hz_Sampling_2000Hz(uint16_t EMG_original, uint8_t Channel);

#endif
