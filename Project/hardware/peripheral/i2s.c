/**
 * @file i2s.c
 * @brief 
 * @date Wed 31 May 2017 07:15:25 PM CST
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
#define __i2s_clr_bit(val, bit)            (val) &= ~(1u<<bit)
#define __i2s_set_bit(val, bit)            (val) |= (1u<<bit)

#define __i2s_set_bitval(val, bit, bitval)                \
    do{                                                   \
        uint32_t mask;                                    \
        mask = 1u<<(bit);                                 \
        (val) = ((val)&~mask) | (((bitval)<<(bit))&mask); \
    }while(0)

#define __i2s_set_bitsval(val, s, e, bitval)              \
    do{                                                   \
        uint32_t mask;                                    \
        mask = ((1u<<((e)-(s)+1)) - 1) << (s);            \
        (val) = ((val)&~mask) | (((bitval)<<(s))&mask);   \
    }while(0)

#define __i2s_set_rxsamplewidth(i2sp, width)      (i2sp)->CHN_R0.RCR = (width)
#define __i2s_set_txsamplewidth(i2sp, width)      (i2sp)->CHN_R0.TCR = (width)
#define __i2s_set_workmode(i2sp, mode)            __i2s_set_bitval((i2sp)->CER, 0, mode)
#define __i2s_set_i2smode(i2sp, mode)             __i2s_set_bitval((i2sp)->IER, 4, mode)

#define __i2s_get_enable(i2sp)                    ((i2sp)->IER & 0x01)
#define __i2s_set_enable(i2sp, en)                __i2s_set_bitval((i2sp)->IER, 0, en)

#define __i2s_enable_rx(i2sp, en)                 (i2sp)->IRER = (en)
#define __i2s_enable_tx(i2sp, en)                 (i2sp)->ITER = (en)

#define __i2s_enable_rxChn(i2sp, en)              (i2sp)->CHN_R0.RER = (en)
#define __i2s_enable_txChn(i2sp, en)              (i2sp)->CHN_R0.TER = (en)

#define __i2s_set_rxFifoDepth(i2sp, depth)        (i2sp)->CHN_R0.RFCR = (depth)
#define __i2s_set_txFifoDepth(i2sp, depth)        (i2sp)->CHN_R0.TFCR = (depth)

#define __i2s_mask_allInt(i2sp)                   (i2sp)->CHN_R0.IMR = 0x33
#define __i2s_unmask_allInt(i2sp)                 (i2sp)->CHN_R0.IMR = 0

#define __i2s_flush_rxFifo(i2sp)                  (i2sp)->CHN_R0.RFF = 1
#define __i2s_flush_txFifo(i2sp)                  (i2sp)->CHN_R0.TFF = 1

#define __i2s_reset_allRxFifo(i2sp)               (i2sp)->RXFFR = 1
#define __i2s_reset_allTxFifo(i2sp)               (i2sp)->TXFFR = 1

#define __i2s_set_wss(i2sp, val)                  __i2s_set_bitsval((i2sp)->CCR, 3, 4, val)   /*(i2sp)->i2s->CCR |= (val) << 3*/
#define __i2s_set_sclkGate(i2sp, val)             __i2s_set_bitsval((i2sp)->CCR, 0, 2, val)   /*(i2sp)->i2s->CCR |= (val) << 0*/

#define __i2s_reset_rxDma(i2sp)                   (i2sp)->RRXDMA = 1
#define __i2s_reset_txDma(i2sp)                   (i2sp)->RTXDMA = 1


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
 * @brief i2s_set_ccr()
 *
 * @param[in] i2s  
 * @param[in] width  
 *
 * @return 
 **/
