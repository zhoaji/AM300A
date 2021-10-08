
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_battery.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_battery.h"
#include "adc_ex.h"
#include "bsp_adc.h"
#include "bsp_gpio.h"

#define BAT_ADC_CH			ADC_CHANNEL_EXTERN_CH6     

Battery_Typedef battery;

/************************************************
	@Function			: battery_adc_sample_handler
	@Description	:	电池电压ADC采样回调函数
	@parameter		: exent , 回调事件
	@Return				: None
	@Remark				: None
*/
static void battery_adc_sample_handler(adc_event_t exent)
{
	
	
	battery.ststus = bat_normal;
//	 gpio_write(BITMASK(30), GPIO_HIGH);
	battery.adc_value = (int16_t)(adc_channel_read_data(BAT_ADC_CH)&0x0000FFFF);
//	 gpio_write(BITMASK(30), GPIO_LOW);
	battery.voltage_adc_mv = (uint16_t)((battery.adc_value * 0.8 / 2048.0) * 1000);
	battery.voltage_bat_mv = battery.voltage_adc_mv * 2;
	battery.vol_level = 3;  // 0 ~ 3 level 
	
	
	
	adc_del_channel(BAT_ADC_CH);
	
//	printf("battery.adc_value = %d\r\n",battery.adc_value);
//	printf("battery.voltage_mv = %d mv \r\n", battery.voltage_mv);
}

/************************************************
	@Function			: Get_battery_adc_value
	@Description	:	获取电池电压ADC值
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void get_battery_adc_value(void)
{
//	 gpio_write(BITMASK(PIN_SW_OFF_OR_BAT_EN), GPIO_HIGH);  // enable bat voltage sample
	adc_sample_one_channel_irq(BAT_ADC_CH, (adc_callback_t)battery_adc_sample_handler);
//	 gpio_write(BITMASK(PIN_SW_OFF_OR_BAT_EN), GPIO_LOW);   // disable bat voltage sample
}



