
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: queue.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdint.h>

typedef struct queue_u8
{
	uint8_t 	*pdat;
	uint32_t	write;
	uint32_t	read;
	uint32_t	len;
}QUEUE_U8;

typedef struct queue_u16
{
	uint16_t	*pdat;
	uint32_t	write;
	uint32_t	read;
	uint32_t	len;
}QUEUE_U16;

typedef struct queue_s16
{
	int16_t		*pdat;
	uint32_t	write;
	uint32_t	read;
	uint32_t	len;
}QUEUE_S16;

#define QUEUE_INIT(_Queue, _pBuf, _len)		do{_Queue.pdat = _pBuf; _Queue.write = 0; _Queue.read = 0; _Queue.len = _len;}while(0)
#define QUEUE_INIT_P(_Queue, _pBuf, _len)		do{_Queue->pdat = _pBuf; _Queue->write = 0; _Queue->read = 0; _Queue->len = _len;}while(0)

#define QUEUE_WRITE(_Queue, _dat)		do{_Queue.write = (_Queue.write + 1) % _Queue.len; _Queue.pdat[_Queue.write] = _dat;}while(0)
#define QUEUE_WRITE_P(_Queue, _dat)		do{_Queue->write = (_Queue->write + 1) % _Queue->len; _Queue->pdat[_Queue->write] = _dat;}while(0)

#define QUEUE_READ(_Queue)	(_Queue.read = (_Queue.read + 1) % _Queue.len, _Queue.pdat[_Queue.read])
#define QUEUE_READ_P(_Queue)	(_Queue->read = (_Queue->read + 1) % _Queue->len, _Queue->pdat[_Queue->read])

#define QUEUE_STOCK(_Queue)	 ((_Queue.len + _Queue.write - _Queue.read) % _Queue.len)
#define QUEUE_STOCK_P(_Queue)	 ((_Queue->len + _Queue->write - _Queue->read) % _Queue->len)

#define QUEUE_CLR(_Queue)		do{_Queue.write = 0; _Queue.read = 0;}while(0)
#define QUEUE_CLR_P(_Queue)		do{_Queue->write = 0; _Queue->read = 0;}while(0)

#define QUEUE_CLONE(_Queue, _CloneIndex)		(_Queue.pdat[_CloneIndex])
#define QUEUE_CLONE_P(_Queue, _CloneIndex)		(_Queue->pdat[_CloneIndex])

#endif
