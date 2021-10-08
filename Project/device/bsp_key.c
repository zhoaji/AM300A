
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_key.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_key.h"
#include "bsp_gpio.h"
#include "stim_control.h"
#include "bsp_systick.h"



static void key_handler(uint32_t pin_mask)
{
//  static uint32_t counter[2] = {0};
	static uint32_t interval_time_tick[2] = {0};
	uint8_t key_num = 0;
	
//    log_debug("gpio: %08X [%d]\n", pin_mask, counter++);
	
	key_num = (pin_mask >> 25) & 0x03;
	
	switch(key_num)
	{
		case 0x01: // PIN_KEY_DOWN
			// 距离上一次按的时间超过100ms
			if(TICK_PASSED(TICK_NOW, interval_time_tick[0]) > TICK_X10MS(2)) 
			{
				intensity_up_or_down(STIM_CH_AB, KEY_DOWN);
				interval_time_tick[0] = TICK_NOW;
			}	
			break;
		case 0x02: // PIN_KEY_UP
			// 距离上一次按的时间超过100ms
			if(TICK_PASSED(TICK_NOW, interval_time_tick[1]) > TICK_X10MS(2)) 
			{
				intensity_up_or_down(STIM_CH_AB, KEY_UP);
				interval_time_tick[1] = TICK_NOW;
			}
			
			break;
		default: break;
	}
}

void key_config(void)
{
	gpio_set_interrupt(BIT_MASK(PIN_KEY_DOWN), GPIO_RISING_EDGE);
	gpio_set_interrupt(BIT_MASK(PIN_KEY_UP), GPIO_RISING_EDGE);
	gpio_set_interrupt_callback(key_handler);
}
