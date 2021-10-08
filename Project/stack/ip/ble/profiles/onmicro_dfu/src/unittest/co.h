#ifndef __CO_H__
#define __CO_H__

#include <stdint.h>
#include <stdbool.h>
#define __ALIGNED(x) __attribute__((aligned(x)))  
#define co_assert(...)
#define log_debug printf

uint16_t co_crc16_ccitt(uint16_t crcinit, const void *buf, unsigned len);

#endif
