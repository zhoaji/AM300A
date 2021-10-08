
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_usart.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_usart.h"
#include "main.h"

//uint8_t Usart_RX_Buf[USART_BUF_LEN] = {0};
//QUEUE_U8	Usart_Rx;

//uint8_t Usart_TX_Buf[USART_BUF_LEN] = {0};
//QUEUE_U8	Usart_Tx;

static void receive_handler(uint8_t data)
{		
  QUEUE_WRITE(BLE_Rx, data);
}


void usart_config(void)
{
	uart_open(HS_UART0, UART_BAUDRATE, UART_FLOW_CTRL_DISABLED, receive_handler);
}


