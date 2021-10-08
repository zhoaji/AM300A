/**
 * @file sf_base.c
 * @brief 
 * @date Mon, Jul  8, 2019  2:10:02 PM
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
#define SFB_DONE               0x01
#define SFB_CMD_READ           1
#define SFB_CMD_WRITE          2

#ifdef CONFIG_SF_XJF_DESIGN
#define SFB_NODMA_WRITE_DATA_LEN_MAX 8
#define SFB_NODMA_READ_DATA_LEN_MAX  4
#else
#define SFB_NODMA_WRITE_DATA_LEN_MAX 8
#define SFB_NODMA_READ_DATA_LEN_MAX  8
#endif

#define DIV_ROUND_UP(n,d)     (((n) + (d) - 1) / (d))

/*********************************************************************
 * TYPEDEFS
 */
#pragma pack(1)
#ifdef CONFIG_HS6621
typedef struct __spi_cmd_t
{
    uint32_t cmd:2;
    uint32_t rfu0:2;
    uint32_t cs:2;
    uint32_t keepCs:1;
    uint32_t rfu1:1;

    uint32_t cmdBits:7;
    uint32_t rfu2:1;

    uint32_t dataBytes:16;
}spi_cmd_t;
#else
typedef struct __spi_cmd_t
{
    uint32_t cmd:2;
#ifdef CONFIG_SF_XJF_DESIGN
    uint32_t cs:1;
    uint32_t lcd2lane:1;
#else
    uint32_t cs:2;
#endif
    uint32_t keepCs:1;
    uint32_t cmdBits:7;
    uint32_t dataBytes:20;
}spi_cmd_t;
#endif
#pragma pack()

typedef struct __sfb_env_t
{
    // critical object
    HS_SF_Type *critical_obj;

    // callback
    sfb_callback_t callback[SFB_MODULE_NUM];

    // Configuration save space
    struct {
        uint32_t ctrl;
        uint32_t cs;
    }configuration_save[SFB_MODULE_NUM][SFB_CS_NUM];

#ifdef CONFIG_SF_XJF_DESIGN
    // 2lane mode for LCD
    uint8_t lcd2lane[SFB_MODULE_NUM][SFB_CS_NUM];
#endif
}sfb_env_t;

typedef struct __sfb_friend_t
{
    __IO uint32_t *cpm_cfg;
    cpm_clk_t      cpm_type;
    IRQn_Type      irq;
} sfb_friend_t;

/*********************************************************************
 * CONSTANTS
 */
static const sfb_friend_t sfb_friend_tbl[SFB_MODULE_NUM] =
{
    {&HS_PSO->SF0_CFG, CPM_SF0_CLK, SF_IRQn},
    {&HS_PSO->SF1_CFG, CPM_SF1_CLK, SF1_IRQn},
#if SFB_MODULE_NUM > 2
    {&HS_PSO->SF2_CFG, CPM_SF2_CLK, SF2_IRQn},
#endif
#if SFB_MODULE_NUM > 3
    {&HS_PSO->SF3_CFG, CPM_SF3_CLK, SF3_IRQn},
#endif
};

/*********************************************************************
 * LOCAL VARIABLES
 */
static sfb_env_t sfb_env = {
#ifdef CONFIG_XIP_FLASH_ALL
    /* critical_obj */  HS_SF,
#else
    /* critical_obj */  NULL,
#endif
    /* ... */
};


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
/**
 * @brief  sfb workaround hs6621 cs1 issue
 *
 * @param[in] viud  viud
 **/
