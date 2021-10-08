/**
 * @file dma.c
 * @brief 
 * @date Tue 16 May 2017 02:52:40 PM CST
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details DMA driver, Used for HS6621A/HS6621P
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
#define DMA_CH_NUM          4
#define DMA_CH_MASK         ((1u<<DMA_CH_NUM) - 1)

#define DMA_CH2INDEX(ch)    ((((uint32_t)ch) - HS_DMA_CH0_BASE) / (HS_DMA_CH1_BASE - HS_DMA_CH0_BASE))
#define DMA_INDEX2CH(i)     ((HS_DMA_CH_Type *)(HS_DMA_CH0_BASE + i * (HS_DMA_CH1_BASE - HS_DMA_CH0_BASE)))

#define DMA_CLEAR_BITS(reg, mask)   ((reg) = ((mask << 8) | 0))
#define DMA_SET_BITS(reg, mask)     ((reg) = ((mask << 8) | (mask)))

#define DMA_MAX_TRANS_COUNT  4095

/*********************************************************************
 * TYPEDEFS
 */
enum
{
    DW_DMA_FC_D_M2M,
    DW_DMA_FC_D_M2P,
    DW_DMA_FC_D_P2M,
    DW_DMA_FC_D_P2P,
    DW_DMA_FC_P_P2M,
    DW_DMA_FC_SP_P2P,
    DW_DMA_FC_P_M2P,
    DW_DMA_FC_DP_P2P,
};

typedef struct
{
    uint32_t        allocated_mask;
    dma_callback_t  callback[DMA_CH_NUM];
}dma_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static dma_env_t dma_env = {0};

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief _dma_setup_lli()
 *
 * @param[in] lli  
 * @param[in] ctrllo  
 *
 * @return 
 **/
