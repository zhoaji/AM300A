
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_gpio.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/
#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "peripheral.h"

// SGM8751 Control Pin
#define PIN_OFF_CHA							2
#define PIN_OFF_CHB							3
#define PIN_OFF_CHC							21 //7

// VBUS_IN/DAC_EN  Pin
#define PIN_VBUS_IN_OR_DAC_EN 	8

// UART0 Pin
#define PIN_UART0_TX      			5
#define PIN_UART0_RX      			6

#ifdef CONFIG_LOG_OUTPUT
// I2C Pin
#define PIN_IIC_SDA      				8
#define PIN_IIC_SCK      				31
#else
#define PIN_IIC_SDA      				5
#define PIN_IIC_SCK      				6
#endif

// EMG Lead off check ADC_CH1
#define PIN_EMG_OFF_AD					11 //9

// STIM Lead off Pin
#define PIN_STIM_OFF						9 //10

// STIM Current check ADC_CH5
#define PIN_CURRENT_ACQ					10 //11

// Battery Voltage check ADC_CH6
#define PIN_BAT_AD							12

// AD7683 SPI Pin
#define PIN_ADC_CS							20 //13
#define PIN_ADC_CLK							18 //14
#define PIN_ADC_MISO						19 //15

// STIM Ouput Control Pin
#define PIN_STIM_PWM_L					13 //16
#define PIN_STIM_PWM_H					14 //17
#define PIN_STIM_PWR_EN					16 //18
#define PIN_EMG_CH_SW						23 //19
#define PIN_EMG_OR_STIM_SW			30 //20
#define PIN_STIM_OUT_B					17 //21
#define PIN_STIM_OUT_A					15 //25

// Battery Charging Status Pin					
#define PIN_CHG_ON							31 //22

// Power On Key or Battery Voltage Check Enable Pin
#define PIN_SW_OFF_OR_BAT_EN		24 //23

// SGM48751 EN Pin or STIM Discharge Pin
#define PIN_OFF_EN_OR_RELEASE		22 //24

// KEY Pin
#define PIN_KEY_DOWN						25 //26
#define PIN_KEY_UP							26 //27

// LED Pin
#define PIN_LED3								29
#define PIN_LED2								28 //29
#define PIN_LED1								27 //30
#define PIN_LED0								7 //31


extern void gpio_config(void);


#endif


