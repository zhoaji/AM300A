/**
 * @file co_fifo.h
 * @brief fifo
 * @date 2015/06/02 8:57:23
 * @author liqiang
 *
 * @defgroup CO_FIFO Fifo
 * @ingroup COROUTINE
 * @brief FIFO Module
 * @details
 *
 * @{
 */

#ifndef __CO_FIFO_H__
#define __CO_FIFO_H__

#ifdef __cplusplus
extern "C"
{
#endif

/// @cond

/*********************************************************************
 * INCLUDES
 */
#include "co_debug.h"

/*********************************************************************
 * MACROS
 */
#define is_power_of_2(x)    ((x) != 0 && (((x) & ((x) - 1)) == 0))

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint8_t *buffer;
    unsigned size;
    volatile unsigned in;
    volatile unsigned out;
}co_fifo_t;

/*********************************************************************
 * INSIDE FUNCTIONS
 */

/**
 * @brief __min()
 *
 * @param[in] a  
 * @param[in] b  
 *
 * @return 
 **/
__STATIC_FORCEINLINE unsigned __min(unsigned a, unsigned b)
{
    return (((a) < (b)) ? (a) : (b));
}

/**
 * @brief __co_fifo_off()
 *
 * @param[in] fifo  
 * @param[in] off  
 *
 * @return 
 **/
__STATIC_FORCEINLINE unsigned __co_fifo_off(co_fifo_t *fifo, unsigned off)
{
    return off & (fifo->size - 1);
}

/**
 * @brief __co_fifo_in_data()
 *
 * @param[in] fifo  
 * @param[in] from  
 * @param[in] len  
 * @param[in] off  
 *
 * @return 
 **/
__STATIC_INLINE void __co_fifo_in_data(co_fifo_t *fifo, const uint8_t *from, unsigned len, unsigned off)
{
    unsigned l;

    off = __co_fifo_off(fifo, fifo->in + off);

    l = __min(len, fifo->size - off);
    memcpy(fifo->buffer + off, from, l);

    memcpy(fifo->buffer, from + l, len - l);
}

/**
 * @brief  co fifo in data 1byte
 *
 * @param[in] fifo  fifo
 * @param[in] from  from
 **/
__STATIC_FORCEINLINE void __co_fifo_in_data_1byte(co_fifo_t *fifo, const uint8_t *from)
{
    unsigned off = __co_fifo_off(fifo, fifo->in);

    *(fifo->buffer + off) = *from;
}

/**
 * @brief __co_fifo_out_data()
 *
 * @param[in] fifo  
 * @param[in] to  
 * @param[in] len  
 * @param[in] off  
 *
 * @return 
 **/
__STATIC_INLINE void __co_fifo_out_data(co_fifo_t *fifo, uint8_t *to, unsigned len, unsigned off)
{
    unsigned l;

    off = __co_fifo_off(fifo, fifo->out + off);

    l = __min(len, fifo->size - off);
    memcpy(to, fifo->buffer + off, l);

    memcpy(to + l, fifo->buffer, len - l);
}

/**
 * @brief  co fifo out data 1byte
 *
 * @param[in] fifo  fifo
 * @param[in] to  to
 **/
__STATIC_FORCEINLINE void __co_fifo_out_data_1byte(co_fifo_t *fifo, uint8_t *to)
{
    unsigned off = __co_fifo_off(fifo, fifo->out);

    *to = *(fifo->buffer + off);
}

/**
 * @brief __co_fifo_add_out()
 *
 * @param[in] fifo  
 * @param[in] off  
 *
 * @return 
 **/
__STATIC_FORCEINLINE void __co_fifo_add_out(co_fifo_t *fifo, unsigned off)
{
    fifo->out += off;
}

/**
 * @brief __co_fifo_add_in()
 *
 * @param[in] fifo  
 * @param[in] off  
 *
 * @return 
 **/
__STATIC_FORCEINLINE void __co_fifo_add_in(co_fifo_t *fifo, unsigned off)
{
    fifo->in += off;
}

/// @endcond

/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief fifo reset
 *
 * @param[in] fifo  fifo object
 *
 * @return None
 **/
__STATIC_FORCEINLINE void co_fifo_reset(co_fifo_t *fifo)
{
    fifo->in = fifo->out = 0;
}

/**
 * @brief get fifo size
 *
 * @param[in] fifo  fifo object
 *
 * @return size of fifo
 **/
__STATIC_FORCEINLINE unsigned co_fifo_size(co_fifo_t *fifo)
{
    return fifo->size;
}

/**
 * @brief get fifo length
 *
 * @param[in] fifo  fifo object
 *
 * @return length of fifo
 **/
