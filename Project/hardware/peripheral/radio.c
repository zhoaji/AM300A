/**
 * @file radio.c
 * @brief 
 * @date Tue, Sep 24, 2019  5:32:48 PM
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
#include "rwip.h"

/*********************************************************************
 * MACROS
 */


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


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief  rf txrx pin enable
 *
 * @param[in] enable  enable
 * @param[in] tx_pin  tx pin
 * @param[in] rx_pin  rx pin
 * @param[in] pol  polarity, 0 or 1
 **/
void rf_txrx_pin_enable(bool enable, int tx_pin, int rx_pin, int pol)
{
    CPM_ANA_AHB_CLK_ENABLE();
    if (enable)
    {
        REGW(&HS_DAIF->TRX_EXT_PD_CFG, MASK_2REG(DAIF_TX_EXT_PD_POL,pol, DAIF_RX_EXT_PD_POL,pol));
        co_delay_us(1);

        if (tx_pin >= 0)
        {
            pinmux_config(tx_pin, PINMUX_TX_EXT_PD_CFG);
            REGW(&HS_DAIF->TRX_EXT_PD_CFG, MASK_1REG(DAIF_TX_EXT_PD_EN, 1));
        }
        if (rx_pin >= 0)
        {
            pinmux_config(rx_pin, PINMUX_RX_EXT_PD_CFG);
            REGW(&HS_DAIF->TRX_EXT_PD_CFG, MASK_1REG(DAIF_RX_EXT_PD_EN, 1));
        }
    }
    else
    {
        REGW(&HS_DAIF->TRX_EXT_PD_CFG, MASK_2REG(DAIF_TX_EXT_PD_EN,0, DAIF_RX_EXT_PD_EN,0));
    }
    CPM_ANA_CLK_RESTORE();
}

/**
 * @brief  rf carrier enable
 *
 * @param[in] enable  enable
 * @param[in] freq  2402MHz ... 2480MHz
 **/
void rf_carrier_enable(bool enable, uint32_t freq)
{
    pmu_ana_enable(enable, PMU_ANA_RF);

    if(enable)
    {
        CPM_ANA_CLK_ENABLE();

        // frequency
        REGW(&HS_DAIF->FREQ_CFG0, MASK_2REG(DAIF_FREQ_REG_ME,1, DAIF_FREQ_REG_MO,freq));

        // SDM
        REGW(&HS_DAIF->PLL_CTRL1, MASK_2REG(DAIF_DIGI_DIN_BYPASS, 1, DAIF_DIGI_DIN_REG, 0));
        REGW(&HS_DAIF->PLL_CTRL2, MASK_2REG(DAIF_DIN_SDM_TX_ME, 1, DAIF_DATA_SYNC_BYPASS, 1));

        // do TX
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,1, DAIF_RX_EN_MO,0, DAIF_TX_EN_MO,0));
        co_delay_10us(10);
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,1, DAIF_RX_EN_MO,0, DAIF_TX_EN_MO,1));

        CPM_ANA_CLK_RESTORE();
    }
    else
    {
        CPM_ANA_CLK_ENABLE();
        REGW(&HS_DAIF->FREQ_CFG0, MASK_1REG(DAIF_FREQ_REG_ME,0));
        REGW(&HS_DAIF->PLL_CTRL1, MASK_2REG(DAIF_DIGI_DIN_BYPASS, 0, DAIF_DIGI_DIN_REG, 0));
        REGW(&HS_DAIF->PLL_CTRL2, MASK_2REG(DAIF_DIN_SDM_TX_ME, 0, DAIF_DATA_SYNC_BYPASS, 0));
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,0, DAIF_RX_EN_MO,0, DAIF_TX_EN_MO,0));
        CPM_ANA_CLK_RESTORE();
    }
}

/**
 * @brief  rf full rx enable
 *
 * @param[in] enable  enable
 * @param[in] freq  2402MHz ... 2480MHz
 **/
void rf_full_rx_enable(bool enable, uint32_t freq)
{
    pmu_ana_enable(enable, PMU_ANA_RF);

    if (enable)
    {
        CPM_ANA_CLK_ENABLE();

        // frequency
        REGW(&HS_DAIF->FREQ_CFG0, MASK_2REG(DAIF_FREQ_REG_ME,1, DAIF_FREQ_REG_MO,freq));

        // Do RX
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,1, DAIF_RX_EN_MO,0, DAIF_TX_EN_MO,0));
        co_delay_10us(10);
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,1, DAIF_RX_EN_MO,1, DAIF_TX_EN_MO,0));

        CPM_ANA_CLK_RESTORE();
    }
    else
    {
        CPM_ANA_CLK_ENABLE();
        REGW(&HS_DAIF->FREQ_CFG0, MASK_1REG(DAIF_FREQ_REG_ME,0));
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,0, DAIF_RX_EN_MO,0, DAIF_TX_EN_MO,0));
        CPM_ANA_CLK_RESTORE();
    }
}