static void sfb_workaround_hs6621_cs1_issue(void)
{
    REGW0(&HS_PSO->TIM_CFG[2], CPM_TIMER_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
    HS_TIM2->CR1 = TIM_CR1_ARPE;
    HS_TIM2->CR2 = 0;
    HS_TIM2->CCMR1 = _TIM_CCMR1_OC2M(4) | TIM_CCMR1_OC2PE;
    HS_TIM2->CCER = TIM_CCER_CCE(1) | TIM_CCER_CC2NE;
    HS_TIM2->BDTR = TIM_BDTR_MOE;
}
#endif

/**
 * @brief  sfb cpm
 *
 * @param[in] sf  sf
 *
 * @return cpm
 **/
static __IO uint32_t *sfb_cpm(HS_SF_Type *sf)
{
    return sfb_friend_tbl[sfb_index(sf)].cpm_cfg;
}

/**
 * @brief  sfb clk
 *
 * @param[in] sf  sf
 *
 * @return clk
 **/
static uint32_t sfb_clk(HS_SF_Type *sf)
{
    return cpm_get_clock(sfb_friend_tbl[sfb_index(sf)].cpm_type);
}

/**
 * @brief  sfb irqn
 *
 * @param[in] sf  sf
 *
 * @return IRQn
 **/
static IRQn_Type sfb_irqn(HS_SF_Type *sf)
{
    return sfb_friend_tbl[sfb_index(sf)].irq;
}

/**
 * @brief sfb_is_opened()
 *
 * @param[in] sf  sf object
 *
 * @return 
 **/
static bool sfb_is_opened(HS_SF_Type *sf)
{
    return (*sfb_cpm(sf) & CPM_SF_GATE_EN_MASK) ? false : true;
}

/**
 * @brief sfb_irq_process()
 *
 * @param[in] sf  sf object
 *
 * @return 
 **/
static void sfb_irq_process(HS_SF_Type *sf, uint32_t sf_index)
{
    // check
    if(sf->RAW_INTR_STATUS & SFB_DONE)
    {
        // clear
        sf->RAW_INTR_STATUS = SFB_DONE;

        // Default Disable IRQ
        sf->INTR_MASK = 0;

        // Allow sleep
        pmu_lowpower_allow(PMU_LP_SF0<<sf_index);

        // callback
        if(sfb_env.callback[sf_index])
            sfb_env.callback[sf_index](sf);
    }
}

/**
 * @brief sfb_process_nonblock()
 *
 * @param[in] sf  sf object
 * @param[in] spi_cmd  
 * @param[in] cmd  
 * @param[in] data  
 *
 * @return 
 **/
static void sfb_process_nonblock(HS_SF_Type *sf, spi_cmd_t *spi_cmd, uint32_t cmd[2], void *data)
{
    int sf_index = sfb_index(sf);

    // check
    co_assert((sf->INTR_MASK & SFB_DONE) == 0);

    // Prevent sleep
    pmu_lowpower_prevent(PMU_LP_SF0<<sf_index);

    // Clear and Enable done IQR
    sf->RAW_INTR_STATUS = SFB_DONE;
    sf->INTR_MASK = SFB_DONE;

    // ctrl
    sf->COMMAND_DATA0_REG = cmd[0];
    sf->COMMAND_DATA1_REG = cmd[1];
    sf->ADDRESS_REG = (uint32_t)data;
    sf->COMMAND = *(uint32_t *)spi_cmd;
}

/**
 * @brief  sfb process block
 *
 * @param[in] sf  sf
 * @param[in] spi_cmd  spi cmd
 * @param[in] cmd cmd
 * @param[in] data  data
 **/
static void sfb_process_block(HS_SF_Type *sf, spi_cmd_t *spi_cmd, uint32_t cmd[2], void *data)
{
    uint32_t irq_save = 0;

    // check
    co_assert((sf->INTR_MASK & SFB_DONE) == 0);

    // critical entry
    if(sfb_env.critical_obj == sf)
        CO_DISABLE_IRQ_EX(irq_save);

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    bool workaround_cs = false;
    if (sf==HS_SF && spi_cmd->cs==1)
    {
        workaround_cs = true;
        spi_cmd->cs = 0;
        REGW1(&HS_SYS->PINMUX[1], SYS_PINMUX_SFLASH_CTRL_SEL_MASK);
    }
#endif

    // ctrl
    sf->COMMAND_DATA0_REG = cmd[0];
    sf->COMMAND_DATA1_REG = cmd[1];
    sf->ADDRESS_REG = (uint32_t)data;
    sf->COMMAND = *(uint32_t *)spi_cmd;

    // wait done
    while(!(sf->RAW_INTR_STATUS & SFB_DONE));
    sf->RAW_INTR_STATUS = SFB_DONE;

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    if (workaround_cs)
    {
        spi_cmd->cs = 1;
        if (spi_cmd->keepCs == 0)
            REGW0(&HS_SYS->PINMUX[1], SYS_PINMUX_SFLASH_CTRL_SEL_MASK);
    }
#endif

    // critical exit
    if(sfb_env.critical_obj == sf)
        CO_RESTORE_IRQ_EX(irq_save);
}

/**
 * @brief sfb_raw_rdata()
 *
 * @param[in] sf  sf object
 * @param[in] data  
 *
 * @return 
 **/
static void sfb_raw_rdata(HS_SF_Type *sf, uint32_t data[2])
{
    data[0] = sf->READ0_REG;
#if SFB_NODMA_READ_DATA_LEN_MAX == 8
    data[1] = sf->READ1_REG;
#endif
}

/**
 * @brief sfb_raw_rdata0()
 *
 * @param[in] sf  sf object
 * @param[in] data  
 *
 * @return 
 **/
static void sfb_raw_rdata0(HS_SF_Type *sf, uint32_t data[1])
{
    data[0] = sf->READ0_REG;
}

/**
 * @brief sfb_read_nodma_1cmd_le3data()
 *
 * more efficiency than sfb_read_nodma_common
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 *
 * @return 
 **/
static void sfb_read_nodma_1cmd_le3data(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param)
{
    spi_cmd_t spi_cmd;
    uint32_t rdata[1];

    co_assert(cs < SFB_CS_NUM);

    co_assert(param->cmd_bits == 8);
    co_assert(param->data_bytes <= 3);

    // cmd
    *((uint32_t *)(&spi_cmd)) = 0;
    spi_cmd.cmdBits = param->cmd_bits + (param->data_bytes << 3);
    spi_cmd.dataBytes = 0;
    spi_cmd.cmd = SFB_CMD_READ;
    spi_cmd.cs = cs;

    // send cmd
    sfb_process_block(sf, &spi_cmd, param->cmd, NULL);

    // copy result
    sfb_raw_rdata0(sf, rdata);

    // convert
    switch(param->data_bytes)
    {
        case 1: ((uint8_t *)param->data)[0] = ((uint8_t *)rdata)[0]; break;
        case 2: ((uint8_t *)param->data)[0] = ((uint8_t *)rdata)[1];
                ((uint8_t *)param->data)[1] = ((uint8_t *)rdata)[0]; break;
        case 3: ((uint8_t *)param->data)[0] = ((uint8_t *)rdata)[2];
                ((uint8_t *)param->data)[1] = ((uint8_t *)rdata)[1];
                ((uint8_t *)param->data)[2] = ((uint8_t *)rdata)[0]; break;
        default: break;
    }

}

/**
 * @brief sfb_read_nodma()
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 *
 * @return 
 **/
static void sfb_read_nodma_common(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param)
{
    spi_cmd_t spi_cmd;
    uint32_t rdata[2], wdata[2] = {0}, i, i4x;
    uint32_t cur_data_bytes, cur_data_bytes_4align;
    uint32_t left_data_bytes = param->data_bytes;
    uint32_t index_data_bytes = 0;

    co_assert(cs < SFB_CS_NUM);

    uint32_t irq_save = 0;
    if(sfb_env.critical_obj == sf)
        CO_DISABLE_IRQ_EX(irq_save);

    // cmd
    *((uint32_t *)(&spi_cmd)) = 0;
    spi_cmd.cmdBits = param->cmd_bits;
    spi_cmd.dataBytes = 0;
    spi_cmd.cmd = SFB_CMD_READ;
    spi_cmd.cs = cs;
    spi_cmd.keepCs = left_data_bytes ? 1 : 0;

    // send cmd
    sfb_process_block(sf, &spi_cmd, param->cmd, NULL);

    // read data
    while(left_data_bytes)
    {
        cur_data_bytes = (left_data_bytes > SFB_NODMA_READ_DATA_LEN_MAX) ? SFB_NODMA_READ_DATA_LEN_MAX : left_data_bytes;
        left_data_bytes -= cur_data_bytes;

        // new cmd
        spi_cmd.cmdBits = cur_data_bytes << 3;
        spi_cmd.keepCs = left_data_bytes ? 1 : 0;

        // read data
        sfb_process_block(sf, &spi_cmd, wdata, NULL);

        // copy result
        sfb_raw_rdata(sf, rdata);

        // save
        cur_data_bytes_4align = cur_data_bytes >> 2;
        for (i=0,i4x=0; i4x<cur_data_bytes_4align; ++i4x,i+=4)
        {
            ((uint8_t *)param->data)[index_data_bytes + i + 0] = ((uint8_t *)rdata)[i + 3];
            ((uint8_t *)param->data)[index_data_bytes + i + 1] = ((uint8_t *)rdata)[i + 2];
            ((uint8_t *)param->data)[index_data_bytes + i + 2] = ((uint8_t *)rdata)[i + 1];
            ((uint8_t *)param->data)[index_data_bytes + i + 3] = ((uint8_t *)rdata)[i + 0];
        }
        switch(cur_data_bytes - i)
        {
            case 1: ((uint8_t *)param->data)[index_data_bytes + i + 0] = ((uint8_t *)rdata)[i + 0]; break;
            case 2: ((uint8_t *)param->data)[index_data_bytes + i + 0] = ((uint8_t *)rdata)[i + 1];
                    ((uint8_t *)param->data)[index_data_bytes + i + 1] = ((uint8_t *)rdata)[i + 0]; break;
            case 3: ((uint8_t *)param->data)[index_data_bytes + i + 0] = ((uint8_t *)rdata)[i + 2];
                    ((uint8_t *)param->data)[index_data_bytes + i + 1] = ((uint8_t *)rdata)[i + 1];
                    ((uint8_t *)param->data)[index_data_bytes + i + 2] = ((uint8_t *)rdata)[i + 0]; break;
            default: break;
        }
        index_data_bytes += cur_data_bytes;
    }

    if(sfb_env.critical_obj == sf)
        CO_RESTORE_IRQ_EX(irq_save);
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief read with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_read_dma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param)
{
    spi_cmd_t spi_cmd;
    int sf_index = sfb_index(sf);

    co_assert(cs < SFB_CS_NUM);

    co_assert(param->data_bytes <= SFB_DMA_DATA_LEN_MAX);
    co_assert(((uint32_t)param->data & 3) == 0);

    // cmd
    *((uint32_t *)(&spi_cmd)) = 0;
    spi_cmd.cmdBits = param->cmd_bits;
    spi_cmd.dataBytes = param->data_bytes;
    spi_cmd.cmd = SFB_CMD_READ;
    spi_cmd.cs = cs;

    // op
    if(sfb_env.callback[sf_index])
        sfb_process_nonblock(sf, &spi_cmd, param->cmd, (void *)param->data);
    else
        sfb_process_block(sf, &spi_cmd, param->cmd, (void *)param->data);
}

/**
 * @brief read with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] keep_cs  keep cs
 * @param[in] param  rw param
 **/
void sfb_read_dma_ex(HS_SF_Type *sf, uint32_t cs, sfb_keep_cs_t keep_cs, sfb_rw_params_t *param)
{
    spi_cmd_t spi_cmd;
    int sf_index = sfb_index(sf);

    co_assert(cs < SFB_CS_NUM);

    co_assert(param->data_bytes <= SFB_DMA_DATA_LEN_MAX);
    co_assert(((uint32_t)param->data & 3) == 0);

    // cmd
    *((uint32_t *)(&spi_cmd)) = 0;
    spi_cmd.cmdBits = (keep_cs==SFB_CS_BEGIN||keep_cs==SFB_CS_NOKEEP) ? param->cmd_bits : 0;
    spi_cmd.dataBytes = param->data_bytes;
    spi_cmd.cmd = SFB_CMD_READ;
    spi_cmd.cs = cs;
    spi_cmd.keepCs = (keep_cs==SFB_CS_END||keep_cs==SFB_CS_NOKEEP) ? 0 : 1;

    // op
    if(sfb_env.callback[sf_index])
        sfb_process_nonblock(sf, &spi_cmd, param->cmd, (void *)param->data);
    else
        sfb_process_block(sf, &spi_cmd, param->cmd, (void *)param->data);
}

/**
 * @brief write with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_write_dma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param)
{
    spi_cmd_t spi_cmd;
    int sf_index = sfb_index(sf);

    co_assert(cs < SFB_CS_NUM);

    co_assert(param->data_bytes <= SFB_DMA_DATA_LEN_MAX);
    co_assert(((uint32_t)param->data & 3) == 0);
    co_assert(((uint32_t)param->data & 0xFF000000) != 0x08000000);

    // cmd
    *((uint32_t *)(&spi_cmd)) = 0;
    spi_cmd.cmdBits = param->cmd_bits;
    spi_cmd.dataBytes = param->data_bytes;
    spi_cmd.cmd = SFB_CMD_WRITE;
    spi_cmd.cs = cs;
#ifdef CONFIG_SF_XJF_DESIGN
    spi_cmd.lcd2lane = sfb_env.lcd2lane[sf_index][cs];
#endif

    // op
    if(sfb_env.callback[sf_index])
        sfb_process_nonblock(sf, &spi_cmd, param->cmd, (void *)param->data);
    else
        sfb_process_block(sf, &spi_cmd, param->cmd, (void *)param->data);
}

/**
 * @brief write with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] keep_cs  keep cs
 * @param[in] param  rw param
 **/
void sfb_write_dma_ex(HS_SF_Type *sf, uint32_t cs, sfb_keep_cs_t keep_cs, sfb_rw_params_t *param)
{
    spi_cmd_t spi_cmd;
    int sf_index = sfb_index(sf);

    co_assert(cs < SFB_CS_NUM);

    co_assert(param->data_bytes <= SFB_DMA_DATA_LEN_MAX);
    co_assert(((uint32_t)param->data & 3) == 0);
    co_assert(((uint32_t)param->data & 0xFF000000) != 0x08000000);

    // cmd
    *((uint32_t *)(&spi_cmd)) = 0;
    spi_cmd.cmdBits = (keep_cs==SFB_CS_BEGIN||keep_cs==SFB_CS_NOKEEP) ? param->cmd_bits : 0;
    spi_cmd.dataBytes = param->data_bytes;
    spi_cmd.cmd = SFB_CMD_WRITE;
    spi_cmd.cs = cs;
    spi_cmd.keepCs = (keep_cs==SFB_CS_END||keep_cs==SFB_CS_NOKEEP) ? 0 : 1;
#ifdef CONFIG_SF_XJF_DESIGN
    spi_cmd.lcd2lane = sfb_env.lcd2lane[sf_index][cs];
#endif

    // op
    if(sfb_env.callback[sf_index])
        sfb_process_nonblock(sf, &spi_cmd, param->cmd, (void *)param->data);
    else
        sfb_process_block(sf, &spi_cmd, param->cmd, (void *)param->data);
}

/**
 * @brief read without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_read_nodma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param)
{
    co_assert(cs < SFB_CS_NUM);

    if (param->cmd_bits == 8 && param->data_bytes <= 3)
        sfb_read_nodma_1cmd_le3data(sf, cs, param); // more efficiency
    else
        sfb_read_nodma_common(sf, cs, param);
}

/**
 * @brief write without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_write_nodma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param)
{
    spi_cmd_t spi_cmd;
    uint32_t wdata[2], i;
    uint32_t cur_data_bytes;
    uint32_t left_data_bytes = param->data_bytes;
    uint32_t index_data_bytes = 0;

    co_assert(cs < SFB_CS_NUM);

    uint32_t irq_save = 0;
    if(sfb_env.critical_obj == sf)
        CO_DISABLE_IRQ_EX(irq_save);

    // cmd
    *((uint32_t *)(&spi_cmd)) = 0;
    spi_cmd.cmdBits = param->cmd_bits;
    spi_cmd.dataBytes = 0;
    spi_cmd.cmd = SFB_CMD_WRITE;
    spi_cmd.cs = cs;
    spi_cmd.keepCs = left_data_bytes ? 1 : 0;

    // send cmd
    sfb_process_block(sf, &spi_cmd, param->cmd, NULL);

    // read data
    while(left_data_bytes)
    {
        cur_data_bytes = (left_data_bytes > SFB_NODMA_WRITE_DATA_LEN_MAX) ? SFB_NODMA_WRITE_DATA_LEN_MAX : left_data_bytes;
        left_data_bytes -= cur_data_bytes;

        // save
        for (i=0; i<cur_data_bytes; ++i)
            ((uint8_t *)wdata)[(i&~3)+3-(i%4)] = ((uint8_t *)param->data)[index_data_bytes + i];
        index_data_bytes += cur_data_bytes;

        // new cmd
        spi_cmd.cmdBits = cur_data_bytes << 3;
        spi_cmd.keepCs = left_data_bytes ? 1 : 0;

        // read data
        sfb_process_block(sf, &spi_cmd, wdata, NULL);
    }

    if(sfb_env.critical_obj == sf)
        CO_RESTORE_IRQ_EX(irq_save);
}

/**
 * @brief open
 *
 * @param[in] sf  sf object
 **/
void sfb_open(HS_SF_Type *sf)
{
    register_set(sfb_cpm(sf), MASK_3REG(CPM_SF_SOFT_RESET,1, CPM_SF_DIV_SEL,0, CPM_SF_GATE_EN,0));
    HS_PSO_UPD_RDY();
}

/**
 * @brief close
 *
 * @param[in] sf  sf object
 **/
void sfb_close(HS_SF_Type *sf)
{
    register_set(sfb_cpm(sf), MASK_3REG(CPM_SF_SOFT_RESET,0, CPM_SF_DIV_SEL,0, CPM_SF_GATE_EN,1));
    HS_PSO_UPD_RDY();
}

/**
 * @brief set dma done event
 *
 * @param[in] sf  sf object
 * @param[in] callback  event callback
 **/
void sfb_dma_done_event_register(HS_SF_Type *sf, sfb_callback_t callback)
{
    IRQn_Type irqn = sfb_irqn(sf);
    int sf_index = sfb_index(sf);

    sfb_env.callback[sf_index] = callback;

    if (callback)
    {
        NVIC_ClearPendingIRQ(irqn);
        NVIC_SetPriority(irqn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(irqn);
    }
    else
    {
        NVIC_DisableIRQ(irqn);
    }
}

/**
 * @brief  sfb dma done event get
 *
 * @param[in] sf  sf
 *
 * @return callback
 **/
sfb_callback_t sfb_dma_done_event_get(HS_SF_Type *sf)
{
    return sfb_env.callback[sfb_index(sf)];
}

/**
 * @brief config
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  config
 **/
void sfb_config(HS_SF_Type *sf, uint32_t cs, const sfb_config_t *config)
{
    int sf_index = sfb_index(sf);
    uint32_t clk = sfb_clk(sf);
    uint32_t div = config->freq_hz<256 ? config->freq_hz : DIV_ROUND_UP(clk, config->freq_hz);

    co_assert(cs < SFB_CS_NUM);

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    // HS6621A2: HS_SF's CS0-1 use CS0, switch by SYS_PINMUX_SFLASH_CTRL_SEL_MASK
    uint32_t cs_save = cs;
    if (sf==HS_SF && cs==1)
    {
        sfb_workaround_hs6621_cs1_issue();
        cs = 0;
    }
#endif

    if (div > (SF_CTRL_CLOCK_DIV_MASK))
        div = SF_CTRL_CLOCK_DIV_MASK;
    div &= ~1; // must be even

    // Write REG
    REGWA(&sf->CONFIGURATION[cs].CTRL, MASK_8REG(
        SF_CTRL_LCD_RD_EN,      0,
        SF_CTRL_RGB_MODE,       0,
        SF_CTRL_LCD_SPI_CTRL,   0,
        SF_CTRL_WIDTH,          0,
        SF_CTRL_DLYX_SAMPLE,    config->delay,
        SF_CTRL_BP_CLOCK_DIV,   div<2 ? 1 : 0,
        SF_CTRL_MODE,           config->transmode,
        SF_CTRL_CLOCK_DIV,      div<2 ? 2 : div));

    REGW(&sf->CONFIGURATION[cs].CS, MASK_1REG(SF_CS_POL, config->cs_pol));

    // Default disable all done irq
    sf->INTR_MASK = 0; /* disable all */
    sf->RAW_INTR_STATUS = SFB_DONE; /* clear */

    // Save
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    if (sf == HS_SF)
    {
        if (cs_save == 1)
        {
            sfb_env.configuration_save[sf_index][1].ctrl = sf->CONFIGURATION[0].CTRL;
            sfb_env.configuration_save[sf_index][1].cs   = sf->CONFIGURATION[0].CS;
            return;
        }

        REGW0(&HS_SYS->PINMUX[1], SYS_PINMUX_SFLASH_CTRL_SEL_MASK);
    }
#endif

#ifdef CONFIG_SF_XJF_DESIGN
    sfb_env.lcd2lane[sf_index][cs] = 0;
#endif

    sfb_env.configuration_save[sf_index][cs].ctrl = sf->CONFIGURATION[cs].CTRL;
    sfb_env.configuration_save[sf_index][cs].cs   = sf->CONFIGURATION[cs].CS;
}

#ifdef CONFIG_SF_XJF_DESIGN
/**
 * @brief  sfb lcd2lane enable
 *
 * @param[in] sf  sf
 * @param[in] cs  cs
 * @param[in] enable  enable
 **/
void sfb_lcd2lane_enable(HS_SF_Type *sf, uint32_t cs, bool enable)
{
    int sf_index = sfb_index(sf);
    sfb_env.lcd2lane[sf_index][cs] = enable;
}
#else // move to lcd_serial_new.c in CONFIG_SF_XJF_DESIGN
/**
 * @brief lcd config
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  config
 **/
void sfb_lcd_config(HS_SF_Type *sf, uint32_t cs, const sfb_lcd_config_t *config)
{
    int sf_index = sfb_index(sf);
    uint8_t lcd_spi_ctrl = 1;
    bool support = true;

    co_assert(cs < SFB_CS_NUM);

    // Check lcd_spi_ctrl
    if(config->wire_mode == SFB_LCD_9BITS_WITH_DCX)
    {
        switch(config->rgb_transmode)
        {
            case SFB_LCD_1LANE:
                lcd_spi_ctrl = 1;
                break;

            case SFB_LCD_2LANE_1PIXLE_1TRANS:
                switch (config->rgb_mode)
                {
                    case SFB_LCD_RGB565: lcd_spi_ctrl = 3; break;
                    case SFB_LCD_RGB666: lcd_spi_ctrl = 4; break;
                    case SFB_LCD_RGB888: lcd_spi_ctrl = 5; break;
                    case SFB_LCD_RGBNON: support = false;  break;
                }
                break;

            case SFB_LCD_2LANE_2PIXLE_3TRANS:
                switch (config->rgb_mode)
                {
                    case SFB_LCD_RGB565: support = false;  break;
                    case SFB_LCD_RGB666: lcd_spi_ctrl = 6; break;
                    case SFB_LCD_RGB888: lcd_spi_ctrl = 7; break;
                    case SFB_LCD_RGBNON: support = false;  break;
                }
                break;

            default:
                support = false;
                break;
        }

        // Not support: back to SFB_LCD_1LANE
        if(!support)
            lcd_spi_ctrl = 1;
    }
    else // if(config->wire_mode == SFB_LCD_8BITS_DCX_ALONE)
    {
        switch(config->rgb_transmode)
        {
            case SFB_LCD_1LANE:
                lcd_spi_ctrl = 2;
                break;

            case SFB_LCD_2LANE_1PIXLE_1TRANS:
            case SFB_LCD_2LANE_2PIXLE_3TRANS:
            default:
                support = false;
                break;
        }

        // Not support: back to SFB_LCD_1LANE
        if(!support)
            lcd_spi_ctrl = 2;
    }

    // Write REG
    REGW(&sf->CONFIGURATION[cs].CTRL, MASK_4REG(
        SF_CTRL_LCD_RD_EN,      config->rw_mode,
        SF_CTRL_RGB_MODE,       config->rgb_mode==SFB_LCD_RGBNON ? 0 : config->rgb_mode,
        SF_CTRL_LCD_SPI_CTRL,   lcd_spi_ctrl,
        SF_CTRL_WIDTH,          config->rgb_mode==SFB_LCD_RGBNON ? 0 : config->rgb_mode==SFB_LCD_RGB565 ? 1 : 2));

    // Save
    sfb_env.configuration_save[sf_index][cs].ctrl = sf->CONFIGURATION[cs].CTRL;
}
#endif

/**
 * @brief restore
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sfb_restore(HS_SF_Type *sf, uint32_t cs)
{
    int sf_index = sfb_index(sf);

    co_assert(cs < SFB_CS_NUM);

    if(sfb_env.configuration_save[sf_index][cs].ctrl == 0)
        return;

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    if (sf == HS_SF)
    {
        if (cs == 1)
        {
            sfb_workaround_hs6621_cs1_issue();
            sf->CONFIGURATION[0].CTRL = sfb_env.configuration_save[sf_index][1].ctrl;
            sf->CONFIGURATION[0].CS = sfb_env.configuration_save[sf_index][1].cs;
            return;
        }

        REGW0(&HS_SYS->PINMUX[1], SYS_PINMUX_SFLASH_CTRL_SEL_MASK);
    }
#endif

    sf->CONFIGURATION[cs].CTRL = sfb_env.configuration_save[sf_index][cs].ctrl;
    sf->CONFIGURATION[cs].CS = sfb_env.configuration_save[sf_index][cs].cs;

    if (sfb_env.callback[sf_index])
    {
        IRQn_Type irqn = sfb_irqn(sf);
        NVIC_SetPriority(irqn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(irqn);
    }
}

/**
 * @brief regs get
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return reg
 **/
uint32_t sfb_regs_get(HS_SF_Type *sf, uint32_t cs)
{
    return sf->CONFIGURATION[cs].CTRL;
}

/**
 * @brief regs get
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] regs  reg
 **/
void sfb_regs_set(HS_SF_Type *sf, uint32_t cs, uint32_t regs)
{
    int sf_index = sfb_index(sf);

    sf->CONFIGURATION[cs].CTRL = sfb_env.configuration_save[sf_index][cs].ctrl = regs;
}

/**
 * @brief  sfb critical object set
 *
 * @param[in] sf  sf
 **/
void sfb_critical_object_set(HS_SF_Type *sf)
{
    sfb_env.critical_obj = sf;
}

/**
 * @brief  sfb critical object get
 *
 * @return obj
 **/
HS_SF_Type *sfb_critical_object_get(void)
{
    return sfb_env.critical_obj;
}

/**
 * @brief sfb_enable()
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sfb_enable(HS_SF_Type *sf, uint32_t cs)
{
    if(!sfb_is_opened(sf))
        sfb_open(sf);
    sfb_restore(sf, cs);
}

/**
 * @brief SF IRQ handler
 **/
void SF_IRQHandler(void)
{
    sfb_irq_process(HS_SF,  0);
}

/**
 * @brief SF IRQ handler
 **/
void SF1_IRQHandler(void)
{
    sfb_irq_process(HS_SF1, 1);
}

#ifndef CONFIG_HS6621
/**
 * @brief SF IRQ handler
 **/
void SF2_IRQHandler(void)
{
    sfb_irq_process(HS_SF2, 2);
}
#endif

/** @} */

