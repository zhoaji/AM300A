/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: algorithm.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include <stdint.h>


uint16_t emg_arithmetic_pp ( uint8_t ch, uint16_t EMG_org );   			// 计算峰值最大值
uint16_t EMG_arithmetic_average ( uint8_t ch, uint16_t EMG_org );  // 计算峰值平均值
uint16_t EMG_arithmetic_RMS( uint8_t ch, uint16_t EMG_org );				// 计算均方根


#endif