static void i2s_set_ccr(HS_I2S_Type *i2s, i2s_bitwidth_t width)
{
    uint32_t bit_w;

    switch(width)
    {
        case I2S_BITWIDTH_16BIT: bit_w = 0; break;
        case I2S_BITWIDTH_24BIT: bit_w = 1; break;
        case I2S_BITWIDTH_32BIT: bit_w = 2; break;
        default:                 bit_w = 0; break;
    }

    __i2s_set_wss(i2s, bit_w);
    __i2s_set_sclkGate(i2s, 0);
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief i2s initialize
 *
 * @return None
 **/
void i2s_init(void)
{

}

/**
 * @brief i2s config
 *
 * @param[in] config  configuration @ref i2s_config_t
 *
 * @return None
 **/
void i2s_config(const i2s_config_t *config)
{
    /// cpm i2s config
    REGW(&HS_PSO->I2S_CFG, MASK_2REG(CPM_I2S_TX_AHB_EN,     1, 
                                     CPM_I2S_TX_SOFT_RESET, 1
                                     ));
    HS_PSO_UPD_RDY();

    /// setting i2s clock
    REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_3REG(AU_CODEC_RSTN_REG,      0, 
                                                AU_CODEC_CLK_EN_REG,    1,
                                                AU_I2S_RXCLK_RSTN_REG,  1
                                                ));
#ifdef CONFIG_HS6621
    switch(config->sample_rate)
    {
        case I2S_SAMPLE_8K:
        case I2S_SAMPLE_12K:
        case I2S_SAMPLE_24K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 1200*1000/config->sample_rate/2, 
                                                        AU_I2S_RX_ODD,         0,
                                                        AU_I2S_RX_DIV_EN,      1,
                                                        AU_I2S_RX_DIV_SEL,     1,
                                                        AU_I2S_RX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_16K:
        case I2S_SAMPLE_48K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 1200*1000/config->sample_rate/2+1, 
                                                        AU_I2S_RX_ODD,         1,
                                                        AU_I2S_RX_DIV_EN,      1,
                                                        AU_I2S_RX_DIV_SEL,     1,
                                                        AU_I2S_RX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_32K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 12000*1000/config->sample_rate/2+1, 
                                                        AU_I2S_RX_ODD,         1,
                                                        AU_I2S_RX_DIV_EN,      0,
                                                        AU_I2S_RX_DIV_SEL,     0,
                                                        AU_I2S_RX_DIV_COEFF,   0
                                                        ));
            break;
        case I2S_SAMPLE_11K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 6000*1000/config->sample_rate/2, 
                                                        AU_I2S_RX_ODD,         0,
                                                        AU_I2S_RX_DIV_EN,      1,
                                                        AU_I2S_RX_DIV_SEL,     1,
                                                        AU_I2S_RX_DIV_COEFF,   2
                                                        ));
            break;
        case I2S_SAMPLE_22K:
        case I2S_SAMPLE_44P1K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 12000*1000/config->sample_rate/2, 
                                                        AU_I2S_RX_ODD,         0,
                                                        AU_I2S_RX_DIV_EN,      0,
                                                        AU_I2S_RX_DIV_SEL,     0,
                                                        AU_I2S_RX_DIV_COEFF,   0
                                                        ));
            break;
        default:
            return;
    }   
#else
    switch(config->sample_rate)
    {
        case I2S_SAMPLE_8K:
        case I2S_SAMPLE_16K:
        case I2S_SAMPLE_32K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 1600*1000/config->sample_rate/2, 
                                                        AU_I2S_RX_ODD,         0,
                                                        AU_I2S_RX_DIV_EN,      1,
                                                        AU_I2S_RX_DIV_SEL,     1,
                                                        AU_I2S_RX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_12K:
        case I2S_SAMPLE_24K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 1600*1000/config->sample_rate/2+1, 
                                                        AU_I2S_RX_ODD,         1,
                                                        AU_I2S_RX_DIV_EN,      1,
                                                        AU_I2S_RX_DIV_SEL,     1,
                                                        AU_I2S_RX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_11K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 8000*1000/config->sample_rate/2, 
                                                        AU_I2S_RX_ODD,         0,
                                                        AU_I2S_RX_DIV_EN,      1,
                                                        AU_I2S_RX_DIV_SEL,     1,
                                                        AU_I2S_RX_DIV_COEFF,   2
                                                        ));
            break;
        case I2S_SAMPLE_22K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 8000*1000/config->sample_rate/2+1, 
                                                        AU_I2S_RX_ODD,         1,
                                                        AU_I2S_RX_DIV_EN,      1,
                                                        AU_I2S_RX_DIV_SEL,     1,
                                                        AU_I2S_RX_DIV_COEFF,   2
                                                        ));
            break;
        case I2S_SAMPLE_44P1K:
        case I2S_SAMPLE_48K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_3, MASK_5REG(AU_I2S_RX_HIGH_NUM, 16000*1000/config->sample_rate/2+1, 
                                                        AU_I2S_RX_ODD,         1,
                                                        AU_I2S_RX_DIV_EN,      0,
                                                        AU_I2S_RX_DIV_SEL,     0,
                                                        AU_I2S_RX_DIV_COEFF,   0
                                                        ));
            break;
        default:
            return;
    }   
