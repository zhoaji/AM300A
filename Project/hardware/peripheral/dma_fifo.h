/**
 * @file dma_fifo.h
 * @brief DMA fifo driver
 * @date Sat 27 May 2017 03:12:37 PM CST
 * @author liqiang
 *
 * @defgroup DMA_FIFO DMA-FIFO
 * @ingroup DMA
 * @brief DMA fifo driver
 * @details DMA FIFO descript
 *
 * @{
 */

#ifndef __DMA_FIFO_H__
#define __DMA_FIFO_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

/// @cond
/// DMA fifo structure
typedef struct
{
    uint8_t *in;
    uint8_t *out;
    uint8_t *begin;
    uint8_t *end;
}dma_fifo_t;
/// @endcond

/*********************************************************************
 * EXTERN VARIABLES
 */

/*********************************************************************
 * NOT EXTERN FUNCTIONS
 */

/// @cond
__STATIC_INLINE unsigned __dma_min(unsigned a, unsigned b)
{
    return (((a) < (b)) ? (a) : (b));
}
/// @endcond

/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief dma_fifo_init()
 *
 * @param[in] fifo  
 * @param[in] buffer  
 * @param[in] size  
 *
 * @return 
 **/
__STATIC_INLINE void dma_fifo_init(dma_fifo_t *fifo, void *buffer, unsigned size)
{
    uint8_t *p = buffer;

    fifo->in = p;
    fifo->out = p;
    fifo->begin = p;
    fifo->end = p + size;
}

/**
 * @brief dma_fifo_init()
 *
 * @param[in] fifo  
 *
 * @return 
 **/
__STATIC_INLINE void dma_fifo_reset(dma_fifo_t *fifo)
{
    fifo->in = fifo->begin;
    fifo->out = fifo->begin;
}

/**
 * @brief dma_fifo_len()
 *
 * @param[in] fifo  
 *
 * @return 
 **/
__STATIC_INLINE unsigned dma_fifo_len(dma_fifo_t *fifo)
{
    unsigned len;

    if (fifo->in < fifo->out)
        len = (fifo->end - fifo->out) + (fifo->in - fifo->begin);
    else
        len = fifo->in - fifo->out;

    return len;
}

/**
 * @brief dma_fifo_in()
 *
 * @param[in] fifo  
 * @param[in] from  
 * @param[in] len  
 *
 * @return 
 **/
__STATIC_INLINE unsigned dma_fifo_in(dma_fifo_t *fifo, const void *from, unsigned len)
{
    const uint8_t *p = from;
    uint8_t *end;

    len = __dma_min(len, fifo->end - fifo->begin);

    end = fifo->in + len;
    if(end > fifo->end)
    {
        unsigned len1 = fifo->end - fifo->in;
        unsigned len2 = end - fifo->end;
        memcpy(fifo->in, &p[0], len1);
        memcpy(fifo->begin, &p[len1], len2);
        fifo->in = fifo->begin + len2;
    }
    else
    {
        memcpy(fifo->in, p, len);
        fifo->in = end;
    }

    return len;
}

/**
 * @brief dma_fifo_out()
 *
 * @param[in] fifo  
 * @param[in] to  
 * @param[in] len  
 *
 * @return 
 **/
__STATIC_INLINE unsigned dma_fifo_out(dma_fifo_t *fifo, void *to, unsigned len)
{
    uint8_t *p = to;
    uint8_t *end;

    len = __dma_min(len, dma_fifo_len(fifo));

    end = fifo->out + len;
    if(end > fifo->end)
    {
        unsigned len1 = fifo->end - fifo->out;
        unsigned len2 = end - fifo->end;
        memcpy(&p[0], fifo->out, len1);
        memcpy(&p[len1], fifo->begin, len2);
        fifo->out = fifo->begin + len2;
    }
    else
    {
        memcpy(p, fifo->out, len);
        fifo->out = end;
    }

    return len;
}

/**
 * @brief dma_fifo_set_in_pointer()
 *
 * @param[in] fifo  
 * @param[in] in  
 *
 * @return 
 **/
__STATIC_INLINE void dma_fifo_set_in_pointer(dma_fifo_t *fifo, uint32_t in)
{
    fifo->in = (uint8_t *)in;
}

/**
 * @brief dma_fifo_set_out_pointer()
 *
 * @param[in] fifo  
 * @param[in] out  
 *
 * @return 
 **/
__STATIC_INLINE void dma_fifo_set_out_pointer(dma_fifo_t *fifo, uint32_t out)
{
    fifo->out = (uint8_t *)out;
}

#ifdef __cplusplus
}
#endif

#endif

/** @} */

