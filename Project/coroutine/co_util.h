/*********************************************************************
 * @file co_util.h
 * @brief 
 * @version 1.0
 * @date 14/11/3 19:33:49
 * @author liqiang
 *
 * @note 
 */

#ifndef __CO_UTIL_H__
#define __CO_UTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "co.h"
#include "hs66xx.h"

/*********************************************************************
 * MACROS
 */
#define co_type_cpy(dst, src, type)         memcpy(dst, src, sizeof(type))
#define co_type_set(dst, val, type)         memset(dst, val, sizeof(type))

#define countof(s)                          (sizeof(s)/sizeof((s)[0]))

#ifndef offsetof
#define offsetof(type, member)              ((unsigned int) &((type *)0)->member)
#endif
#define container_of(ptr, type, member)     ((type *)((char *)(ptr) - offsetof(type,member)))

/* OTA application: also defined in nordic_common.h */
#if !defined(MAX)
#define MAX(x, y)                           (((x) > (y)) ? (x) : (y))
#define MIN(x, y)                           (((x) < (y)) ? (x) : (y))
#endif

#define UINT32_TO_STREAM(p, u32)            {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}
#define UINT24_TO_STREAM(p, u24)            {*(p)++ = (uint8_t)(u24); *(p)++ = (uint8_t)((u24) >> 8); *(p)++ = (uint8_t)((u24) >> 16);}
#define UINT16_TO_STREAM(p, u16)            {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT8_TO_STREAM(p, u8)              {*(p)++ = (uint8_t)(u8);}
#define INT8_TO_STREAM(p, u8)               {*(p)++ = (int8_t)(u8);}
#define ARRAY32_TO_STREAM(p, a)             {register int ijk; for (ijk = 0; ijk < 32;  ijk++) *(p)++ = (uint8_t) a[31 - ijk];}
#define ARRAY16_TO_STREAM(p, a)             {register int ijk; for (ijk = 0; ijk < 16;  ijk++) *(p)++ = (uint8_t) a[15 - ijk];}
#define ARRAY8_TO_STREAM(p, a)              {register int ijk; for (ijk = 0; ijk < 8;   ijk++) *(p)++ = (uint8_t) a[7 - ijk];}
#define ARRAY_TO_STREAM(p, a, len)          {register int ijk; for (ijk = 0; ijk < len; ijk++) *(p)++ = (uint8_t) a[ijk];}
#define REVERSE_ARRAY_TO_STREAM(p, a, len)  {register int ijk; for (ijk = 0; ijk < len; ijk++) *(p)++ = (uint8_t) a[len - 1 - ijk];}

#define STREAM_TO_UINT8(u8, p)              {u8 = (uint8_t)(*(p)); (p) += 1;}
#define STREAM_TO_UINT16(u16, p)            {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define STREAM_TO_UINT24(u32, p)            {u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) ); (p) += 3;}
#define STREAM_TO_UINT32(u32, p)            {u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) + ((((uint32_t)(*((p) + 3)))) << 24)); (p) += 4;}
#define STREAM_TO_ARRAY32(a, p)             {register int ijk; register uint8_t *_pa = (uint8_t *)a + 31; for (ijk = 0; ijk < 32; ijk++) *_pa-- = *p++;}
#define STREAM_TO_ARRAY16(a, p)             {register int ijk; register uint8_t *_pa = (uint8_t *)a + 15; for (ijk = 0; ijk < 16; ijk++) *_pa-- = *p++;}
#define STREAM_TO_ARRAY8(a, p)              {register int ijk; register uint8_t *_pa = (uint8_t *)a + 7;  for (ijk = 0; ijk < 8;  ijk++) *_pa-- = *p++;}
#define STREAM_TO_ARRAY(a, p, len)          {register int ijk; for (ijk = 0; ijk < len; ijk++) ((uint8_t *) a)[ijk] = *p++;}
#define REVERSE_STREAM_TO_ARRAY(a, p, len)  {register int ijk; register uint8_t *_pa = (uint8_t *)a + len - 1; for (ijk = 0; ijk < len; ijk++) *_pa-- = *p++;}

