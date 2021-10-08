/**
 * @file dma.h
 * @brief DMA driver
 * @date Tue 16 May 2017 02:52:45 PM CST
 * @author liqiang
 *
 * @defgroup DMA DMA
 * @ingroup PERIPHERAL
 * @brief DMA driver
 * @details DMA driver, Used for HS6621C/HS6621CB
 *
 * @{
 */

#ifndef __DMA_NDS_H__
#define __DMA_NDS_H__

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
    /// Slave mode & From Device to Device, not supported currently, due to dma_config_t now has only one slave_id.
    DMA_DEV_TO_DEV,
    /// Error indicator
    DMA_DIR_MAX,
}dma_dir_t;

typedef enum
{
    DMA_ADDR_CTRL_INC      = 0,
    DMA_ADDR_CTRL_DEC         ,
    DMA_ADDR_CTRL_FIX         ,
    DMA_ADDR_CTRL_RESERVED    ,
} dma_addr_ctrl_t;

typedef enum
{
    DMA_HANDSHAKE_MODE_NORMAL      = 0,
    DMA_HANDSHAKE_MODE_HANDSHAKE      ,
} dma_handshake_mode_t;
    
/// defines bus with of the DMA slave device, source or target buses
typedef enum
{
    DMA_SLAVE_BUSWIDTH_8BITS     = 0,
    DMA_SLAVE_BUSWIDTH_16BITS       ,
    DMA_SLAVE_BUSWIDTH_32BITS       ,
    DMA_SLAVE_BUSWIDTH_MAXBITS      ,
} dma_slave_buswidth_t, dma_buswidth_t;
    
/// DMA status
typedef enum
{
    DMA_STATUS_BLOCK_OK,
    DMA_STATUS_ERROR,
    DMA_STATUS_ABORT,
    DMA_STATUS_UNKNOWN,
}dma_status_t;

/// DMA burst length
typedef enum
{
    DMA_BURST_LEN_1UNITS      = 0,     /* a unit length equal to SRC/DST_TR_WIDTH */
    DMA_BURST_LEN_2UNITS         ,
    DMA_BURST_LEN_4UNITS         ,
    DMA_BURST_LEN_8UNITS         ,
    DMA_BURST_LEN_16UNITS        ,
    DMA_BURST_LEN_32UNITS        ,
    DMA_BURST_LEN_64UNITS        ,
    DMA_BURST_LEN_128UNITS       ,

    DMA_BURST_LEN_RESERVE        ,
}dma_burstlen_t;

/// DMA ID for support peripheral
/**
 * HS6621C DMA req/ack allocation (according to LaoXing):
 * 
 * F: w_i2c2_dma_rx_req
 * E: w_i2c2_dma_tx_req
 * D: w_i2s_dma_rx_req
 * C: gadc_dma_ahb_req
 * B: w_i2s_dma_tx_req
 * A: w_timer3_dma_req
 * 9: w_timer2_dma_req
 * 8: w_timer1_dma_req
 * 7: i2c3_spi1_dma_sel(HS_SYS->MON[16], default=0)? i2c3_dma_rx : spi_mst1_dma_rx;
 * 6: i2c3_spi1_dma_sel(HS_SYS->MON[16], default=0)? i2c3_dma_tx : spi_mst1_dma_tx;
 * 5: w_spi_mst0_dma_rx_req
 * 4: w_spi_mst0_dma_tx_req
 * 3: w_i2c_dma_rx_req
 * 2: w_i2c_dma_tx_req
 * 1: w_uart1_dma_rx_req
 * 0: w_uart1_dma_tx_req
 *
 */
typedef enum
{
    UART1_TX_DMA_ID              = 0,
    UART1_RX_DMA_ID              = 1,
    I2C_TX_DMA_ID                = 2, // I2C0
    I2C_RX_DMA_ID                = 3, // I2C0
    SPI_MST0_TX_DMA_ID           = 4,
    SPI_MST0_RX_DMA_ID           = 5,
    /*
     * HS6621:
     * req/ack pair 6 is fixed to SPI_MST1_TX_DMA_ID,
     * req/ack pair 7 is fixed to SPI_MST1_RX_DMA_ID.
     *
     * HS6621C:
     * req/ack pair 6/7 are multiplexed between SPI1 and I2C2
     * by bit 16 of register HS6620_MON_AD (i2c2_spi1_dma_sel), which is default to 0:
     *   req_ack_pair_6 = HS6620_MON_AD[16] ? I2C2_TX_DMA_ID : SPI_MST1_TX_DMA_ID
     *   req_ack_pair_7 = HS6620_MON_AD[16] ? I2C2_RX_DMA_ID : SPI_MST1_RX_DMA_ID
     * So, I2C2 and SPI1 can't use DMA at the same time. 
     */
    SPI_MST1_TX_DMA_ID           = 6,
    SPI_MST1_RX_DMA_ID           = 7,
    TIMER0_DMA_ID                = 8,
    TIMER1_DMA_ID                = 9,
    TIMER2_DMA_ID                = 10,
    I2S_DMA_ID                   = 11, // hs6621
    I2S_TX_DMA_ID                = 11, // hs6621c
    ADC_DMA_ID                   = 12,
    DMIC_DMA_ID                  = 13, // hs6621
    I2S_RX_DMA_ID                = 13, // hs6621c
    I2C1_TX_DMA_ID               = 14, // hs6621c
    I2C1_RX_DMA_ID               = 15, // hs6621c
    I2C2_TX_DMA_ID               = 16, // use same req/ack with SPI_MST1_TX_DMA_ID
    I2C2_RX_DMA_ID               = 17, // use same req/ack with SPI_MST1_RX_DMA_ID

    MEM_DMA_ID                   = 18, // extended for identifying memory, but note that memory have no req/ack for DMA.
    
    DMA_MAX_ID                       ,
    
}dma_id_t;

