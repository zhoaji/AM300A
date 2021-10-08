
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_gpio.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_gpio.h"

static void gpio_pin_config(uint8_t PIN, gpio_direction_t set, pmu_pin_mode_t type)
{
	pinmux_config(PIN, PINMUX_GPIO_MODE_CFG);  
	gpio_set_direction(BITMASK(PIN), set);
	pmu_pin_mode_set(BITMASK(PIN), type);
//	if( GPIO_OUTPUT == set ) gpio_write(BITMASK(PIN), level);
}


void gpio_config(void)
{
	
	pmu_pin_mode_set(0xFFFFFFF3, PMU_PIN_MODE_PU);   // 说是防漏电
	pmu_pin_mode_set(0x0000000C, PMU_PIN_MODE_PD);   // 说是防漏电
	
	gpio_open(); // Open GPIO Clock
	
	// SGM8751 Control Pin
	gpio_pin_config(PIN_OFF_CHA, GPIO_OUTPUT, PMU_PIN_MODE_PD);
	gpio_pin_config(PIN_OFF_CHB, GPIO_OUTPUT, PMU_PIN_MODE_PD);
	gpio_pin_config(PIN_OFF_CHC, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	
	gpio_pin_config(PIN_VBUS_IN_OR_DAC_EN, GPIO_INPUT, PMU_PIN_MODE_FLOAT);		
	
#ifdef CONFIG_LOG_OUTPUT
	// UART0
  pinmux_config(PIN_UART0_TX, PINMUX_UART0_SDA_O_CFG);  // TX
	pinmux_config(PIN_UART0_RX, PINMUX_UART0_SDA_I_CFG);	// RX
	
	gpio_pin_config(PIN_IIC_SDA, GPIO_OUTPUT, PMU_PIN_MODE_PU); // SDA
	gpio_pin_config(PIN_IIC_SCK, GPIO_OUTPUT, PMU_PIN_MODE_PU);	// SCK	
#else
	gpio_pin_config(PIN_IIC_SDA, GPIO_OUTPUT, PMU_PIN_MODE_PU); // SDA
	gpio_pin_config(PIN_IIC_SCK, GPIO_OUTPUT, PMU_PIN_MODE_PU);	// SCK	
#endif
	
	// ADC
	pinmux_config(PIN_EMG_OFF_AD, PINMUX_ANALOG_CH7_PIN7_CFG);
	pinmux_config(PIN_CURRENT_ACQ, PINMUX_ANALOG_CH1_PIN9_CFG);
	pinmux_config(PIN_BAT_AD, PINMUX_ANALOG_CH1_PIN9_CFG);
	
	// STIM OFF
	gpio_pin_config(PIN_STIM_OFF, GPIO_INPUT, PMU_PIN_MODE_FLOAT);	
	
	// SPI
	pinmux_config(PIN_ADC_MISO,  PINMUX_SPI0_MST_SDA_I_CFG); // MISO
	pinmux_config(PIN_ADC_CLK,  PINMUX_SPI0_MST_SCK_CFG);    // SCLK
	pinmux_config(PIN_ADC_CS,  PINMUX_SPI0_MST_CSN_CFG);   	 // CSN 
//	gpio_pin_config(PIN_ADC_MISO, GPIO_INPUT, PMU_PIN_MODE_FLOAT);	
//	gpio_pin_config(PIN_ADC_CLK, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
//	gpio_pin_config(PIN_ADC_CS, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	
	// STIM Power
	gpio_pin_config(PIN_STIM_PWR_EN, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	
	// STIM Control
	gpio_pin_config(PIN_STIM_PWM_L, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_STIM_PWM_H, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_EMG_CH_SW, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_EMG_OR_STIM_SW, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_STIM_OUT_B, GPIO_OUTPUT, PMU_PIN_MODE_PP);
	gpio_pin_config(PIN_STIM_OUT_A, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_OFF_EN_OR_RELEASE, GPIO_OUTPUT, PMU_PIN_MODE_PP);	

#ifndef CONFIG_LOG_OUTPUT
	// Battery 
	gpio_pin_config(PIN_CHG_ON, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_SW_OFF_OR_BAT_EN, GPIO_OUTPUT, PMU_PIN_MODE_PP);
#endif

	// LED 
	gpio_pin_config(PIN_LED0, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_LED1, GPIO_OUTPUT, PMU_PIN_MODE_PP);
	gpio_pin_config(PIN_LED2, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	gpio_pin_config(PIN_LED3, GPIO_OUTPUT, PMU_PIN_MODE_PP);	
	
	// KEY
	gpio_pin_config(PIN_KEY_UP, GPIO_INPUT, PMU_PIN_MODE_FLOAT);			// KEY_UP
	gpio_pin_config(PIN_KEY_DOWN, GPIO_INPUT, PMU_PIN_MODE_FLOAT);		// KEY_DOWN	
	
	
	gpio_write(BITMASK(PIN_OFF_CHA), GPIO_LOW);
	gpio_write(BITMASK(PIN_OFF_CHB), GPIO_LOW);
	gpio_write(BITMASK(PIN_OFF_CHC), GPIO_LOW);
	
#ifndef CONFIG_LOG_OUTPUT	
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_HIGH);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH);
#endif	
	
	gpio_write(BITMASK(PIN_ADC_CLK), GPIO_LOW);
	gpio_write(BITMASK(PIN_ADC_CS), GPIO_HIGH);

	gpio_write(BITMASK(PIN_STIM_PWR_EN), GPIO_HIGH);
	
	gpio_write(BITMASK(PIN_STIM_PWM_L), GPIO_LOW);
	gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_LOW);
	gpio_write(BITMASK(PIN_EMG_CH_SW), GPIO_LOW);
	gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_LOW);
	gpio_write(BITMASK(PIN_STIM_OUT_B), GPIO_LOW);
	gpio_write(BITMASK(PIN_STIM_OUT_A), GPIO_LOW);
	gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_LOW);
	
	gpio_write(BITMASK(PIN_CHG_ON), GPIO_LOW);
	gpio_write(BITMASK(PIN_SW_OFF_OR_BAT_EN), GPIO_LOW);
	
	gpio_write(BITMASK(PIN_LED0), GPIO_LOW);
	gpio_write(BITMASK(PIN_LED1), GPIO_HIGH);
	gpio_write(BITMASK(PIN_LED2), GPIO_HIGH);
	gpio_write(BITMASK(PIN_LED3), GPIO_HIGH);
	
	
}