#endif
    __i2s_set_enable(HS_I2S, 0);
    __i2s_set_i2smode(HS_I2S, config->pcmmode);
    i2s_set_ccr(HS_I2S, config->ws_width);

    __i2s_set_txsamplewidth(HS_I2S, config->sample_width);
    __i2s_enable_tx(HS_I2S, 0);
    __i2s_enable_txChn(HS_I2S, 0);
    __i2s_set_txFifoDepth(HS_I2S, 7);

    __i2s_set_workmode(HS_I2S, config->workmode);
    

    //__i2s_mask_allInt(HS_I2S);
    __i2s_unmask_allInt(HS_I2S);
    __i2s_set_enable(HS_I2S, 0);
}

/**
 * @brief i2s rx config
 *
 * @param[in] config  configuration @ref i2s_config_t
 *
 * @return None
 **/
void i2s_rx_config(const i2s_config_t *config)
{
    /// cpm i2s config
    REGW(&HS_PSO->I2S_CFG, MASK_2REG(CPM_I2S_RX_AHB_EN,     1, 
                                     CPM_I2S_RX_SOFT_RESET, 1
                                     ));
    HS_PSO_UPD_RDY();

    /// setting i2s clock
    REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_3REG(AU_CODEC_RSTN_REG,      1, 
                                                AU_CODEC_CLK_EN_REG,    1,
                                                AU_I2S_TXCLK_RSTN_REG,  1
                                                ));
#ifdef CONFIG_HS6621    
    switch(config->sample_rate)
    {
        case I2S_SAMPLE_8K:
        case I2S_SAMPLE_12K:
        case I2S_SAMPLE_24K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 1200*1000/config->sample_rate/2, 
                                                        AU_I2S_TX_ODD,         0,
                                                        AU_I2S_TX_DIV_EN,      1,
                                                        AU_I2S_TX_DIV_SEL,     1,
                                                        AU_I2S_TX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_16K:
        case I2S_SAMPLE_48K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 1200*1000/config->sample_rate/2+1, 
                                                        AU_I2S_TX_ODD,         1,
                                                        AU_I2S_TX_DIV_EN,      1,
                                                        AU_I2S_TX_DIV_SEL,     1,
                                                        AU_I2S_TX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_32K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 12000*1000/config->sample_rate/2+1, 
                                                        AU_I2S_TX_ODD,         1,
                                                        AU_I2S_TX_DIV_EN,      0,
                                                        AU_I2S_TX_DIV_SEL,     0,
                                                        AU_I2S_TX_DIV_COEFF,   0
                                                        ));
            break;
        case I2S_SAMPLE_11K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 6000*1000/config->sample_rate/2, 
                                                        AU_I2S_TX_ODD,         0,
                                                        AU_I2S_TX_DIV_EN,      1,
                                                        AU_I2S_TX_DIV_SEL,     1,
                                                        AU_I2S_TX_DIV_COEFF,   2
                                                        ));
            break;
        case I2S_SAMPLE_22K:
        case I2S_SAMPLE_44P1K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 12000*1000/config->sample_rate/2, 
                                                        AU_I2S_TX_ODD,         0,
                                                        AU_I2S_TX_DIV_EN,      0,
                                                        AU_I2S_TX_DIV_SEL,     0,
                                                        AU_I2S_TX_DIV_COEFF,   0
                                                        ));
            break;
        default:
            return;
    }   
