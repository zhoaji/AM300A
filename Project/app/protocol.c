/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: protocol.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/


#include <stdio.h>
#include "protocol.h"
#include "main.h"
#include "bsp_systick.h"
#include "crc8.h"
#include "handler.h"

PACKET_Typedef Packet;

/************************************************
	@Function			: protocol_upacket
	@Description	:	协议拆包
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
static uint8_t protocol_upacket(void)
{	
	static _unpacket_step step = Idle;
	static uint8_t Token = 0;
	static uint8_t data_len = 0;
	static uint32_t timeout_tick = 0;
	
	uint16_t index = 0;
	uint16_t byte = 0;
	
	while(QUEUE_STOCK(BLE_Rx))
	{
		if( step < Len_chk ) byte = QUEUE_READ(BLE_Rx);
		
		switch(step)
		{
			case Idle: 
				if(byte == HEAD_1) step = Hd1_match;
			break;
			
			case Hd1_match:		
				if(byte == HEAD_2) step = Hd2_match;
				else if(byte != HEAD_1) step = Idle;
			break;
			
			case Hd2_match: 
				if((byte == GERNARL_TOKEN) || (byte == AM300_TOKEN)) 
				{
					Token = byte;
					step = Tok_valid;
				}
				else if(byte ==HEAD_1) step = Hd1_match;
				else step = Hd1_match;
			break;
			
			case Tok_valid: 
				data_len = byte;
				timeout_tick = TICK_NOW;
				step = Len_chk;
			break;
			
			case Len_chk: 
				if(QUEUE_STOCK(BLE_Rx) >= data_len ) step = CRC_chk;
				if(TICK_PASSED(TICK_NOW, timeout_tick) > TICK_X10MS(20)) step = Idle;
			break;
			
			case CRC_chk: 
				index = BLE_Rx.read;
				byte = CRC8(0, HEAD_1);
				byte = CRC8(byte, HEAD_2);
				byte = CRC8(byte, Token);
				byte = CRC8(byte, data_len);
				for(int i = 0; i < data_len; i++)
				{
					if(++index >= BLE_Rx.len) index = 0;
					byte = CRC8(byte, BLE_Rx.pdat[index]);
				}	
				step = Idle;
				if( byte == 0)
				{
					Packet.para.Head1 = HEAD_1;
					Packet.para.Head2 = HEAD_2;
					Packet.para.Token = Token;
					Packet.para.Length = data_len;
					Packet.para.Type = QUEUE_READ(BLE_Rx);
					for(int i = 0; i < (data_len - 1); i++) Packet.para.Data[i] = QUEUE_READ(BLE_Rx);
					
					return (data_len + 4);
				}
//				else
//				{
//					printf("byte = 0x%x\r\n", byte);
//				}
				
			break;
			
			default: step = Idle; break;
		}
		if(step == Len_chk) return 0;
	}
	return 0;
}

/************************************************
	@Function			: protocol_handler
	@Description	:	协议处理函数
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void protocol_handler(void)
{
	if( protocol_upacket() ) 
	{
		execute_handler(&Packet); 
	}
}















