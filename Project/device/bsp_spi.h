/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_spi.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __BSP_SPI_H__
#define	__BSP_SPI_H__

#include <stdint.h>

void spi_config(void);
uint16_t spi_read(void);

uint16_t spi_read_data(void);

#endif