#else
    switch(config->sample_rate)
    {
        case I2S_SAMPLE_8K:
        case I2S_SAMPLE_16K:
        case I2S_SAMPLE_32K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 1600*1000/config->sample_rate/2, 
                                                        AU_I2S_TX_ODD,         0,
                                                        AU_I2S_TX_DIV_EN,      1,
                                                        AU_I2S_TX_DIV_SEL,     1,
                                                        AU_I2S_TX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_12K:
        case I2S_SAMPLE_24K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 1600*1000/config->sample_rate/2+1, 
                                                        AU_I2S_TX_ODD,         1,
                                                        AU_I2S_TX_DIV_EN,      1,
                                                        AU_I2S_TX_DIV_SEL,     1,
                                                        AU_I2S_TX_DIV_COEFF, 0xA
                                                        ));
            break;
        case I2S_SAMPLE_11K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 8000*1000/config->sample_rate/2, 
                                                        AU_I2S_TX_ODD,         0,
                                                        AU_I2S_TX_DIV_EN,      1,
                                                        AU_I2S_TX_DIV_SEL,     1,
                                                        AU_I2S_TX_DIV_COEFF,   2
                                                        ));
            break;
        case I2S_SAMPLE_22K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 8000*1000/config->sample_rate/2+1, 
                                                        AU_I2S_TX_ODD,         1,
                                                        AU_I2S_TX_DIV_EN,      1,
                                                        AU_I2S_TX_DIV_SEL,     1,
                                                        AU_I2S_TX_DIV_COEFF,   2
                                                        ));
            break;
        case I2S_SAMPLE_44P1K:
        case I2S_SAMPLE_48K:
            REGW(&HS_AUDIO->CODEC_CLK_CTRL_2, MASK_5REG(AU_I2S_TX_HIGH_NUM, 16000*1000/config->sample_rate/2+1, 
                                                        AU_I2S_TX_ODD,         1,
                                                        AU_I2S_TX_DIV_EN,      0,
                                                        AU_I2S_TX_DIV_SEL,     0,
                                                        AU_I2S_TX_DIV_COEFF,   0
                                                        ));
            break;
        default:
            return;
    }   
