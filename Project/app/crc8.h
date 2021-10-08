
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: crc.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/


#ifndef __CRC8_H__
#define __CRC8_H__

#include <stdint.h>

uint8_t CRC8(uint8_t crc, uint8_t data);
uint8_t CRC_8( uint8_t *pData, uint8_t Len);

#endif

