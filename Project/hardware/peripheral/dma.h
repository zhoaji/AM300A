/**
 * @file dma.h
 * @brief DMA driver
 * @date Tue 16 May 2017 02:52:45 PM CST
 * @author liqiang
 *
 * @defgroup DMA DMA
 * @ingroup PERIPHERAL
 * @brief DMA driver
 * @details DMA driver, Used for HS6621A/HS6621P
 *
 * @{
 */

#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"
#include "dma_fifo.h"

/*********************************************************************
 * MACROS
 */
/// Declare link list item
#define DMA_DECLARE_LLIP(name, n) static dma_llip_t name[n]

/*********************************************************************
 * TYPEDEFS
 */


/// DMA transfer mode and direction indicator
typedef enum
{
    /// Async memcpy mode
    DMA_MEM_TO_MEM,
    /// Slave mode & From Memory to Device
    DMA_MEM_TO_DEV,
    /// Slave mode & From Device to Memory
    DMA_DEV_TO_MEM,
    /// Slave mode & From Device to Device
    DMA_DEV_TO_DEV,
}dma_dir_t;


/// defines bus with of the DMA slave device, source or target buses
typedef enum
{
    DMA_SLAVE_BUSWIDTH_8BITS     = 0,
    DMA_SLAVE_BUSWIDTH_16BITS       ,
    DMA_SLAVE_BUSWIDTH_32BITS       ,
    DMA_SLAVE_BUSWIDTH_64BITS       ,
    DMA_SLAVE_BUSWIDTH_128BITS      ,
    DMA_SLAVE_BUSWIDTH_256BITS      ,
    DMA_SLAVE_BUSWIDTH_MAXBITS      ,
}dma_slave_buswidth_t;

/// DMA status
typedef enum
{
    DMA_STATUS_ERROR,
    DMA_STATUS_XFER_OK,
    DMA_STATUS_BLOCK_OK,
    DMA_STATUS_UNKNOWN,
}dma_status_t;

/// DMA burst length
typedef enum
{
    DMA_BURST_LEN_1UNITS      = 0,     /* a unit length equal to SRC/DST_TR_WIDTH */
    DMA_BURST_LEN_4UNITS         ,
    DMA_BURST_LEN_8UNITS         ,
    DMA_BURST_LEN_16UNITS        ,
    DMA_BURST_LEN_32UNITS        ,
    DMA_BURST_LEN_64UNITS        ,
    DMA_BURST_LEN_128UNITS       ,
    DMA_BURST_LEN_256UNITS       ,

    DMA_BURST_LEN_RESERVE        ,
}dma_burstlen_t;

/// DMA ID for support peripheral
typedef enum
{
    UART1_TX_DMA_ID              = 0,
    UART1_RX_DMA_ID              = 1,
    I2C_TX_DMA_ID                = 2,
    I2C_RX_DMA_ID                = 3,
    SPI_MST0_TX_DMA_ID           = 4,
    SPI_MST0_RX_DMA_ID           = 5,
    SPI_MST1_TX_DMA_ID           = 6,
    SPI_MST1_RX_DMA_ID           = 7,
    TIMER0_DMA_ID                = 8,
    TIMER1_DMA_ID                = 9,
    TIMER2_DMA_ID                = 10,
    I2S_TX_DMA_ID                = 11,
    ADC_DMA_ID                   = 12,
    I2S_RX_DMA_ID                = 13,
    I2C2_TX_DMA_ID               = 14,
    I2C2_RX_DMA_ID               = 15,
    DMA_MAX_ID
}dma_id_t;

/**
 * @brief DMA Event callback
 *
 * @param[in] status  event status
 * @param[in] cur_src_addr  current source address
 * @param[in] cur_dst_addr  current dest address
 * @param[in] xfer_size  transmited size
 *
 * @return None
 **/
typedef void (*dma_callback_t)(dma_status_t status, uint32_t cur_src_addr, uint32_t cur_dst_addr, uint32_t xfer_size);

/// @cond

/// Linked List Item (DMA block descriptor), driver filled automatically
typedef struct
{
    uint32_t sar;
    uint32_t dar;
    uint32_t llp;
    uint32_t ctllo;
    uint32_t ctlhi;
    uint32_t sstat;
    uint32_t dstat;
}dma_llip_t;

/// @endcond