#endif

    switch(config->sample_rate)
    {
        case I2S_SAMPLE_8K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 1, 
                                                AU_ADC_SR_44K, 0,
                                                AU_ADC_SR_48K, 0,
                                                AU_ADC_SR_4X,  1,
                                                AU_ADC_SR_2X,  0,
                                                AU_ADC_SR_1X,  0
                                                ));
            break;
        case I2S_SAMPLE_11K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 0, 
                                                AU_ADC_SR_44K, 1,
                                                AU_ADC_SR_48K, 0,
                                                AU_ADC_SR_4X,  1,
                                                AU_ADC_SR_2X,  0,
                                                AU_ADC_SR_1X,  0
                                                ));
            break;
        case I2S_SAMPLE_12K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 0, 
                                                AU_ADC_SR_44K, 0,
                                                AU_ADC_SR_48K, 1,
                                                AU_ADC_SR_4X,  1,
                                                AU_ADC_SR_2X,  0,
                                                AU_ADC_SR_1X,  0
                                                ));
            break;
        case I2S_SAMPLE_16K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 1, 
                                                AU_ADC_SR_44K, 0,
                                                AU_ADC_SR_48K, 0,
                                                AU_ADC_SR_4X,  0,
                                                AU_ADC_SR_2X,  1,
                                                AU_ADC_SR_1X,  0
                                                ));
            break;
        case I2S_SAMPLE_22K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 0, 
                                                AU_ADC_SR_44K, 1,
                                                AU_ADC_SR_48K, 0,
                                                AU_ADC_SR_4X,  0,
                                                AU_ADC_SR_2X,  1,
                                                AU_ADC_SR_1X,  0
                                                ));
            break;
        case I2S_SAMPLE_24K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 0, 
                                                AU_ADC_SR_44K, 0,
                                                AU_ADC_SR_48K, 1,
                                                AU_ADC_SR_4X,  0,
                                                AU_ADC_SR_2X,  1,
                                                AU_ADC_SR_1X,  0
                                                ));
            break;
        case I2S_SAMPLE_32K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 1, 
                                                AU_ADC_SR_44K, 0,
                                                AU_ADC_SR_48K, 0,
                                                AU_ADC_SR_4X,  0,
                                                AU_ADC_SR_2X,  0,
                                                AU_ADC_SR_1X,  1
                                                ));
            break;
        case I2S_SAMPLE_44P1K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 1, 
                                                AU_ADC_SR_44K, 0,
                                                AU_ADC_SR_48K, 0,
                                                AU_ADC_SR_4X,  0,
                                                AU_ADC_SR_2X,  0,
                                                AU_ADC_SR_1X,  1
                                                ));
            break;
        case I2S_SAMPLE_48K:
            REGW(&HS_AUDIO->ADC_CTRL, MASK_6REG(AU_ADC_SR_32K, 0, 
                                                AU_ADC_SR_44K, 0,
                                                AU_ADC_SR_48K, 1,
                                                AU_ADC_SR_4X,  0,
                                                AU_ADC_SR_2X,  0,
                                                AU_ADC_SR_1X,  1
                                                ));
            break;
        default:
            return;
    } 
    __i2s_set_enable(HS_I2S_RX, 0);
    __i2s_set_i2smode(HS_I2S_RX, config->pcmmode);
    i2s_set_ccr(HS_I2S_RX, config->ws_width);

    __i2s_set_rxsamplewidth(HS_I2S_RX, config->sample_width);
    __i2s_enable_rx(HS_I2S_RX, 0);
    __i2s_enable_rxChn(HS_I2S_RX, 0);
    //__i2s_reset_allRxFifo(HS_I2S_RX); 
    //__i2s_flush_rxFifo(HS_I2S_RX);
    __i2s_set_rxFifoDepth(HS_I2S_RX, 7);

    __i2s_set_workmode(HS_I2S_RX, config->workmode);

    //__i2s_mask_allInt(HS_I2S);
    __i2s_unmask_allInt(HS_I2S_RX);
    __i2s_set_enable(HS_I2S_RX, 0);
}

/**
 * @brief adc_dma_config()
 *
 * @param[in] dma  NULL: ADC will allocate a new dma; Not NULL: use it as ADC dma
 * @param[in] config  configuration @ref i2s_dma_config_t
 *
 * @retval NULL No DMA valid or error happen
 * @retval NOT_NULL: Current used DMA
 **/
HS_DMA_CH_Type *i2s_dma_config(HS_DMA_CH_Type *dma, const i2s_dma_config_t *config)
{
    if(dma == NULL)
        dma = dma_allocate();

    if(dma)
    {
        bool res;
        dma_config_t dconfig;

        dconfig.slave_id       = I2S_TX_DMA_ID;
        dconfig.direction      = DMA_MEM_TO_DEV;
        dconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
        dconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
        dconfig.src_burst      = DMA_BURST_LEN_1UNITS;
        dconfig.dst_burst      = DMA_BURST_LEN_1UNITS;
        dconfig.dev_flow_ctrl  = false;
        dconfig.priority       = 0;
        dconfig.callback       = config->callback;

        dconfig.lli.enable     = true;
        dconfig.lli.use_fifo   = config->use_fifo;
        dconfig.lli.src_addr   = (uint32_t)config->buffer;
        dconfig.lli.dst_addr   = (uint32_t)&HS_I2S->TXDMA;
        dconfig.lli.block_num  = config->block_num;
        dconfig.lli.block_len  = config->buffer_len / config->block_num;
        dconfig.lli.llip       = config->block_llip;

        res = dma_config(dma, &dconfig);
        if(!res)
        {
            dma_release(dma);
            return NULL;
        }

        __i2s_reset_txDma(HS_I2S);
    }

    return dma;
}