__STATIC_FORCEINLINE unsigned co_fifo_len(co_fifo_t *fifo)
{
    register unsigned out;
    out = fifo->out;
    return fifo->in - out;
}

/**
 * @brief whether is fifo empty
 *
 * @param[in] fifo  fifo object
 *
 * @return empty?
 **/
__STATIC_FORCEINLINE int co_fifo_is_empty(co_fifo_t *fifo)
{
    return fifo->in == fifo->out;
}

/**
 * @brief whether is fifo full
 *
 * @param[in] fifo  fifo object
 *
 * @return full?
 **/
__STATIC_FORCEINLINE int co_fifo_is_full(co_fifo_t *fifo)
{
    return co_fifo_len(fifo) == co_fifo_size(fifo);
}

/**
 * @brief get fifo available length
 *
 * @param[in] fifo  fifo object
 *
 * @return available length
 **/
__STATIC_FORCEINLINE unsigned co_fifo_avail(co_fifo_t *fifo)
{
    return co_fifo_size(fifo) - co_fifo_len(fifo);
}

/**
 * @brief fifo init
 *
 * @param[in] fifo  fifo object
 * @param[in] buffer  fifo buffer
 * @param[in] size  fifo size
 *
 * @return None
 **/
__STATIC_INLINE void co_fifo_init(co_fifo_t *fifo, uint8_t *buffer, unsigned size)
{
    co_assert(is_power_of_2(size));

    fifo->buffer = buffer;
    fifo->size = size;

    co_fifo_reset(fifo);
}

/**
 * @brief put data to fifo
 *
 * @param[in] fifo  fifo object
 * @param[in] from  data buffer
 * @param[in] len  data buffer length
 *
 * @return putted length
 **/
__STATIC_INLINE unsigned co_fifo_in(co_fifo_t *fifo, const uint8_t *from, unsigned len)
{
    len = __min(co_fifo_avail(fifo), len);

    __co_fifo_in_data(fifo, from, len, 0);
    __co_fifo_add_in(fifo, len);

    return len;
}

/**
 * @brief  co fifo in 1byte
 *
 * @param[in] fifo  fifo
 * @param[in] from  from
 *
 * @return putted length
 **/
__STATIC_FORCEINLINE unsigned co_fifo_in_1byte(co_fifo_t *fifo, const uint8_t *from)
{
    if (!co_fifo_avail(fifo)) return 0;

    __co_fifo_in_data_1byte(fifo, from);
    __co_fifo_add_in(fifo, 1);

    return 1;
}

/**
 * @brief obtain data from fifo
 *
 * @param[in] fifo  fifo object
 * @param[in] to  data buffer
 * @param[in] len  data buffer length
 *
 * @return obtained length
 **/
__STATIC_INLINE unsigned co_fifo_out(co_fifo_t *fifo, uint8_t *to, unsigned len)
{
    len = __min(co_fifo_len(fifo), len);

    __co_fifo_out_data(fifo, to, len, 0);
    __co_fifo_add_out(fifo, len);

    return len;
}

/**
 * @brief  co fifo out 1byte
 *
 * @param[in] fifo  fifo
 * @param[in] to  to
 *
 * @return obtained length
 **/
__STATIC_FORCEINLINE unsigned co_fifo_out_1byte(co_fifo_t *fifo, uint8_t *to)
{
    if (co_fifo_is_empty(fifo)) return 0;

    __co_fifo_out_data_1byte(fifo, to);
    __co_fifo_add_out(fifo, 1);

    return 1;
}

/**
 * @brief peek data from fifo
 *
 * @param[in] fifo  fifo object
 * @param[in] to  data buffer
 * @param[in] len  data buffer length
 *
 * @return peeked length
 **/
__STATIC_INLINE unsigned co_fifo_peek(co_fifo_t *fifo, uint8_t *to, unsigned len)
{
    len = __min(co_fifo_len(fifo), len);

    __co_fifo_out_data(fifo, to, len, 0);

    return len;
}

/**
 * @brief peek data from fifo with offset
 *
 * @param[in] fifo  fifo object
 * @param[in] to  data buffer
 * @param[in] len  data buffer length
 * @param[in] offset  buffer offset
 *
 * @return peeked length
 **/
__STATIC_INLINE unsigned co_fifo_peek_ex(co_fifo_t *fifo, uint8_t *to, unsigned len, unsigned offset)
{
    len = __min(co_fifo_len(fifo), len + offset);

    __co_fifo_out_data(fifo, to, len, offset);

    return len;
}

#ifdef __cplusplus
}
#endif

#endif

/** @} */