/**
 * @brief DMA Event callback
 *
 * @param[in] status  event status
 * @param[in] cur_src_addr  current source address
 * @param[in] cur_dst_addr  current dest address
 * @param[in] xfer_size  transmited size in source transfer widths
 *
 * @return None
 **/
typedef void (*dma_callback_t)(dma_status_t status, uint32_t cur_src_addr, uint32_t cur_dst_addr, uint32_t xfer_size);

/// @cond

/** 
 *  DMA block transfer desciptor, corresponding to linked-list item of channel registers.
 *  Low-level structure dedicated to hardware implementations.
 *  User need not to know the details of its content.
 */
typedef HS_DMA_CH_Type dma_block_t;

/// Linked List Item (DMA block descriptor), driver filled automatically
typedef dma_block_t dma_llip_t;

/// @endcond

/// Linked list item config
/// Note: This lli config can only be used for DMA transfers of which the memory side is consecutive memory buffers.
typedef struct
{
    /// Enabel link list item
    bool       enable;
    /// Fifo enable (ring buffer), name misleading, should be named "circular"
    bool       use_fifo;
    /// Source address
    uint32_t   src_addr;
    /// Destination address
    uint32_t   dst_addr;
    /// Link list item block num
    uint32_t   block_num;
    /// Link list item block length, MAX 4095*src_width/8 for hs6621 / (2**22 - 1)*src_width/8 for hs6621c, in bytes.
    uint32_t   block_len;
    /// Linked List Item struct
    dma_llip_t *llip;
}dma_lli_t, dma_lli_of_consecutive_mem_buffers_t;

/// DMA configuration
typedef struct
{
    /// direction
    dma_dir_t      direction;
    /// source bus width
    dma_slave_buswidth_t src_addr_width; // the name "addr_width" leads to misunderstanding.
                                         // "bus_width" is the right selection.
    /// dest bus width
    dma_slave_buswidth_t dst_addr_width;
    /// source burst length, in elements
    dma_burstlen_t src_burst;
    /// dest burst length, in elements
    dma_burstlen_t dst_burst;
    /// behave as flow controller by the device other than by DMAC.
    bool           dev_flow_ctrl;
    /// peripheral ID
    dma_id_t       slave_id;
    /// Priority
    uint32_t       priority;
    /// Linked List Item
    dma_lli_t      lli;
    /// Event callback
    dma_callback_t callback;
}dma_config_t;

    
#if 1
/*********************************************************************
 * New Programming Interface
 */
/// for HS6621's DesignWare DMA Controller -- the DW_ahb_dmac
typedef enum
{
    DMA_FLOW_CONTROLLER_USE_NONE,
    DMA_FLOW_CONTROLLER_USE_DMAC,
    DMA_FLOW_CONTROLLER_USE_SRC,
    DMA_FLOW_CONTROLLER_USE_DST,
} dma_flow_controller_t;

/// DMA device config.
typedef struct
{
    /// device ID
    dma_id_t        id;
    /// address
    void            *addr;
    /// address control
    dma_addr_ctrl_t addr_ctrl;
    /// bus width
    dma_buswidth_t  bus_width;
    /// burst length, in bus events
    dma_burstlen_t  burst_size;
} dma_dev_t;
    
/// DMA block transfer config.
/**
 * @note  Source/Destination address must be aligned to source/destination bus width.
 *        Block size must be aligned to source/destination (bus_width * burst_size).
 */
typedef struct
{
    /// dma source device
    dma_dev_t       *src;
    /// dma dest device
    dma_dev_t       *dst;
    /// Link list item block length, in bytes.
    /// hs6621:  max src_width_in_bytes * 4095;
    /// hs6621c: max src_width_in_bytes * (2**22 - 1).
    uint32_t        block_size_in_bytes;
    /// Priority
    uint32_t        priority;
    /// specify DMAC, src, dst or none as the flow controller.
    dma_flow_controller_t flow_controller;

    /// interrupt enabling
    bool            intr_en;
} dma_block_config_t;
#endif

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
 * @param[in] trans_count  source bus transfer count
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
    return ch->SrcAddr;
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
    return ch->DstAddr;
}

