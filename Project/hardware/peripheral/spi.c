/**
 * @file spi.c
 * @brief 
 * @date Wed 31 May 2017 07:15:48 PM CST
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"


/*********************************************************************
 * MACROS
 */
#define SPI_TX_FIFO_NUM 32
#define SPI_RX_FIFO_NUM 32

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief spi_dma_dummy_handler()
 *
 * @param[in] status  
 * @param[in] cur_src_addr  
 * @param[in] cur_dst_addr  
 * @param[in] xfer_size  
 *
 * @return 
 **/
static void spi_dma_dummy_handler(dma_status_t status, uint32_t cur_src_addr, uint32_t cur_dst_addr, uint32_t xfer_size)
{

}

/**
 * @brief spi_tx_dma_config()
 *
 * @param[in] spi  
 * @param[in] dma  
 *
 * @return 
 **/
static HS_DMA_CH_Type *spi_tx_dma_config(HS_SPI_Type *spi, HS_DMA_CH_Type *dma, dma_callback_t callback)
{
    if(dma == NULL)
        dma = dma_allocate();

    if(dma)
    {
        bool res;
        dma_config_t dconfig;

        dconfig.slave_id       = spi==HS_SPI0 ? SPI_MST0_TX_DMA_ID : SPI_MST1_TX_DMA_ID;
        dconfig.direction      = DMA_MEM_TO_DEV;
        dconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
        dconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
        dconfig.src_burst      = DMA_BURST_LEN_4UNITS;
        dconfig.dst_burst      = DMA_BURST_LEN_8UNITS;
        dconfig.dev_flow_ctrl  = false;
        dconfig.priority       = 0;
        dconfig.callback       = callback;
        dconfig.lli.enable     = false;

        res = dma_config(dma, &dconfig);
        if(!res)
        {
            dma_release(dma);
            return NULL;
        }

        // Must be (DMATDLR + dst_burst <= 32)
        spi->DMATDLR = 7;
    }

    return dma;
}

/**
 * @brief spi_rx_dma_config()
 *
 * @param[in] spi  
 * @param[in] dma  
 *
 * @return 
 **/
