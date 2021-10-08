
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: protocol.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon  协议内容参考《家用康复AM300--通讯协议V1.5》
		2.
*/


#ifndef __PRO_H__
#define __PRO_H__

#include <stdint.h>
#include "handler.h"

#define HEAD_1		0xAA
#define HEAD_2		0x55

typedef enum{
	Idle = 0,				
	Hd1_match,			
	Hd2_match,			
	Tok_valid,			
	Len_chk,				
	CRC_chk	
}_unpacket_step;

extern PACKET_Typedef	Packet;


void protocol_handler(void);



#endif
