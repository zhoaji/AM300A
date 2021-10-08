/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_battery.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__

#include <stdint.h>

enum	battery_status{
	bat_normal  = 0,		// 正常
	bat_low_voltage,		// 低压	
	bat_sleep_mode,			// 低功耗模式
	bat_charging, 			// 充电中
	bat_recharged,			// 充电完毕
};

typedef struct{
	uint8_t 	ststus;
	uint16_t 	adc_value;
	uint8_t 	vol_level;
	uint16_t 	voltage_adc_mv;
	uint16_t  voltage_bat_mv;
}Battery_Typedef;


extern Battery_Typedef battery;


void get_battery_adc_value(void);

#endif