static bool dma_setup_lli(dma_config_t *config, uint32_t ctrllo)
{
    dma_lli_t *lli = &config->lli;
    dma_llip_t *llip = config->lli.llip;
    uint32_t i;
    uint32_t trans_count;

    for(i=0; i<lli->block_num; i++)
    {
        switch(config->direction)
        {
            case DMA_MEM_TO_MEM:
                llip[i].sar = lli->src_addr + i * lli->block_len;
                llip[i].dar = lli->dst_addr + i * lli->block_len;
                trans_count = lli->block_len / (1<<config->dst_addr_width);
                break;

            case DMA_MEM_TO_DEV:
                llip[i].sar = lli->src_addr + i * lli->block_len;
                llip[i].dar = lli->dst_addr;
                trans_count = lli->block_len / (1<<config->dst_addr_width);
                break;

            case DMA_DEV_TO_MEM:
                llip[i].sar = lli->src_addr;
                llip[i].dar = lli->dst_addr + i * lli->block_len;
                trans_count = lli->block_len / (1<<config->src_addr_width);
                break;

            default:
                llip[i].llp = 0;
                return false;
        }

        if(trans_count > DMA_MAX_TRANS_COUNT)
            return false;

        llip[i].ctlhi = trans_count;
        llip[i].ctllo = ctrllo;
        llip[i].llp = (uint32_t)&llip[i+1];
    }

    if(lli->use_fifo)
    {
        llip[lli->block_num-1].llp = (uint32_t)&llip[0];
    }
    else
    {
        llip[lli->block_num-1].llp = 0;
    }

    return true;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief DMA initialize
 *
 * @return None
 **/
void dma_init(void)
{
//    memset(&dma_env, 0, sizeof(dma_env_t));

    // Reset and Open clock
    register_set1(&HS_PSO->DMA_CFG, CPM_DMA_SOFT_RESET_MASK);
    register_set0(&HS_PSO->DMA_CFG, CPM_DMA_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    /* disable dma*/
    HS_DMAC->CFG = 0;

    /* disable interrupts*/
    DMA_CLEAR_BITS(HS_DMAC->MASK.XFER, DMA_CH_MASK);
    DMA_CLEAR_BITS(HS_DMAC->MASK.SRC_TRAN, DMA_CH_MASK);
    DMA_CLEAR_BITS(HS_DMAC->MASK.DST_TRAN, DMA_CH_MASK);
    DMA_CLEAR_BITS(HS_DMAC->MASK.ERROR, DMA_CH_MASK);
    DMA_CLEAR_BITS(HS_DMAC->MASK.BLOCK, DMA_CH_MASK);

    DMA_CLEAR_BITS(HS_DMAC->CH_EN, DMA_CH_MASK);

    /* Clear all interrupts on all channels. */
    HS_DMAC->CLEAR.XFER = DMA_CH_MASK;
    HS_DMAC->CLEAR.BLOCK = DMA_CH_MASK;
    HS_DMAC->CLEAR.SRC_TRAN = DMA_CH_MASK;
    HS_DMAC->CLEAR.DST_TRAN = DMA_CH_MASK;
    HS_DMAC->CLEAR.ERROR = DMA_CH_MASK;

//    int r = DMA_CH_NUM - i - 1;
//    max_block_size = (4 << ((dmac->MAX_BLK_SIZE >> 4 * i) & 0xf)) - 1;
//    lli_support = (dmac->DWC_PARAMS[r] >> DWC_PARAMS_MBLK_EN & 0x1);
//    max_burst_len = (((dmac->DWC_PARAMS[r] >> DWC_PARAMS_MAX_MULT_SIZE) & 0x7) + 2) << 1;

    /* enable */
    HS_DMAC->CFG = 1;
}

/**
 * @brief DMA allocate a new channel object
 *
 * @retval NOTNULL new DMA object
 * @retval NULL No DMA to allocate
 **/
HS_DMA_CH_Type *dma_allocate(void)
{
    int i;

    for(i=0; i<DMA_CH_NUM; ++i)
    {
        if((dma_env.allocated_mask & (1u<<i)) == 0)
        {
            dma_env.allocated_mask |= 1u<<i;
            return DMA_INDEX2CH(i);
        }
    }

    return NULL;
}

/**
 * @brief DMA release a channel object
 *
 * @param[in] ch  DMA channel object (return by dma_allocate)
 *
 * @return None
 **/
void dma_release(HS_DMA_CH_Type *ch)
{
    dma_env.allocated_mask &= ~(1u<<DMA_CH2INDEX(ch));
}

/**
 * @brief DMA config
 *
 * @param[in] ch  DMA channel object
 * @param[in] config  dma configurtaion
 *
 * @retval true success
 * @retval false fail
 **/
bool dma_config(HS_DMA_CH_Type *ch, dma_config_t *config)
{
    uint32_t ctllo=0, cfghi=0, cfglo;
    uint32_t ch_index = DMA_CH2INDEX(ch);

    ctllo = DWC_CTLL_DST_MSIZE(config->dst_burst) | DWC_CTLL_SRC_MSIZE(config->src_burst);
    cfglo = DWC_CFGL_CH_PRIOR(config->priority);

    if(config->lli.enable)
    {
        ctllo = DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;
    }

    switch (config->direction)
    {
        case DMA_MEM_TO_MEM:
            ctllo |= 0
                | DWC_CTLL_DST_WIDTH(config->dst_addr_width)
                | DWC_CTLL_SRC_WIDTH(config->src_addr_width)
                | DWC_CTLL_DMS(1)
                | DWC_CTLL_SMS(0)
                | DWC_CTLL_DST_INC
                | DWC_CTLL_SRC_INC
                | DWC_CTLL_FC_M2M;
            cfghi = DWC_CFGH_FCMODE;
            cfglo |= DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
            return true;

        case DMA_MEM_TO_DEV:
            ctllo |= 0
                | DWC_CTLL_DST_WIDTH(config->dst_addr_width)
                | DWC_CTLL_SRC_WIDTH(config->src_addr_width)
                | DWC_CTLL_DMS(1)
                | DWC_CTLL_SMS(0)
                | DWC_CTLL_DST_FIX
                | DWC_CTLL_SRC_INC
                | (config->dev_flow_ctrl ? DWC_CTLL_FC(DW_DMA_FC_P_M2P) :
                        DWC_CTLL_FC(DW_DMA_FC_D_M2P));
            cfghi = DWC_CFGH_DST_PER(config->slave_id) | DWC_CFGH_FCMODE ;
            cfglo |= DWC_CFGL_HS_SRC;
            break;

        case DMA_DEV_TO_MEM:
            ctllo |= 0
                | DWC_CTLL_DST_WIDTH(config->dst_addr_width)
                | DWC_CTLL_SRC_WIDTH(config->src_addr_width)
                | DWC_CTLL_DMS(0)
                | DWC_CTLL_SMS(1)
                | DWC_CTLL_DST_INC
                | DWC_CTLL_SRC_FIX
                | (config->dev_flow_ctrl ? DWC_CTLL_FC(DW_DMA_FC_P_P2M) :
                        DWC_CTLL_FC(DW_DMA_FC_D_P2M));
            cfghi = DWC_CFGH_SRC_PER(config->slave_id) | DWC_CFGH_FCMODE ;
            cfglo |= DWC_CFGL_HS_DST;
            break;

        default:
            break;
    }

    ctllo |= DWC_CTLL_INT_EN;

    if(config->lli.enable)
    {
        bool res = dma_setup_lli(config, ctllo);
        if(!res)
            return false;
        ch->LLP = (uint32_t)config->lli.llip;
        ch->CTL_HI = config->lli.llip->ctlhi;
    }
    else
    {
        ch->LLP = 0;
    }

    ch->CTL_LO = ctllo;
    ch->CFG_LO = cfglo;
    ch->CFG_HI = cfghi;

    dma_env.callback[ch_index] = config->callback;

    if(config->callback)
    {
        uint32_t ch_mask = 1<<ch_index;

        DMA_SET_BITS(HS_DMAC->MASK.XFER, ch_mask);
        DMA_SET_BITS(HS_DMAC->MASK.BLOCK, ch_mask);
        DMA_SET_BITS(HS_DMAC->MASK.ERROR, ch_mask);

        NVIC_SetPriority(DMA_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(DMA_IRQn);
        NVIC_EnableIRQ(DMA_IRQn);

        // some clock auto gate when WFI (DMA should open)
        if(HS_PMU->SW_STATUS & PMU_SW_STATUS_AGGRESSIVE_CPUGATE_MASK)
            REGW0(HS_SYS->PINMUX, SYS_PINMUX_SYSPLL_GT_CPUCLK_HW_CTRL_MASK);
    }

    return true;
}

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
bool dma_start(HS_DMA_CH_Type *ch, uint32_t src_addr, uint32_t dst_addr, uint32_t trans_count)
{
    uint32_t ch_index = DMA_CH2INDEX(ch);
    uint32_t ch_mask = 1u << ch_index;

    if(trans_count > DMA_MAX_TRANS_COUNT)
        return false;

    ch->SAR    = src_addr;
    ch->DAR    = dst_addr;
    ch->CTL_HI = trans_count;

    DMA_SET_BITS(HS_DMAC->CH_EN, ch_mask);

    pmu_lowpower_prevent(PMU_LP_DMA_CH0<<ch_index);

    return true;
}

/**
 * @brief DMA start with LINK LIST
 *
 * @param[in] ch  DMA channel object
 *
 * @return None
 **/
void dma_start_with_lli(HS_DMA_CH_Type *ch)
{
    uint32_t ch_index = DMA_CH2INDEX(ch);
    uint32_t ch_mask = 1u << ch_index;

    DMA_SET_BITS(HS_DMAC->CH_EN, ch_mask);

    pmu_lowpower_prevent(PMU_LP_DMA_CH0<<ch_index);
}

/**
 * @brief DMA stop
 *
 * @param[in] ch  DMA channel object
 *
 * @return None
 **/
void dma_stop(HS_DMA_CH_Type *ch)
{
    uint32_t ch_index = DMA_CH2INDEX(ch);
    uint32_t ch_mask = 1u << ch_index;

    DMA_CLEAR_BITS(HS_DMAC->CH_EN, ch_mask);
    while(HS_DMAC->CH_EN & ch_mask);

    HS_DMAC->CLEAR.XFER = ch_mask;
    HS_DMAC->CLEAR.ERROR = ch_mask;

    pmu_lowpower_allow(PMU_LP_DMA_CH0<<ch_index);
}

/**
 * @brief DMA wait stop
 *
 * @param[in] ch  DMA channel object
 *
 * @return None
 **/
void dma_wait_stop(HS_DMA_CH_Type *ch)
{
    uint32_t ch_index = DMA_CH2INDEX(ch);
    uint32_t ch_mask = 1u << ch_index;

    while(HS_DMAC->CH_EN & ch_mask);

    HS_DMAC->CLEAR.XFER = ch_mask;
    HS_DMAC->CLEAR.ERROR = ch_mask;

    pmu_lowpower_allow(PMU_LP_DMA_CH0<<ch_index);
}

void DMA_IRQHandler(void)
{
    uint32_t i, status_xfer, status_err, status_block;

    status_xfer  = HS_DMAC->RAW.XFER;
    status_block = HS_DMAC->RAW.BLOCK;
    status_err   = HS_DMAC->RAW.ERROR;

    for (i=0; i<DMA_CH_NUM; i++)
    {
        uint32_t mask = 1<<i;
        HS_DMA_CH_Type *ch = DMA_INDEX2CH(i);
        dma_status_t status = DMA_STATUS_UNKNOWN;

        if(status_block & mask)
        {
            HS_DMAC->CLEAR.BLOCK = mask;
            status = DMA_STATUS_BLOCK_OK;
        }

        if(status_xfer & mask)
        {
            HS_DMAC->CLEAR.XFER = mask;
            status = DMA_STATUS_XFER_OK;

            if (ch->LLP == 0)
                pmu_lowpower_allow(PMU_LP_DMA_CH0<<i);
        }

        if (status_err & mask)
        {
            HS_DMAC->CLEAR.ERROR = mask;
            status = DMA_STATUS_ERROR;
        }

        if(status != DMA_STATUS_UNKNOWN)
        {
            if (dma_env.callback[i])
                dma_env.callback[i](status, ch->SAR, ch->DAR, ch->CTL_HI & 0x0FFF);
        }
    }
}

/** @} */


