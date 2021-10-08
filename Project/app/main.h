/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: main.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/


#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>
#include "queue.h"

#define PRODUCT_NAME		"AM300"

#define DEBUG_VERSION		00
#define APP_VERSION			0001
#define HW_VERSION			10


#define ENABLE 				1
#define DISABLE				0

#define BLE_BUF_LEN		256
extern uint8_t BLE_RX_Buf[BLE_BUF_LEN];
extern QUEUE_U8	BLE_Rx;

//extern uint8_t BLE_TX_Buf[BLE_BUF_LEN];
//extern QUEUE_U8	BLE_Tx;


#endif