/**
 * @brief i2s_rx_dma_config() for rx
 *
 * @param[in] dma  NULL: ADC will allocate a new dma; Not NULL: use it as ADC dma
 * @param[in] config  configuration @ref i2s_dma_config_t
 *
 * @retval NULL No DMA valid or error happen
 * @retval NOT_NULL: Current used DMA
 **/
HS_DMA_CH_Type *i2s_rx_dma_config(HS_DMA_CH_Type *dma, const i2s_dma_config_t *config)
{
    if(dma == NULL)
        dma = dma_allocate();

    if(dma)
    {
        bool res;
        dma_config_t dconfig;

        dconfig.slave_id       = I2S_RX_DMA_ID;
        dconfig.direction      = DMA_DEV_TO_MEM;
        dconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
        dconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
        dconfig.src_burst      = DMA_BURST_LEN_1UNITS;
        dconfig.dst_burst      = DMA_BURST_LEN_1UNITS;
        dconfig.dev_flow_ctrl  = false;
        dconfig.priority       = 0;
        dconfig.callback       = config->callback;

        dconfig.lli.enable     = true;
        dconfig.lli.use_fifo   = config->use_fifo;
        dconfig.lli.src_addr   = (uint32_t)&HS_I2S_RX->RXDMA;
        dconfig.lli.dst_addr   = (uint32_t)config->buffer;
        dconfig.lli.block_num  = config->block_num;
        dconfig.lli.block_len  = config->buffer_len / config->block_num;
        dconfig.lli.llip       = config->block_llip;

        res = dma_config(dma, &dconfig);
        if(!res)
        {
            dma_release(dma);
            return NULL;
        }

        __i2s_reset_rxDma(HS_I2S_RX);
    }

    return dma;
}

/**
 * @brief i2s start
 *
 * @return None
 **/
void i2s_start(void)
{
    __i2s_set_enable(HS_I2S, 1);
    __i2s_enable_tx(HS_I2S, 0);
    __i2s_enable_txChn(HS_I2S, 0);

    __i2s_reset_allTxFifo(HS_I2S);
    __i2s_flush_txFifo(HS_I2S);

    __i2s_enable_tx(HS_I2S, 1);
    __i2s_enable_txChn(HS_I2S, 1);

    pmu_lowpower_prevent(PMU_LP_I2S);

    REGW(&HS_AUDIO->IF_CTRL, MASK_2REG(AU_I2S_RX_MS_SEL,      0,
                                        AU_RECEIVE_EN,       1));                               
}

/**
 * @brief i2s start rx
 *
 * @return None
 **/
void i2s_rx_start(const i2s_config_t *config)
{
    __i2s_set_enable(HS_I2S_RX, 1);
    __i2s_enable_rx(HS_I2S_RX, 0);
    __i2s_enable_rxChn(HS_I2S_RX, 0);

    __i2s_reset_allTxFifo(HS_I2S_RX);
    __i2s_flush_rxFifo(HS_I2S_RX);

    __i2s_enable_rx(HS_I2S_RX, 1);
    __i2s_enable_rxChn(HS_I2S_RX, 1);
    
    if (config->sample_width == I2S_BITWIDTH_16BIT)
    {
        REGW(&HS_AUDIO->ADC_CTRL, MASK_5REG(AU_ADC_24B_EN, 0, 
                                            AU_ADC_CIC_SCALE, 0x0,
                                            AU_ADC_DC_EN, 1,
                                            AU_ADC_SW_RESET_X,  1,
                                            AU_ADC_EN,  1
                                            ));
    }
    else
    {
        REGW(&HS_AUDIO->ADC_CTRL, MASK_5REG(AU_ADC_24B_EN, 1, 
                                            AU_ADC_CIC_SCALE, 0x0,
                                            AU_ADC_DC_EN, 1,
                                            AU_ADC_SW_RESET_X,  1,
                                            AU_ADC_EN,  1
                                            ));
    }
    
    co_delay_us(100); // must delay for modify AU_ADC_ANTI_CLIP
    REGW(&HS_AUDIO->ADC_CTRL, MASK_1REG(AU_ADC_ANTI_CLIP, 1)); 
    
    REGW(&HS_AUDIO->IF_CTRL, MASK_2REG(AU_I2S_TX_MS_SEL,     1,
                                        AU_TRANSMIT_EN,       1));

    pmu_lowpower_prevent(PMU_LP_I2S);
}