/// Linked list item config
typedef struct
{
    /// Enabel link list item
    bool       enable;
    /// Fifo enable (ring buffer)
    bool       use_fifo;
    /// Source address
    uint32_t   src_addr;
    /// Destination address
    uint32_t   dst_addr;
    /// Link list item block num
    uint32_t   block_num;
    /// Link list item block length, MAX 4095
    uint32_t   block_len;
    /// Linked List Item struct
    dma_llip_t *llip;
}dma_lli_t;

/// DMA configuration
typedef struct
{
    /// direction
    dma_dir_t      direction;
    /// source bus width
    dma_slave_buswidth_t src_addr_width;
    /// dest bus width
    dma_slave_buswidth_t dst_addr_width;
    /// source burst length
    dma_burstlen_t src_burst;
    /// dest burst length
    dma_burstlen_t dst_burst;
    /// enable flow ctrl
    bool           dev_flow_ctrl;
    /// peripheral ID
    dma_id_t       slave_id;
    /// Priority
    uint32_t       priority;
    /// Liked List Item
    dma_lli_t      lli;
    /// Event callback
    dma_callback_t callback;
}dma_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/**
 * @brief DMA initialize
 *
 * @return None
 **/
void dma_init(void);

/**
 * @brief DMA allocate a new channel object
 *
 * @retval NOTNULL new DMA object
 * @retval NULL No DMA to allocate
 **/
HS_DMA_CH_Type *dma_allocate(void);

/**
 * @brief DMA release a channel object
 *
 * @param[in] ch  DMA channel object (return by dma_allocate)
 *
 * @return None
 **/
void dma_release(HS_DMA_CH_Type *ch);

/**
 * @brief DMA config
 *
 * @param[in] ch  DMA channel object
 * @param[in] config  dma configurtaion
 *
 * @retval true success
 * @retval false fail
 **/
bool dma_config(HS_DMA_CH_Type *ch, dma_config_t *config);

/**
 * @brief DMA start
 *
 * @param[in] ch  DMA channel object
 * @param[in] src_addr  source address
 * @param[in] dst_addr  dest address
 * @param[in] trans_count  transmit count
 *
 * @retval true success
 * @retval false fail
 **/
bool dma_start(HS_DMA_CH_Type *ch, uint32_t src_addr, uint32_t dst_addr, uint32_t trans_count);

/**
 * @brief DMA start with LINK LIST
 *
 * @param[in] ch  DMA channel object
 *
 * @return None
 **/
void dma_start_with_lli(HS_DMA_CH_Type *ch);

/**
 * @brief DMA stop
 *
 * @param[in] ch  DMA channel object
 *
 * @return None
 **/
void dma_stop(HS_DMA_CH_Type *ch);

/**
 * @brief DMA wait stop
 *
 * @param[in] ch  DMA channel object
 *
 * @return None
 **/
void dma_wait_stop(HS_DMA_CH_Type *ch);

/**
 * @brief DMA get current source address
 *
 * @param[in] ch  DMA channel object
 *
 * @return current source address
 **/
__STATIC_INLINE uint32_t dma_get_src_addr(HS_DMA_CH_Type *ch)
{
    return ch->SAR;
}

/**
 * @brief DMA get dest address
 *
 * @param[in] ch  DMA channel object
 *
 * @return current dest address
 **/
__STATIC_INLINE uint32_t dma_get_dst_addr(HS_DMA_CH_Type *ch)
{
    return ch->DAR;
}

/**
 * @brief DMA set address increment mode
 *
 * @param[in] ch  DMA channel object
 * @param[in] src_mode  source mode: DWC_CTLL_SRC_INC/DWC_CTLL_SRC_DEC/DWC_CTLL_SRC_FIX
 * @param[in] dst_mode  dest mode: DWC_CTLL_SRC_INC/DWC_CTLL_SRC_DEC/DWC_CTLL_SRC_FIX
 *
 * @return current dest address
 **/
__STATIC_INLINE void dma_addr_inc_mode(HS_DMA_CH_Type *ch, uint32_t src_mode, uint32_t dst_mode)
{
    ch->CTL_LO = (ch->CTL_LO & ~(DWC_CTLL_SRC_MASK|DWC_CTLL_DST_MASK)) | (src_mode | dst_mode);
}

#ifdef __cplusplus
}
#endif

#endif

/** @} */