static HS_DMA_CH_Type *spi_rx_dma_config(HS_SPI_Type *spi, HS_DMA_CH_Type *dma, dma_callback_t callback)
{
    if(dma == NULL)
        dma = dma_allocate();

    if(dma)
    {
        bool res;
        dma_config_t dconfig;

        dconfig.slave_id       = spi==HS_SPI0 ? SPI_MST0_RX_DMA_ID : SPI_MST1_RX_DMA_ID;
        dconfig.direction      = DMA_DEV_TO_MEM;
        dconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
        dconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
        dconfig.src_burst      = DMA_BURST_LEN_4UNITS;
        dconfig.dst_burst      = DMA_BURST_LEN_8UNITS;
        dconfig.dev_flow_ctrl  = false;
        dconfig.priority       = 0;
        dconfig.callback       = callback;
        dconfig.lli.enable     = false;

        res = dma_config(dma, &dconfig);
        if(!res)
        {
            dma_release(dma);
            return NULL;
        }

        // Must be (DMARDLR >= src_burst - 1)
#ifdef CONFIG_HS6621
        spi->DMARDLR = 3;
#else
        spi->DMARDLR = 0;
#endif
    }

    return dma;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief spi initialize
 *
 * @param[in] spi  spi object
 * @param[in] mode  work mode
 * @param[in] transmode  transfer mode
 * @param[in] speed  speed in Hz
 *
 * @return None
 **/
void spi_open(HS_SPI_Type *spi, spi_mode_t mode, spi_transmode_t transmode, uint32_t speed)
{
    int div = 0;
    uint32_t master_enable;

    // Open clock
    if(spi == HS_SPI0)
    {
        register_set1(&HS_PSO->SPI0_CFG, CPM_SPI0_SOFT_RESET_MASK);
        register_set0(&HS_PSO->SPI0_CFG, CPM_SPI0_GATE_EN_MASK);
        HS_PSO_UPD_RDY();
    }
    else
    {
        register_set1(&HS_PSO->SPI1_CFG, CPM_SPI1_SOFT_RESET_MASK);
        register_set0(&HS_PSO->SPI1_CFG, CPM_SPI1_GATE_EN_MASK);
        HS_PSO_UPD_RDY();
    }

    if(SPI_MODE_MASTER == mode)
    {
        div = (cpm_get_clock(CPM_SPI_CLK)/2) / speed - 1;
        if (div < 0)
            div = 0;
        master_enable = 1;
    }
    else
    {
        master_enable = 0;
    }

    // Setup mode
    register_set1(&spi->CTRL, SPI_SOFT_RESET_MASK | SPI_RX_CLR_FIFO_MASK | SPI_TX_CLR_FIFO_MASK);

    // CS manual
    register_set1(&spi->CSNCTRL, SPI_CS_MANUAL_MASK | SPI_CS_LEVEL_MASK);

    register_set(&spi->CTRL, MASK_12REG(
                SPI_CLK_DIVIDER, div,
                SPI_MASTER_ENABLE, master_enable,
                SPI_MODE1, (transmode==SPI_TRANS_MODE_0 || transmode==SPI_TRANS_MODE_2) ? 0 : 1,
                SPI_INVERT_CLOCK, (transmode==SPI_TRANS_MODE_0 || transmode==SPI_TRANS_MODE_1) ? 0 : 1,
                SPI_ACTIVE_DO_ENL, 0,
                SPI_INACTIVE_DO_ENL, 0, // 1=SPI_DO pin is high-Z while byte is not being transferred; 0=SPI_DO pin is driven while byte is not being transferred
                SPI_USE_RDY_OUT, 0,
                SPI_BIDIRECT_DATA, 0,
                SPI_MSB_FIRST, 1,
                SPI_TX_FIFO_ENABLE, 1U, // +U to Fix Keil Warning:  #61-D: integer operation result is out of range
                SPI_RX_FIFO_ENABLE, 1,
                SPI_RX_TRIGGER_LEVEL, 0));

    // Start
    register_set0(&spi->CTRL, SPI_SOFT_RESET_MASK | SPI_RX_CLR_FIFO_MASK | SPI_TX_CLR_FIFO_MASK);

    // disable the tx empty and rx trigger interrupt
    register_set1(&spi->STAT, SPI_STAT_TX_EMPTY_INT_EN_MASK | SPI_STAT_RX_TRIG_INT_EN_MASK);
}

/**
 * @brief spi_stop()
 *
 * @param[in] spi  
 *
 * @return 
 **/
void spi_close(HS_SPI_Type *spi)
{
    if(spi == HS_SPI0)
        register_set1(&HS_PSO->SPI0_CFG, CPM_SPI0_GATE_EN_MASK);
    else
        register_set1(&HS_PSO->SPI1_CFG, CPM_SPI1_GATE_EN_MASK);

    HS_PSO_UPD_RDY();
}

/**
 * @brief spi_dma_config()
 *
 * @param[in] spi  
 * @param[in] dma  
 * @param[in] callback  
 *
 * @return 
 **/
void spi_dma_config(HS_SPI_Type *spi, spi_dma_t *dma, dma_callback_t callback)
{
    spi->DMACR = 0;

    dma->callback = callback;
    dma->rx_dma = spi_rx_dma_config(spi, dma->rx_dma, callback);
    dma->tx_dma = spi_tx_dma_config(spi, dma->tx_dma, spi_dma_dummy_handler);
}

/**
 * @brief spi master cs to low
 *
 * @param[in] spi  spi object
 *
 * @return None
 **/
void spi_master_cs_low(HS_SPI_Type *spi)
{
    register_set0(&spi->CSNCTRL, SPI_CS_LEVEL_MASK);
}

/**
 * @brief spi master cs to high
 *
 * @param[in] spi  spi object
 *
 * @return None
 **/
void spi_master_cs_high(HS_SPI_Type *spi)
{
    register_set1(&spi->CSNCTRL, SPI_CS_LEVEL_MASK);
}

/**
 * @brief spi master exchange data with block
 *
 * @param[in] spi  spi object
 * @param[in] txbuf  transmit data buffer, NULL for transmit 0xFF
 * @param[in] rxbuf  receive data buffer, NULL for not receive data
 * @param[in] len  transmit and receive buffer size
 *
 * @return None
 **/
void spi_master_exchange(HS_SPI_Type *spi, const uint8_t *txbuf, uint8_t *rxbuf, uint32_t len)
{
    volatile uint32_t dummy;
    volatile uint32_t spi_stat;
    uint32_t tx_cnt = 0;
    uint32_t rx_cnt = 0;

    while(rx_cnt < len)
    {
        spi_stat = spi->STAT;

        if(tx_cnt < len)
        {
            if(register_get(&spi_stat, MASK_POS(SPI_STAT_TX_BYTE_CNT)) < SPI_TX_FIFO_NUM)
            {
                spi->WDATA = txbuf ? txbuf[tx_cnt] : 0xFF;
                ++tx_cnt;
            }
        }

        if(register_get(&spi_stat, MASK_POS(SPI_STAT_RX_BYTE_CNT)))
        {
            if(rxbuf)
                rxbuf[rx_cnt] = spi->RDATA;
            else
                dummy = spi->RDATA;

            ++rx_cnt;
        }
    }

    (void)dummy;
}

/**
 * @brief spi_master_exchange_dma()
 *
 * @param[in] spi  
 * @param[in] dma  
 * @param[in] txbuf  
 * @param[in] rxbuf  
 * @param[in] len  
 *
 * @return 
 **/
void spi_master_exchange_dma(HS_SPI_Type *spi, spi_dma_t *dma, const uint8_t *txbuf, uint8_t *rxbuf, uint32_t len)
{
    if(len == 0)
        return;

    // clear the dma_rx_req_n signal
    spi->DMACR = 0;
    spi->DMACR = 3;

    // RX DMA
    if(rxbuf)
    {
        dma_addr_inc_mode(dma->rx_dma, DWC_CTLL_SRC_FIX, DWC_CTLL_DST_INC);
    }
    else
    {
        static uint32_t rxtmp;
        rxbuf = (uint8_t *)&rxtmp;
        dma_addr_inc_mode(dma->rx_dma, DWC_CTLL_SRC_FIX, DWC_CTLL_DST_FIX);
    }

    // TX DMA
    if(txbuf)
    {
        dma_addr_inc_mode(dma->tx_dma, DWC_CTLL_SRC_INC, DWC_CTLL_DST_FIX);
    }
    else
    {
        static uint32_t txtmp = 0xFFFFFFFF;
        txbuf = (const uint8_t *)&txtmp;
        dma_addr_inc_mode(dma->tx_dma, DWC_CTLL_SRC_FIX, DWC_CTLL_DST_FIX);
    }

    dma_start(dma->rx_dma, (uint32_t)&spi->RDATA, (uint32_t)rxbuf, len);
    dma_start(dma->tx_dma, (uint32_t)txbuf, (uint32_t)&spi->WDATA, len);

    if(dma->callback == NULL)
    {
        dma_wait_stop(dma->rx_dma);
        dma_wait_stop(dma->tx_dma);
    }
}

/** @} */