/**
 * @brief i2s stop
 *
 * @return None
 **/
void i2s_stop(void)
{
    __i2s_enable_tx(HS_I2S, 0);
    __i2s_enable_txChn(HS_I2S, 0);
    __i2s_set_enable(HS_I2S, 0);

        /// cpm i2s config
    REGW(&HS_PSO->I2S_CFG, MASK_2REG(CPM_I2S_TX_AHB_EN,     0, 
                                     CPM_I2S_TX_SOFT_RESET, 0
                                     ));
    HS_PSO_UPD_RDY();
    
    REGW(&HS_AUDIO->IF_CTRL, MASK_2REG(AU_I2S_RX_MS_SEL,      1,
                                        AU_RECEIVE_EN,       0));    
    /// setting i2s clock
    if (register_get(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_POS(AU_I2S_TXCLK_RSTN_REG)))
    {
        REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_1REG(AU_I2S_RXCLK_RSTN_REG,  0));
    }
    else
    {
        REGW(&HS_AUDIO->ADC_CTRL, MASK_4REG(AU_ADC_24B_EN, 0, 
                                        AU_ADC_DC_EN, 0,
                                        AU_ADC_SW_RESET_X,  0,
                                        AU_ADC_EN,  0
                                        ));
        REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_3REG(AU_CODEC_RSTN_REG,      0, 
                                                    AU_CODEC_CLK_EN_REG,    0,
                                                    AU_I2S_RXCLK_RSTN_REG,  0
                                                    ));
        pmu_lowpower_allow(PMU_LP_I2S);
    }
}

/**
 * @brief i2s stop rx
 *
 * @return None
 **/
void i2s_rx_stop(void)
{
    __i2s_enable_rx(HS_I2S_RX, 0);
    __i2s_enable_rxChn(HS_I2S_RX, 0);
    __i2s_set_enable(HS_I2S_RX, 0);

    /// cpm i2s config
    REGW(&HS_PSO->I2S_CFG, MASK_2REG(CPM_I2S_RX_AHB_EN,     0, 
                                     CPM_I2S_RX_SOFT_RESET, 0
                                     ));
    HS_PSO_UPD_RDY();
    REGW(&HS_AUDIO->ADC_CTRL, MASK_4REG(AU_ADC_24B_EN, 0, 
                                        AU_ADC_DC_EN, 0,
                                        AU_ADC_SW_RESET_X,  0,
                                        AU_ADC_EN,  0
                                        ));
    REGW(&HS_AUDIO->IF_CTRL, MASK_2REG(AU_I2S_TX_MS_SEL,     1,
                                        AU_TRANSMIT_EN,       0));    

    /// setting i2s clock
    if (register_get(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_POS(AU_I2S_RXCLK_RSTN_REG)))
    {
        REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_1REG(AU_I2S_TXCLK_RSTN_REG,  0));
    }
    else
    {
        REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_3REG(AU_CODEC_RSTN_REG,      0, 
                                                    AU_CODEC_CLK_EN_REG,    0,
                                                    AU_I2S_TXCLK_RSTN_REG,  0
                                                    ));
        pmu_lowpower_allow(PMU_LP_I2S);
    }
}

/** @} */