/**
 * @brief DMA set address increment mode
 *
 * @param[in] ch  DMA channel object
 * @param[in] src_mode  source mode: DWC_CTLL_SRC_INC/DWC_CTLL_SRC_DEC/DWC_CTLL_SRC_FIX
 * @param[in] dst_mode  dest   mode: DWC_CTLL_DST_INC/DWC_CTLL_DST_DEC/DWC_CTLL_DST_FIX
 *
 * @return current dest address
 **/
__STATIC_INLINE void dma_addr_inc_mode(HS_DMA_CH_Type *ch, uint32_t src_mode, uint32_t dst_mode)
{
    ch->Ctrl.SrcAddrCtrl = src_mode;
    ch->Ctrl.DstAddrCtrl = dst_mode;
}
    
/**
 * @brief DMA build block transfer
 *
 * @note  Source/Destination address must be aligned to source/destination bus width.
 *        Block size must be aligned to source/destination (bus_width * burst_size).
 *
 * @param[out] block            Pointer to DMA block transfer object
 * @param[in]  config           dma block config
 *
 * @return Pointer to the block built or NULL when fail.
 **/
dma_block_t * dma_build_block(dma_block_t *block, dma_block_config_t *config);
/* dma_build_block_ex(dma_block_t *block, dma_dev_t *src, dma_dev_t *dst, uint32_t block_size_in_bytes, uint32_t priority, dma_flow_controller_t flow_contrller, bool intr_en); */

/**
 * @brief DMA append block to the tail of a block list.
 *
 * @param[in] list    block list to be appendded to.
 * @param[in] block   block to append.
 *
 * @return Pointer to the list head.
 **/
dma_block_t * dma_append_block(dma_block_t *list, dma_block_t *block);

/**
 * @brief DMA start block transfers
 *
 * @param[in] ch      Pointer to DMA channel object, auto-allocate an available one when NULL is specified.
 * @param[in] block   Block list to transfer.
 * @param[in] cb      Callback function.
 *
 * @return DMA channel used or NULL when fail.
 **/
HS_DMA_CH_Type * dma_start_transfer(HS_DMA_CH_Type *ch, dma_block_t *block, dma_callback_t cb);
    
/*********************************************************************
 * HELPER FUNCTIONS
 */

/**
 * @brief DMA get transfer direction by given source/destination device IDs.
 *
 * @param[in] src_id   DMA source device ID.
 * @param[in] dst_id   DMA destination device ID.
 *
 * @return DMA transfer direction.
 **/
dma_dir_t dma_get_direction(dma_id_t src_id, dma_id_t dst_id);

/**
 * @brief Helper macro for seting up dma_config_t
 * @note  Limited by dma_config_t, only one of src_id and dst_id can be non-memory device.
 *
 * @param[in] config          Config to setup.
 * @param[in] dir             DMA transfer direction.
 * @param[in] dev_id          DMA device ID.
 * @param[in] src_bus_width   DMA srouce bus width.
 * @param[in] dst_bus_width   DMA destination bus width.
 * @param[in] src_burst_len   DMA srouce burst length.
 * @param[in] dst_burst_len   DMA dest burst length.
 * @param[in] flow_ctrl       Flow control of the non-memory device.
 * @param[in] prio            Priority.
 * @param[in] cb              Callback function.
 **/
#define DMA_SETUP_CONFIG(config,               \
                         dir,                  \
                         dev_id,               \
                         src_bus_width,        \
                         dst_bus_width,        \
                         src_burst_len,        \
                         dst_burst_len,        \
                         flow_ctrl,            \
                         prio,                 \
                         cb)                   \
                                               \
    (config).slave_id       = dev_id;          \
    (config).direction      = dir;             \
    (config).src_addr_width = src_bus_width;   \
    (config).dst_addr_width = dst_bus_width;   \
    (config).src_burst      = src_burst_len;   \
    (config).dst_burst      = dst_burst_len;   \
    (config).dev_flow_ctrl  = flow_ctrl;       \
    (config).priority       = prio;            \
    (config).callback       = cb;              \
    (config).lli.enable     = false;

/**
 * @brief Helper macro for seting up dma_lli_t.
 *
 * @param[in] lli             lli to setup.
 * @param[in] lli_enable      lli enable.
 * @param[in] lli_use_fifo    lli use fifo.
 * @param[in] lli_src_addr    lli source address.
 * @param[in] lli_dst_addr    lli destination address.
 * @param[in] lli_block_num   Number of blocks in the lli.
 * @param[in] lli_block_len   Block length of the lli.
 * @param[in] lli_llip        Pointer to object of @p dma_llip_t.
 **/
#define DMA_SETUP_LLI(lli,                     \
                      lli_enable,              \
                      lli_use_fifo,            \
                      lli_src_addr,            \
                      lli_dst_addr,            \
                      lli_block_num,           \
                      lli_block_len,           \
                      lli_llip)                \
                                               \
    (lli).enable     = lli_enable;             \
    (lli).use_fifo   = lli_use_fifo;           \
    (lli).src_addr   = lli_src_addr;           \
    (lli).dst_addr   = lli_dst_addr;           \
    (lli).block_num  = lli_block_num;          \
    (lli).block_len  = lli_block_len;          \
    (lli).llip       = lli_llip;
    

#ifdef __cplusplus
}
#endif

#endif

/** @} */