/**
 * @brief  rf tx power set
 *
 * @param[in] auto_ctrl_by_ble  false: control by power param; true: auto control by STACK
 * @param[in] power  power
 **/
void rf_tx_power_set(bool auto_ctrl_by_ble, int power)
{
    CPM_ANA_CLK_ENABLE();

#ifdef CONFIG_HS6621
    if(auto_ctrl_by_ble)
    {
        // DBG=1: MO_REG ctrl PA 
        // DBG=0: SEL ctrl PA
        //   SEL=0: REG_TABLE ctrl PA
        //   SEL=1: RW ctrl PA
        REGW(&HS_DAIF->PA_CNS, MASK_2REG(DAIF_PA_DBG,0, DAIF_PA_GAIN_IDX_SEL,1));
    }
    else
    {
        if (power > 4)
            HS_DAIF->PA_CNS = 0x5211A2BB;
        else
            REGW(&HS_DAIF->PA_CNS, MASK_3REG(DAIF_PA_DBG,0, DAIF_PA_GAIN_IDX_SEL,0, DAIF_PA_GAIN_IDX_REG,rwip_rf.txpwr_cs_get(power, 0)));
    }
#else
    bool high_tx_power_mode = false;
    uint32_t ldo_v1p2_vbat;
    uint32_t dcdc_vout;

    if(auto_ctrl_by_ble)
    {
        REGW(&HS_DAIF->PA_CNS, MASK_2REG(DAIF_PA_DBG,0, DAIF_PA_GAIN_IDX_SEL,1));
    }
    else
    {
        if (power > 4)
        {
            HS_DAIF->PA_CNS = 0x1234E3;
            REGW(&HS_PMU->ANA_REG, MASK_1REG(PMU_ANA_LDO_V1P0_PA_VBAT,3));

            ldo_v1p2_vbat = 3;
            dcdc_vout = 7;

            high_tx_power_mode = true;
        }
        else
        {
            REGW(&HS_DAIF->PA_CNS, MASK_3REG(DAIF_PA_DBG,0, DAIF_PA_GAIN_IDX_SEL,0, DAIF_PA_GAIN_IDX_REG,rwip_rf.txpwr_cs_get(power, 0)));
        }
    }

    if (!high_tx_power_mode)
    {
        // @ref calib.c
        if (calib_repair_env.temperature > 70)
        {
            ldo_v1p2_vbat = 2;
            dcdc_vout = 6;
        }
        else
        {
            ldo_v1p2_vbat = 0;
            dcdc_vout = 4;
        }
        REGW(&HS_PMU->ANA_REG, MASK_1REG(PMU_ANA_LDO_V1P0_PA_VBAT,1));
    }

    REGW(&HS_PMU->ANA_REG, MASK_1REG(PMU_ANA_LDO_V1P2_A_VBAT,
                calib_repair_value_select(ldo_v1p2_vbat+calib_repair_env.delta_vdd_1p2,0,3)));
    REGW(&HS_PMU->ANA_PD_1, MASK_1REG(PMU_DCDC_VOUT,
                calib_repair_value_select(dcdc_vout+calib_repair_env.delta_dcdc_1p27,0,7)));
#endif

    calib_rf_pa_save();

    CPM_ANA_CLK_RESTORE();
}

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
#if (CONFIG_DIRECTION_FINDING)
void rf_set_cte(uint8_t* ptn, uint8_t ptn_len, uint8_t smp_win)
{
    uint32_t i;
    uint32_t ant_q[5] = {0x00};
    uint32_t group_index;
    uint32_t bit_index;

    if(ptn == NULL)  // used for not switch antenna
    {
        for(i=0x00; i<sizeof(ant_q)/sizeof(ant_q[0]); i++)
        {
            ant_q[i] = 0x00;
        }
    }
    else
    {
        for(i=0x00; i<ptn_len; i++)
        {
            group_index = i>>4;
            bit_index = (i % 16)*2;
            ant_q[group_index] |= (ptn[i] & 0x03) << bit_index;
        }
    }

    for(i=0x00; i<5; i++)
    {
        HS_PHY->ANT_Q_REG[i] = ant_q[i];
    }

    // smp point is 8/16 * 1us, which locates in the middle in the sample window.
    //HS_PHY->CTE_CTRL = (1<<10) + ((smp_win == 1) << 8) + (0x08 & 0x0F);
    if(smp_win == 1) {
        HS_PHY->CTE_CTRL |= (1<<8);
    } else {
        HS_PHY->CTE_CTRL &= ~(1<<8);
    }
}
#endif  //(CONFIG_DIRECTION_FINDING)
#endif //}BASE_COMPONENT_END
/** @} */