#define UINT32_TO_BE_STREAM(p, u32)         {*(p)++ = (uint8_t)((u32) >> 24);  *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)(u32); }
#define UINT24_TO_BE_STREAM(p, u24)         {*(p)++ = (uint8_t)((u24) >> 16); *(p)++ = (uint8_t)((u24) >> 8); *(p)++ = (uint8_t)(u24);}
#define UINT16_TO_BE_STREAM(p, u16)         {*(p)++ = (uint8_t)((u16) >> 8); *(p)++ = (uint8_t)(u16);}
#define UINT8_TO_BE_STREAM(p, u8)           {*(p)++ = (uint8_t)(u8);}
#define ARRAY_TO_BE_STREAM(p, a, len)       {register int ijk; for (ijk = 0; ijk < len; ijk++) *(p)++ = (uint8_t) a[ijk];}

#define BE_STREAM_TO_UINT8(u8, p)           {u8 = (uint8_t)(*(p)); (p) += 1;}
#define BE_STREAM_TO_UINT16(u16, p)         {u16 = (uint16_t)(((uint16_t)(*(p)) << 8) + (uint16_t)(*((p) + 1))); (p) += 2;}
#define BE_STREAM_TO_UINT24(u32, p)         {u32 = (((uint32_t)(*((p) + 2))) + ((uint32_t)(*((p) + 1)) << 8) + ((uint32_t)(*(p)) << 16)); (p) += 3;}
#define BE_STREAM_TO_UINT32(u32, p)         {u32 = ((uint32_t)(*((p) + 3)) + ((uint32_t)(*((p) + 2)) << 8) + ((uint32_t)(*((p) + 1)) << 16) + ((uint32_t)(*(p)) << 24)); (p) += 4;}
#define BE_STREAM_TO_ARRAY(p, a, len)       {register int ijk; for (ijk = 0; ijk < len; ijk++) ((uint8_t *) a)[ijk] = *p++;}

#define READ_UINT8(buffer)                  ( ((uint8_t *)(buffer))[0])
#define READ_UINT16(buffer)                 ( ((uint16_t) ((uint8_t *)(buffer))[0]) | (((uint16_t)((uint8_t *)(buffer))[1]) << 8))
#define READ_UINT24(buffer)                 ( ((uint32_t) ((uint8_t *)(buffer))[0]) | (((uint32_t)((uint8_t *)(buffer))[1]) << 8) | (((uint32_t)((uint8_t *)(buffer))[2]) << 16))
#define READ_UINT32(buffer)                 ( ((uint32_t) ((uint8_t *)(buffer))[0]) | (((uint32_t)((uint8_t *)(buffer))[1]) << 8) | (((uint32_t)((uint8_t *)(buffer))[2]) << 16) | (((uint32_t) ((uint8_t *)(buffer))[3])) << 24)

#define STATIC_INLINE                       __STATIC_INLINE

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

#if 0
/**
 * @brief co_ctz() 后导0的个数
 *
 * @param[in] x  
 *
 * @return 
 **/
int co_ctz (uint32_t x);
#endif

/**
 * @brief co_array_left_move()
 *
 * @param[in] array  
 * @param[in] len  
 * @param[in] move  
 *
 * @return 
 **/
void co_array_left_move(void *array, unsigned len, unsigned move);

/**
 * @brief co_array_reversal()
 *
 * @param[in] array  
 * @param[in] len  
 *
 * @return 
 **/
void co_array_reversal(void *array, unsigned len);

/**
 * @brief co_array_is_all_zero()
 *
 * @param[in] array  
 * @param[in] len  
 *
 * @return 
 **/
bool co_array_is_all_zero(const void *array, unsigned len);

/**
 * @brief co_crc16_ccitt()
 *
 * @param[in] crcinit  
 * @param[in] buf  
 * @param[in] len  
 *
 * @return 
 **/
uint16_t co_crc16_ccitt(uint16_t crcinit, const void *buf, unsigned len);

#ifdef __cplusplus
}
#endif

#endif

