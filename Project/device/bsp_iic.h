/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_iic.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __BSP_IIC_H__
#define __BSP_IIC_H__

#include <stdint.h>
#include "peripheral.h"
#include "bsp_gpio.h"

#define IIC_SDA_IN							gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_INPUT)//(HS_GPIO0->OUTENCLR = (1 << PIN_IIC_SDA))
#define IIC_SDA_OUT							gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_OUTPUT) //(HS_GPIO0->OUTENSET = (1 << PIN_IIC_SDA))
#define IIC_SDA_STA							gpio_read(BITMASK(PIN_IIC_SDA))//((HS_GPIO0->DATAOUT >> PIN_IIC_SDA) & 0x01)
#define IIC_SDA_SET							gpio_write(BITMASK(PIN_IIC_SDA), GPIO_HIGH)//(HS_GPIO0->MASK_0_7[PIN_IIC_SDA] = GPIO_HIGH)
#define IIC_SDA_CLR							gpio_write(BITMASK(PIN_IIC_SDA), GPIO_LOW)//(HS_GPIO0->MASK_0_7[PIN_IIC_SDA] = GPIO_LOW)

#define IIC_SCK_SET							gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH)//(HS_GPIO0->MASK_0_7[PIN_IIC_SCK] = GPIO_HIGH)
#define IIC_SCK_CLR							gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW)//(HS_GPIO0->MASK_0_7[PIN_IIC_SCK] = GPIO_LOW)


#define DEV_ADDR			0x90

// register address
#define NOOP 					0x00
#define DEVID					0x01
#define SYNC					0x02
#define CONFIG				0x03
#define GAIN					0x04
#define TRIGGER				0x05
#define STATUS				0x07
#define DAC_DATA			0x08

void iic_config(void);
uint8_t dac60501_write_register(uint8_t offset, uint16_t data);
uint16_t dac60501_read_register(uint8_t offset);
void dac_out_value_set(uint16_t value);

#endif
