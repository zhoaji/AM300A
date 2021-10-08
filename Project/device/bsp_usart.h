/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_usart.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/
#ifndef __USART_H__
#define __USART_H__

#include "peripheral.h"
#include "queue.h"

#define UART_BAUDRATE		115200

//#define USART_BUF_LEN		256
//extern uint8_t Usart_RX_Buf[USART_BUF_LEN];
//extern QUEUE_U8	Usart_Rx;

//extern uint8_t Usart_TX_Buf[USART_BUF_LEN];
//extern QUEUE_U8	Usart_Tx;

void usart_config(void);

#endif
