/**
 * @file calib.c
 * @brief 
 * @date 2016/12/26 20:15:02
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
#include "features.h"
#include "peripheral.h"
#include "co.h"
#include "cfg.h"

//{BASE_COMPONENT_BEGIN

/*********************************************************************
 * MACROS
 */

//#define CALIB_DEBUG
#ifdef CALIB_DEBUG
#define calib_log(format, ...) log_debug(format, ## __VA_ARGS__)
#else
#define calib_log(format, ...)
#endif

// ct_fdoubler use same with lpf
#define RC_CALIB_VALUE(lpf, tia, auadc)        {REGW(&HS_DAIF->TIA_LPF_CFG, MASK_2REG(DAIF_FILTER_CTUNE,lpf, DAIF_TIA_CTUNE,tia)); REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_REG_CTUNE_FDOUB, lpf)); calib_env.sys.rc_tune_auadc = auadc;}
#define RC_IF_GREATER_DO(sum, lpf, tia, auadc) if (rc_sum > (sum)) RC_CALIB_VALUE(lpf, tia, auadc)
#define RC_EI_GREATER_DO(sum, lpf, tia, auadc) else if (rc_sum > (sum)) RC_CALIB_VALUE(lpf, tia, auadc)
#define RC_EL_GREATER_DO(lpf, tia, auadc)      else RC_CALIB_VALUE(lpf, tia, auadc)

// extra length
#define CALIB_FLASH_LENGTH          (sizeof(calib_flash_data_t))
#define CALIB_FLASH_EXTRA_LENGTH    ((CALIB_FLASH_LENGTH>CFG_TAG_LENGTH_MAX) ? (CALIB_FLASH_LENGTH-CFG_TAG_LENGTH_MAX) : 0)

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
    struct
    {
        uint16_t vco_afc_lut[80];
        uint8_t  vco_amp;
        uint32_t kdco_1m[2];
        uint32_t kdco_2m[2];
        uint32_t dcoc_lut[63];
        uint16_t agc_pif_os;
        uint16_t agc_pif_os_2m; // not for HS6621
        uint32_t txfe_pa_gain[18];
        uint32_t pa_cns;
    }rf;

    struct
    {
        // calib with Power ON
        uint32_t rc_tia_lpf;
        uint32_t rc_tune_auadc;
    }sys;
}calib_t;
//}BASE_COMPONENT_END

typedef struct
{
    calib_t save;

    struct
    {
        uint8_t ct_fdoubler;
//        uint32_t ct_osc32m;
//        uint32_t ct_xtal32m;
//        uint32_t ct_osc32m_ramp;
//        uint32_t ctrl_xtal32m_ldo;
//        uint32_t sel_xtal32m_gm;
//        uint32_t sel_xtal32m_nrb;
//        uint32_t sel_xtal32m_pkdvref;
    }pmu;

    struct
    {
        uint8_t rtune_rc32k;
        uint8_t ctune_rc32k;
//        uint32_t ct_xtal32k;
//        uint32_t sel_i_xtal32k;
//        uint32_t sel_iglob;
    }hib;

    uint16_t crc16;
}calib_flash_data_t;

typedef struct
{
    uint8_t padr1s_ct_mo;
    uint8_t pacore_ct_mo;
    uint8_t tx_atten_mo;
}calib_rxfe_pa_t;

/*********************************************************************
 * CONSTANTS
 */
#ifdef CONFIG_HS6621C
const static calib_rxfe_pa_t txfe_pa_table[] =
{
    /*                          #1                    #2                   #lose */
    /*                          Average    max        Average    max       #lose */
    /* 1  */ {0x1, 0x1, 0x0}, // -26.04    -25.42     -26.18     -25.58    0
    /* 2  */ {0x1, 0x1, 0x1}, // -23.63    -23.09     -23.91     -23.51    0
    /* 3  */ {0x1, 0x1, 0x2}, // -19.17    -18.85     -19.58     -19.31    0
    /* 4  */ {0x1, 0x1, 0x3}, // -9.11     -8.97      -9.8       -9.68     0
    /* 5  */ {0x3, 0x1, 0x3}, // -7.59     -6.91      -6.53      -6.4      0
    /* 6  */ {0x4, 0x2, 0x3}, // -2.1      -1.91      -1.44      -1.25     0
    /* 7  */ {0x2, 0x3, 0x3}, // -0.16     0.02       0.22       0.43      0
    /* 8  */ {0x3, 0x3, 0x3}, // 0.16      0.35       0.82       0.98      0
    /* 9  */ {0x4, 0x3, 0x3}, // 0.58      0.75       0.95       1.12      0
    /* 10 */ {0x4, 0x4, 0x3}, // 1.93      2.07       2.22       2.37      0
    /* 11 */ {0x4, 0x5, 0x3}, // 2.7       2.84       2.96       3.08      0
    /* 12 */ {0x4, 0x6, 0x3}, // 3.15      3.28       3.38       3.49      0
    /* 13 */ {0x4, 0x7, 0x3}, // 3.51      3.61       3.71       3.83      0
    /* 14 */ {0x4, 0x8, 0x3}, // 3.74      3.84       3.85       4.06      0
    /* 15 */ {0x4, 0x9, 0x3}, // 3.82      4.02       4.03       4.22      0
    /* 16 */ {0x4, 0xA, 0x3}, // 3.96      4.15       4.17       4.36      0
    /* 17 */ {0x4, 0xB, 0x3}, // 4.09      4.28       4.28       4.47      0
    /* 18 */ {0x4, 0xC, 0x3}, // 4.19      4.38       4.38       4.56      0
};
#else
const static calib_rxfe_pa_t txfe_pa_table[] =
{
    /*                          #1      #2      #lose */
    /* 1  */ {0x1, 0x1, 0x0}, //-32.9   -28.2   0.6
    /* 2  */ {0x1, 0x1, 0x1}, //-30.0   -25.1   0.6
    /* 3  */ {0x1, 0x1, 0x2}, //-24.6   -24.3   0.6
    /* 4  */ {0x1, 0x1, 0x3}, //-11.1   -7.00   0.6
    /* 5  */ {0x2, 0x1, 0x3}, //-8.00   -6.10   0.6
    /* 6  */ {0x3, 0x1, 0x3}, //-7.20   -5.60   0.6
    /* 7  */ {0x4, 0x1, 0x3}, //-6.80   -5.40   0.6
    /* 8  */ {0x4, 0x2, 0x3}, //-2.40   -0.60   0.6
    /* 9  */ {0x4, 0x3, 0x3}, //-0.50    1.26   0.6
    /* 10 */ {0x4, 0x4, 0x3}, // 0.37    2.00   0.6
    /* 11 */ {0x4, 0x5, 0x3}, // 0.80    2.40   0.6
    /* 12 */ {0x4, 0x6, 0x3}, // 1.03    2.66   0.6
    /* 13 */ {0x4, 0x7, 0x3}, // 1.21    2.87   0.6
    /* 14 */ {0x4, 0x8, 0x3}, // 1.34    3.00   0.6
    /* 15 */ {0x4, 0x9, 0x3}, // 1.43    3.10   0.6
    /* 16 */ {0x4, 0xA, 0x3}, // 1.50    3.17   0.6
    /* 17 */ {0x4, 0xB, 0x3}, // 1.56    3.23   0.6
    /* 18 */ {0x4, 0xC, 0x3}, // 1.60    3.29   0.6
};
#endif

/*********************************************************************
 * LOCAL VARIABLES
 */

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
static calib_t calib_env; // NOTE: Must in unint section
#else //}BASE_COMPONENT_END
extern calib_t calib_env;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

//{BASE_COMPONENT_BEGIN

/**
 * @brief  calib patch
 *
 * @param[in] temperature  temperature
 **/
static void calib_patch(int16_t temperature)
{
    REGW(&HS_DAIF->MAIN_ST_CFG0, MASK_2REG(DAIF_TXLDO_WAIT,60*16, DAIF_TX_PLL_WAIT,10*16));
    REGW(&HS_DAIF->MAIN_ST_CFG1, MASK_1REG(DAIF_PA_WAIT,2));

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION==1)
    REGW(&HS_DAIF->PLL_CTRL0, MASK_4REG(DAIF_LDO_BUF,0, DAIF_LDO_BUF2,3, DAIF_LDO_PFD,3, DAIF_LDO_CP,3));
#else
    REGW(&HS_DAIF->PLL_CTRL0, MASK_3REG(DAIF_LDO_BUF2,0, DAIF_LDO_PFD,3, DAIF_LDO_CP,3));
#endif
    REGW(&HS_DAIF->PLL_CTRL2, MASK_4REG(DAIF_ENQUAD_DIV_ME,1, DAIF_ENQUAD_DIV_MO,0, DAIF_PLL_SEL_INJ,1, DAIF_DITHER_EN,1));
    REGW(&HS_PMU->ANA_REG, MASK_2REG(PMU_ANA_LDO_V1P0_IF_1P2,3, PMU_ANA_LDO_V1P0_RF_1P2,3));
#ifdef CONFIG_HS6621
    REGW(&HS_DAIF->LNA_MIX_CFG, MASK_3REG(DAIF_CT_CONSTGM,0, DAIF_LNA_VBCT,3, DAIF_MIX_VBCT,5));
#else
    REGW(&HS_DAIF->LNA_MIX_CFG, MASK_3REG(DAIF_CT_CONSTGM,1, DAIF_LNA_VBCT,3, DAIF_MIX_VBCT,5));
#endif
    REGW(&HS_DAIF->TIA_LPF_CFG, MASK_2REG(DAIF_GAIN_LPF_ME,1, DAIF_GAIN_LPF_MO,2));
    REGW(&HS_DAIF->TIA_LPF_CFG, MASK_1REG(DAIF_TIA_SWAP,1));
    REGW(&HS_DAIF->DCOC_CFG1, MASK_4REG(DAIF_OFFSET_I_MO,0, DAIF_OFFSET_I_ME,1, DAIF_OFFSET_Q_MO,0, DAIF_OFFSET_Q_ME,1));

    REGW(&HS_DAIF->PLL_CTRL2, MASK_2REG(DAIF_ICON_VCO_MO,7, DAIF_ICON_VCO_ME,1));

    REGW(&HS_DAIF->AGC_CFG0, MASK_1REG(DAIF_AGCSEL_IQ_MO,1));

    // A1 BUG: PKDADC/AGCAMP/TIF must open
    REGW(&HS_DAIF->PD_CFG0, MASK_6REG(DAIF_PKDADC_PD_MO,0, DAIF_PKDADC_PD_ME,1,
                DAIF_PD_AGCAMP_MO,0, DAIF_PD_AGCAMP_ME,1, DAIF_PD_PDTIF_MO,0, DAIF_PD_PDTIF_ME,1));
    REGW(&HS_DAIF->PD_CFG2, MASK_2REG(DAIF_PD_PLL_BUF_MO,0, DAIF_PD_PLL_BUF_ME,1));
    // disable vtrack
    REGW(&HS_DAIF->VCO_CTRL0, MASK_2REG(DAIF_VTRACK_EN,0, DAIF_VTRACK_MO,4));

#if !(defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION==1))
    HS_DAIF->IF_AGC_LUT_REG0 = 0x4160600;
    HS_DAIF->IF_AGC_LUT_REG1 = 0x44a1908;
    HS_DAIF->IF_AGC_LUT_REG2 = 0x50a511c;
    HS_DAIF->IF_AGC_LUT_REG3 = 0x148;
#endif

    uint32_t ldo_vco;
    uint32_t ldo_buf;

    if (temperature > 70)
    {
        ldo_vco = 7;
        ldo_buf = 3;
    }
    else
    {
        ldo_vco = 5;
        ldo_buf = 1;
    }

    // High temperature
    REGW(&HS_DAIF->PLL_CTRL0, MASK_1REG(DAIF_LDO_VCO, ldo_vco));
    // LDO BUF
    REGW(&HS_DAIF->PLL_CTRL0, MASK_1REG(DAIF_LDO_BUF, ldo_buf));

    // PA (Force max PA)
//    REGW(&HS_DAIF->PA_CNS, MASK_6REG(DAIF_PADR1S_CT_MO,4, DAIF_PADR2S_PCT_MO,6, DAIF_PADR2S_NCT_MO,6,
//                DAIF_PACORE_CT_MO,12, DAIF_TX_ATTEN_MO,3, DAIF_MODE_LDO_PA,0));
//    REGW(&HS_DAIF->PA_CNS, MASK_1REG(DAIF_PA_DBG,1));
}
//}BASE_COMPONENT_END

#if 0
/**
 * @brief  adcoc
 **/
static uint32_t adcoc(int adc_out, uint32_t sel_dcoc_fs)
{
    int adc_tmp;

    if(sel_dcoc_fs)
        adc_tmp = (adc_out - 176) / 5;
    else
        adc_tmp = (adc_out - 96) / 10;

    if(adc_tmp < 0)
        adc_tmp = 0;
    else if (adc_tmp > 31)
        adc_tmp = 31;

    return adc_tmp;
}
#endif

#if 0
/**
 * @brief  calib rf track reset
 **/
static void calib_rf_track_reset(void)
{
    int i;

    for(i=0; i<80; ++i)
    {
        // read
        REGW(&HS_DAIF->PLL_LUT_DBG, MASK_2REG(DAIF_PLL_LUT_LU, 1, DAIF_PLL_LUT_IDX, i));
        // wait lut read done
        while(HS_DAIF->PLL_LUT_DBG & DAIF_PLL_LUT_LU_MASK);

        // write
        REGW(&HS_DAIF->PLL_LUT_DBG, MASK_4REG(
                    DAIF_PLL_LUT_DATA_TRACK, 0x24,
                    DAIF_PLL_LUT_DATA_TUNE, REGR(&HS_DAIF->PLL_LUT_DBG, MASK_POS(DAIF_PLL_LUT_DATA_TUNE)),
                    DAIF_PLL_LUT_IDX, i,
                    DAIF_PLL_LUT_WR, 1));
        // wait lut wr done
        while(HS_DAIF->PLL_LUT_DBG & DAIF_PLL_LUT_WR_MASK);

    }
}
#endif

/**
 * @brief calib_rf_afc()
 *
 * Auto frequency calibration
 *
 * @return 
 **/
static void calib_rf_afc(void)
{
    int i;

    //sw power on analog blocks
    // power on pll analog
    REGW(&HS_DAIF->PD_CFG1, MASK_4REG(DAIF_PD_PLL_ALL_ME,1, DAIF_PD_PLL_ALL_MO,0, DAIF_PD_VCDET_ME,1, DAIF_PD_VCDET_MO,0));
    REGW(&HS_DAIF->PD_CFG2, MASK_2REG(DAIF_PD_PLL_BUF_ME,1, DAIF_PD_PLL_BUF_MO,0));

    // digital ctrl
    REGW(&HS_DAIF->VCO_CTRL0, MASK_1REG(DAIF_AFC_EN_ME,0));

    //initial afc/vtrack table
    for(i=0; i<80; ++i)
    {
        REGW(&HS_DAIF->PLL_LUT_DBG, MASK_3REG(
                    DAIF_PLL_LUT_DATA, 0x1124,
                    DAIF_PLL_LUT_IDX, i,
                    DAIF_PLL_LUT_WR, 1));
        // wait lut wr done
        while(HS_DAIF->PLL_LUT_DBG & DAIF_PLL_LUT_WR_MASK);
    }

    //=================VCO AFC TEST===============================
    //do amp cali
    REGW1(&HS_DAIF->VCO_CTRL1, DAIF_PEAKDET_START_MASK);
    // wait amp done
    while(HS_DAIF->VCO_CTRL1 & DAIF_PEAKDET_START_MASK);

    //do vco afc
    REGW1(&HS_DAIF->VCO_CTRL0, DAIF_AFC_START_MASK);
    // wait afc done
    while(HS_DAIF->VCO_CTRL0 & DAIF_AFC_START_MASK);

    //pd_lotx power on
    REGW(&HS_DAIF->PD_CFG1, MASK_2REG(DAIF_PD_LOTX_MO,0, DAIF_PD_LOTX_ME,1));

    //do kdco -- 1M
    REGW(&HS_DAIF->VCO_CTRL1, MASK_2REG(DAIF_BLE_1M_2M_SEL_MO,0, DAIF_BLE_1M_2M_SEL_ME,1));
    // try kdco 1M mode
    REGW1(&HS_DAIF->VCO_CTRL0, DAIF_KDCO_START_MASK);
    //wait kdco done
    while(HS_DAIF->VCO_CTRL0 & DAIF_KDCO_START_MASK);

    //do kdco -- 2M
    REGW(&HS_DAIF->VCO_CTRL1, MASK_2REG(DAIF_BLE_1M_2M_SEL_MO,1, DAIF_BLE_1M_2M_SEL_ME,1));
    // try kdco 2M mode
    REGW1(&HS_DAIF->VCO_CTRL0, DAIF_KDCO_START_MASK);
    //wait kdco done
    while(HS_DAIF->VCO_CTRL0 & DAIF_KDCO_START_MASK);

    // restore 1m/2m FSM ctrl
    REGW(&HS_DAIF->VCO_CTRL1, MASK_1REG(DAIF_BLE_1M_2M_SEL_ME,0));

    //make analog blocks power controlled by trx FSM
    // change power control by FSM
    //pd_pll_all
    REGW(&HS_DAIF->PD_CFG1, MASK_2REG(DAIF_PD_PLL_ALL_ME,0, DAIF_PD_VCDET_ME,0));
    REGW(&HS_DAIF->PD_CFG2, MASK_1REG(DAIF_PD_PLL_BUF_ME,0));
    //pd_lotx
    REGW(&HS_DAIF->PD_CFG1, MASK_1REG(DAIF_PD_LOTX_ME,0));
}

#if !(defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION==1)) // A1 BUG: AGC calib can't finish
/**
 * @brief calib_rf_agc()
 *
 * RX agc calibration
 *
 * @return 
 **/
static void calib_rf_agc(void)
{
    int i;
    uint32_t agc_pif_os[2];

    REGW(&HS_DAIF->LNA_MIX_CFG, MASK_6REG(DAIF_GAIN_LNA_MO,5, DAIF_GAIN_LNA_ME,1,
        DAIF_GAIN_ATTEN_MO,2, DAIF_GAIN_ATTEN_ME,1,
        DAIF_EN_ATTEN_MO,0, DAIF_EN_ATTEN_ME,1));

    REGW(&HS_DAIF->TIA_LPF_CFG, MASK_2REG(DAIF_GAIN_TIA_MO,2, DAIF_GAIN_TIA_ME,1));

    REGW(&HS_DAIF->PD_CFG1, MASK_2REG(DAIF_RST_PDTIF_MO,0, DAIF_RST_PDTIF_ME,1));

    REGW(&HS_DAIF->TRX_SW_CFG, MASK_2REG(DAIF_TRX_EN_ME,1, DAIF_TRX_EN_MO,0));

    REGW(&HS_DAIF->MAIN_ST_CFG1, MASK_1REG(DAIF_AGC_SYNC_TIME,0xF));
    REGW(&HS_DAIF->AGC_CFG2, MASK_1REG(DAIF_PIF_OS_FLAG, 0));

    // frequency
    REGW(&HS_DAIF->FREQ_CFG0, MASK_2REG(DAIF_FREQ_REG_ME,1, DAIF_FREQ_REG_MO,2404));

    for (i=0; i<2; ++i)
    {
        REGW(&HS_DAIF->TIA_LPF_CFG, MASK_2REG(DAIF_TIA_MOD_SEL_ME,(i==0) ? 1 : 0, DAIF_TIA_MOD_SEL_MO,1));

        // Do RX
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,1, DAIF_RX_EN_MO,0, DAIF_TX_EN_MO,0));
        co_delay_10us(1);
        REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,1, DAIF_RX_EN_MO,1, DAIF_TX_EN_MO,0));
        co_delay_10us(15);

        //polling agc idle
        HS_DAIF->DBG_REG = 0x1B;
        while(HS_DAIF->DBG_REG & (0xF<<16));

        REGW(&HS_DAIF->PD_CFG0, MASK_6REG(DAIF_PD_AGCAMP_MO,0, DAIF_PD_AGCAMP_ME,1,
            DAIF_PD_PDTIF_MO,0, DAIF_PD_PDTIF_ME,1, DAIF_PKDADC_PD_MO,0, DAIF_PKDADC_PD_ME,1));
        REGW(&HS_DAIF->PD_CFG1, MASK_6REG(DAIF_PD_LDO_IF_MO,0, DAIF_PD_LDO_IF_ME,1,
            DAIF_PD_LDO_ANADIG_MO,0, DAIF_PD_LDO_ANADIG_ME,1, DAIF_PD_LDO_PKDADC_MO,0, DAIF_PD_LDO_PKDADC_ME,1));
        REGW(&HS_DAIF->AGC_CFG0, MASK_3REG(DAIF_OS_OFFSET_EN,0, DAIF_PDTIF_CALI_MO,0, DAIF_PDTIF_CALI_ME,1));

        //polling agc idle
        co_delay_10us(1);
        HS_DAIF->DBG_REG = 0x1B;
        while(HS_DAIF->DBG_REG & (0xF<<16));

        REGW(&HS_DAIF->AGC_CFG0, MASK_1REG(DAIF_OS_OFFSET_EN,1));

        //polling agc done
        co_delay_10us(1);
        HS_DAIF->DBG_REG = 0x1B;
        while(HS_DAIF->DBG_REG & (0xF<<16));
        HS_DAIF->DBG_REG = 0;

        // save calib data
        agc_pif_os[i] = HS_DAIF->AGC_CFG3 & DAIF_PIF_OS_MASK;
    }

#ifdef CONFIG_HS6621
    calib_env.rf.agc_pif_os = (agc_pif_os[0] + agc_pif_os[1]) / 2;
    REGW(&HS_DAIF->AGC_CFG2, MASK_2REG(DAIF_PIF_OS_FLAG,1, DAIF_PIF_OS_REG,calib_env.rf.agc_pif_os));
#else
    calib_env.rf.agc_pif_os = agc_pif_os[0];
    calib_env.rf.agc_pif_os_2m = agc_pif_os[1];
    REGW(&HS_DAIF->AGC_CFG2, MASK_2REG(DAIF_PIF_OS_FLAG,1, DAIF_PIF_OS_REG,calib_env.rf.agc_pif_os));
    REGW(&HS_DAIF->AGC_CFG3, MASK_1REG(DAIF_PIF_OS_REG_2M, calib_env.rf.agc_pif_os_2m));
#endif

    REGW(&HS_DAIF->VCO_CTRL0, MASK_3REG(DAIF_TRX_DBG,0, DAIF_RX_EN_MO,0, DAIF_TX_EN_MO,0));

    REGW(&HS_DAIF->PD_CFG0, MASK_3REG(DAIF_PD_AGCAMP_ME,0, DAIF_PD_PDTIF_ME,0, DAIF_PKDADC_PD_ME,0));
    REGW(&HS_DAIF->PD_CFG1, MASK_3REG(DAIF_PD_LDO_IF_ME,0, DAIF_PD_LDO_ANADIG_ME,0, DAIF_PD_LDO_PKDADC_ME,0));
    REGW(&HS_DAIF->AGC_CFG0, MASK_2REG(DAIF_OS_OFFSET_EN,0, DAIF_PDTIF_CALI_ME,0));

    REGW(&HS_DAIF->TRX_SW_CFG, MASK_1REG(DAIF_TRX_EN_ME,0));
    REGW(&HS_DAIF->FREQ_CFG0, MASK_1REG(DAIF_FREQ_REG_ME,0));
    REGW(&HS_DAIF->PD_CFG1, MASK_1REG(DAIF_RST_PDTIF_ME,0));

    REGW(&HS_DAIF->LNA_MIX_CFG, MASK_3REG(DAIF_GAIN_LNA_ME,0, DAIF_GAIN_ATTEN_ME,0, DAIF_EN_ATTEN_ME,0));
    REGW(&HS_DAIF->TIA_LPF_CFG, MASK_1REG(DAIF_GAIN_TIA_ME,0));
}
#endif

/**
 * @brief  calib rf dcoc diff get
 *
 * @param[in] i  i
 * @param[in] q  q
 * @param[in] idiff  idiff
 * @param[in] qdiff  qdiff
 **/
static void calib_rf_dcoc_diff_get(uint32_t i, uint32_t q, uint16_t *idiff, uint16_t *qdiff)
{
    uint32_t ireg=0, qreg=0, n;

    REGW(&HS_DAIF->DCOC_CFG, MASK_2REG(DAIF_DIN_DCOC_I_MO,i, DAIF_DIN_DCOC_Q_MO,q));
    REGW0(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK);
    co_delay_10us(1);

    for (n=0; n<20; ++n)
    {
        __IO uint32_t RX_ADC_OUT = HS_DAIF->RX_ADC_OUT;
        if (idiff)
            ireg += REGR(&RX_ADC_OUT, MASK_POS(DAIF_RXADC_IOUT));
        if (qdiff)
            qreg += REGR(&RX_ADC_OUT, MASK_POS(DAIF_RXADC_QOUT));
    }

    if (idiff)
    {
        ireg /= 20;
        *idiff = ireg>256 ? ireg-256 : 256-ireg;
    }

    if(qdiff)
    {
        qreg /= 20;
        *qdiff = qreg>256 ? qreg-256 : 256-qreg;
    }

    REGW1(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK);
}

/**
 * @brief calib_rf_dcoc()
 *
 * @return 
 **/
static void calib_rf_dcoc(void)
{
    int filt_gidx, rx_gidx, sel_dcoc_fs, lut_idx, iq, rxout;

    // Not fsm ctrl
    HS_DAIF->TRX_SW_CFG = 2;

    //power on analog blocks
    // enable trx dbg and rx_en
    REGW1(&HS_DAIF->VCO_CTRL0, DAIF_RX_EN_MO_MASK | DAIF_TRX_DBG_MASK);
    co_delay_10us(12); // wait 80us to make sure pll stable

    // reset adc > 1
    REGW1(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK | DAIF_RST_ME_MASK);

    REGW(&HS_DAIF->DCOC_CFG, MASK_2REG(DAIF_DIN_DCOC_I_ME,1, DAIF_DIN_DCOC_Q_ME,1));
    REGW(&HS_DAIF->DCOC_CFG1, MASK_2REG(DAIF_OFFSET_I_ME,1, DAIF_OFFSET_Q_ME, 1));

    // ana dcoc I/Q
    sel_dcoc_fs = 0;
    REGW(&HS_DAIF->DCOC_CFG, MASK_1REG(DAIF_SEL_DCOC_FS, sel_dcoc_fs));
    REGW(&HS_DAIF->TIA_LPF_CFG, MASK_3REG(DAIF_FILTER_LP_CP_SEL,1, DAIF_TIA_LP_CP_SEL,1, DAIF_TIA_SWAP,1));
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION==1)
    filt_gidx = 0;
#else
    filt_gidx = 2;
#endif
    {
        REGW(&HS_DAIF->TIA_LPF_CFG, MASK_2REG(DAIF_GAIN_LPF_ME,1, DAIF_GAIN_LPF_MO,filt_gidx));

        for(rx_gidx = 0; rx_gidx <= 8; rx_gidx++)
        {
            lut_idx = filt_gidx * 9 + rx_gidx;

#if 0
            // enable en_dcoc 0
            REGW(&HS_DAIF->DCOC_CFG, MASK_4REG(
                        DAIF_DCOC_CALI, 1u,
                        DAIF_DCOC_FILT_GIDX, filt_gidx,
                        DAIF_DCOC_RX_GIDX, rx_gidx,
                        DAIF_EN_DCOC, 0));

            REGW0(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK);

            co_delay_10us(3);

            // reset adc > 0
            uint32_t rxadc_qout_reg = REGR(&HS_DAIF->RX_ADC_OUT, MASK_POS(DAIF_RXADC_QOUT));
            uint32_t rxadc_iout_reg = REGR(&HS_DAIF->RX_ADC_OUT, MASK_POS(DAIF_RXADC_IOUT));

            uint32_t adcoc_i = adcoc(rxadc_iout_reg, sel_dcoc_fs);
            uint32_t adcoc_q = adcoc(rxadc_qout_reg, sel_dcoc_fs);

            REGW(&HS_DAIF->DCOC_LUT[lut_idx], MASK_2REG(
                        DAIF_ANA_DCOC_I, adcoc_i,
                        DAIF_ANA_DCOC_Q, adcoc_q));

            REGW1(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK);
#else
            uint16_t idiff1=0xFFFF, qdiff1=0xFFFF, idiff2=0xFFFF, qdiff2=0xFFFF, diff;
            uint16_t iindex1=0, qindex1=0, iindex2=0, qindex2=0;
            uint32_t i;

            // enable en_dcoc 0
            REGW(&HS_DAIF->DCOC_CFG, MASK_6REG(
                        DAIF_DCOC_CALI, 1u,
                        DAIF_DCOC_FILT_GIDX, filt_gidx,
                        DAIF_DCOC_RX_GIDX, rx_gidx,
                        DAIF_EN_DCOC, 1,
                        DAIF_DIN_DCOC_I_ME, 1,
                        DAIF_DIN_DCOC_Q_ME, 1));

            calib_log("-----------------filt_gidx=%d rx_gidx=%d-----------------FILTER=%d TIA=%d SWAP=%d-----------------\n",
                    filt_gidx, rx_gidx,
                    REGR(&HS_DAIF->TIA_LPF_CFG, MASK_POS(DAIF_FILTER_LP_CP_SEL)),
                    REGR(&HS_DAIF->TIA_LPF_CFG, MASK_POS(DAIF_TIA_LP_CP_SEL)),
                    REGR(&HS_DAIF->TIA_LPF_CFG, MASK_POS(DAIF_TIA_SWAP)));

            for (i=0; i<0x20; ++i)
            {
                calib_rf_dcoc_diff_get(i, 0x1F, &diff, NULL);
                if (diff < idiff1)
                {
                    idiff1 = diff;
                    iindex1 = i;
                }
            }
            calib_log("\n");

            for (i=0; i<0x20; ++i)
            {
                calib_rf_dcoc_diff_get(iindex1, i, NULL, &diff);
                if (diff < qdiff1)
                {
                    qdiff1 = diff;
                    qindex1 = i;
                }
            }
            calib_log("\n");

            for (i=0; i<0x20; ++i)
            {
                calib_rf_dcoc_diff_get(i, 0x1F, NULL, &diff);
                if (diff < idiff2)
                {
                    idiff2 = diff;
                    iindex2 = i;
                }
            }
            calib_log("\n");

            for (i=0; i<0x20; ++i)
            {
                calib_rf_dcoc_diff_get(iindex2, i, &diff, NULL);
                if (diff < qdiff2)
                {
                    qdiff2 = diff;
                    qindex2 = i;
                }
            }
            calib_log("\n");

            calib_rf_dcoc_diff_get(iindex1, qindex1, &idiff1, &qdiff1);

            calib_rf_dcoc_diff_get(iindex2, qindex2, &idiff2, &qdiff2);

            if(MAX(idiff1, qdiff1) < MAX(idiff2, qdiff2))
                REGW(&HS_DAIF->DCOC_LUT[lut_idx], MASK_2REG(DAIF_ANA_DCOC_I,iindex1, DAIF_ANA_DCOC_Q,qindex1));
            else
                REGW(&HS_DAIF->DCOC_LUT[lut_idx], MASK_2REG(DAIF_ANA_DCOC_I,iindex2, DAIF_ANA_DCOC_Q,qindex2));
#endif
        }
    }

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION==1)
    for (filt_gidx = 1; filt_gidx <= 6; filt_gidx++)
    {
        for(rx_gidx = 0; rx_gidx <= 8; rx_gidx++)
        {
            lut_idx = filt_gidx * 9 + rx_gidx;
            HS_DAIF->DCOC_LUT[lut_idx] = HS_DAIF->DCOC_LUT[rx_gidx];
        }
    }
#else
    for (filt_gidx = 0; filt_gidx <= 6; filt_gidx++)
    {
        if (filt_gidx == 2)
            continue;

        for(rx_gidx = 0; rx_gidx <= 8; rx_gidx++)
        {
            lut_idx = filt_gidx * 9 + rx_gidx;
            HS_DAIF->DCOC_LUT[lut_idx] = HS_DAIF->DCOC_LUT[2*9 + rx_gidx];
        }
    }
#endif

    // HWctrl
    REGW(&HS_DAIF->DCOC_CFG, MASK_2REG(DAIF_DIN_DCOC_I_ME,0, DAIF_DIN_DCOC_Q_ME,0));

//    // enable phy 16m
//    REGW0(&HS_PSO->BTPHY_CFG, CPM_PHY_GATE_EN_MASK);
//    HS_PSO_XTAL32M_UPD_RDY();

    // digi dcoc I/Q
    REGW(&HS_DAIF->TIA_LPF_CFG, MASK_2REG(DAIF_FILTER_LP_CP_SEL,1, DAIF_TIA_LP_CP_SEL,1));
    for (iq=0; iq<2; ++iq)
    {
        HS_PHY->IQ_IN_SWAP = iq;
        for(filt_gidx = 0; filt_gidx <= 6; filt_gidx++)
        {
            for(rx_gidx = 0; rx_gidx <= 8; rx_gidx++)
            {
                // enable en_dcoc 1
                REGW(&HS_DAIF->DCOC_CFG, MASK_4REG(
                            DAIF_DCOC_CALI, 1u,
                            DAIF_DCOC_FILT_GIDX, filt_gidx,
                            DAIF_DCOC_RX_GIDX, rx_gidx,
                            DAIF_EN_DCOC, 1));

                REGW1(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK);

                // 10us;
                co_delay_10us(1);

                // reset adc > 0
                REGW0(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK);

                // 30us;
                co_delay_10us(3);

                rxout= ((HS_PHY->RX_OUT >> 3) & 0x7F) | ((HS_PHY->RX_OUT >> 4) & 0x80); // {phy_rdata[11], phy_rdata[9:3]}
                if(iq)
                    REGW(&HS_DAIF->DCOC_LUT[lut_idx], MASK_1REG(DAIF_RXQ_OFFSET, rxout));
                else
                    REGW(&HS_DAIF->DCOC_LUT[lut_idx], MASK_1REG(DAIF_RXI_OFFSET, rxout));
            }
        }
    }

    // HWctrl
    REGW0(&HS_DAIF->VCO_CTRL0, DAIF_RX_EN_MO_MASK | DAIF_TRX_DBG_MASK);
    // HWctrl
    REGW(&HS_DAIF->DCOC_CFG1, MASK_2REG(DAIF_OFFSET_I_ME,0, DAIF_OFFSET_Q_ME,0));
    // HWctrl
    REGW0(&HS_DAIF->RXADC_CTRL, DAIF_RST_MO_MASK | DAIF_RST_ME_MASK);

//    // Disable phy 16m
//    REGW1(&HS_PSO->BTPHY_CFG, CPM_PHY_GATE_EN_MASK);
//    HS_PSO_XTAL32M_UPD_RDY();

    // fsm
    HS_DAIF->TRX_SW_CFG = 0;

    // Disable dcdc cali
    REGW(&HS_DAIF->DCOC_CFG, MASK_1REG(DAIF_DCOC_CALI, 0));
}

/**
 * @brief  calib txfe
 **/
static void calib_rf_txfe(void)
{
    int i;
    uint32_t padr1s_ct_val, padr2s_pct_val, padr2s_nct_val, pacore_ct_val, tx_atten_val;

    // Power on
    REGW(&HS_DAIF->PD_CFG0, MASK_6REG(DAIF_PD_TXCALI_MO,0, DAIF_PD_TXCALI_ME,1, DAIF_PD_LDO_PA_MO,0, DAIF_PD_LDO_PA_ME,1u, DAIF_PD_PA_MO,0, DAIF_PD_PA_ME,1));
    REGW(&HS_DAIF->PD_CFG1, MASK_6REG(DAIF_PD_LDO_IF_MO,0, DAIF_PD_LDO_IF_ME,1, DAIF_PD_PLL_ALL_MO,0, DAIF_PD_PLL_ALL_ME,1, DAIF_PD_LOTX_MO,0, DAIF_PD_LOTX_ME,1));
    REGW(&HS_DAIF->PD_CFG2, MASK_2REG(DAIF_PD_PLL_BUF_MO,0, DAIF_PD_PLL_BUF_ME,1));

    co_delay_us(10);

    //tx fe loop
    for(i=17; i>=0; i--)
    {
        // tx fe calib for gain

        padr2s_pct_val = 8;
        padr2s_nct_val = 8;
        padr1s_ct_val  = txfe_pa_table[i].padr1s_ct_mo;
        pacore_ct_val  = txfe_pa_table[i].pacore_ct_mo;
        tx_atten_val   = txfe_pa_table[i].tx_atten_mo;

        REGW(&HS_DAIF->PA_CNS, MASK_6REG(DAIF_PA_DBG,1, DAIF_PADR1S_CT_MO,padr1s_ct_val, DAIF_PADR2S_PCT_MO,padr2s_pct_val,
                    DAIF_PADR2S_NCT_MO,padr2s_nct_val, DAIF_PACORE_CT_MO,pacore_ct_val, DAIF_TX_ATTEN_MO,tx_atten_val));
        co_delay_us(2);

        if(HS_DAIF->PA_CNS & DAIF_TXCALI_OUT_SYNC_MASK)
        {
            while(1)
            {
                if(--padr2s_nct_val == 0)
                    break;
                REGW(&HS_DAIF->PA_CNS, MASK_6REG(DAIF_PA_DBG,1, DAIF_PADR1S_CT_MO,padr1s_ct_val, DAIF_PADR2S_PCT_MO,padr2s_pct_val,
                            DAIF_PADR2S_NCT_MO,padr2s_nct_val, DAIF_PACORE_CT_MO,pacore_ct_val, DAIF_TX_ATTEN_MO,tx_atten_val));
                co_delay_us(2);
                if(!(HS_DAIF->PA_CNS & DAIF_TXCALI_OUT_SYNC_MASK))
                    break;

                if(++padr2s_pct_val == 16)
                    break;
                REGW(&HS_DAIF->PA_CNS, MASK_6REG(DAIF_PA_DBG,1, DAIF_PADR1S_CT_MO,padr1s_ct_val, DAIF_PADR2S_PCT_MO,padr2s_pct_val,
                            DAIF_PADR2S_NCT_MO,padr2s_nct_val, DAIF_PACORE_CT_MO,pacore_ct_val, DAIF_TX_ATTEN_MO,tx_atten_val));
                co_delay_us(2);
                if(!(HS_DAIF->PA_CNS & DAIF_TXCALI_OUT_SYNC_MASK))
                    break;
            }
        }
        else
        {
            while(1)
            {
                if(--padr2s_pct_val == 0)
                    break;
                REGW(&HS_DAIF->PA_CNS, MASK_6REG(DAIF_PA_DBG,1, DAIF_PADR1S_CT_MO,padr1s_ct_val, DAIF_PADR2S_PCT_MO,padr2s_pct_val,
                            DAIF_PADR2S_NCT_MO,padr2s_nct_val, DAIF_PACORE_CT_MO,pacore_ct_val, DAIF_TX_ATTEN_MO,tx_atten_val));
                co_delay_us(2);
                if(HS_DAIF->PA_CNS & DAIF_TXCALI_OUT_SYNC_MASK)
                    break;

                if(++padr2s_nct_val == 16)
                    break;
                REGW(&HS_DAIF->PA_CNS, MASK_6REG(DAIF_PA_DBG,1, DAIF_PADR1S_CT_MO,padr1s_ct_val, DAIF_PADR2S_PCT_MO,padr2s_pct_val,
                            DAIF_PADR2S_NCT_MO,padr2s_nct_val, DAIF_PACORE_CT_MO,pacore_ct_val, DAIF_TX_ATTEN_MO,tx_atten_val));
                co_delay_us(2);
                if(HS_DAIF->PA_CNS & DAIF_TXCALI_OUT_SYNC_MASK)
                    break;
            }
        }

//        log_info("p=%d n=%d\n", padr2s_pct_val, padr2s_nct_val);

        if(padr2s_pct_val == 0)  padr2s_pct_val = 1;
        if(padr2s_nct_val == 0)  padr2s_nct_val = 1;
        if(padr2s_pct_val == 16) padr2s_pct_val = 15;
        if(padr2s_nct_val == 16) padr2s_nct_val = 15;

        REGW(&HS_DAIF->PA_GAIN_REG[17-i], MASK_5REG(
                DAIF_PA_GAIN_PADR1S_CT_MO,  padr1s_ct_val,
                DAIF_PA_GAIN_PADR2S_PCT_MO, padr2s_pct_val,
                DAIF_PA_GAIN_PADR2S_NCT_MO, padr2s_nct_val,
                DAIF_PA_GAIN_PACORE_CT_MO,  pacore_ct_val,
                DAIF_PA_GAIN_TX_ATTEN_MO,   tx_atten_val));

        //some delay between each gain
        co_delay_us(5);
    }

    // HWctrl
    REGW(&HS_DAIF->PD_CFG0, MASK_3REG(DAIF_PD_TXCALI_ME,0, DAIF_PD_LDO_PA_ME,0, DAIF_PD_PA_ME,0));
    REGW(&HS_DAIF->PD_CFG1, MASK_3REG(DAIF_PD_LDO_IF_ME,0, DAIF_PD_PLL_ALL_ME,0, DAIF_PD_LOTX_ME,0));
    REGW(&HS_DAIF->PD_CFG2, MASK_1REG(DAIF_PD_PLL_BUF_ME,0));

    //clear pa_dbg
    REGW(&HS_DAIF->PA_CNS, MASK_1REG(DAIF_PA_DBG, 0));
}

/**
 * @brief calib_rf_store()
 *
 * @return 
 **/
static void calib_rf_store(void)
{
    int i;

    // vco afc
    for(i=0; i<80; i++)
    {
        REGWA(&HS_DAIF->PLL_LUT_DBG, MASK_2REG(DAIF_PLL_LUT_LU,1, DAIF_PLL_LUT_IDX,i));
        while(HS_DAIF->PLL_LUT_DBG & DAIF_PLL_LUT_LU_MASK);
        calib_env.rf.vco_afc_lut[i] = HS_DAIF->PLL_LUT_DBG & DAIF_PLL_LUT_DATA_MASK;
    }

    // vco amp
    HS_DAIF->DBG_REG = 0x18;
    calib_env.rf.vco_amp = (HS_DAIF->DBG_REG >> 26) & 0x7;
    HS_DAIF->DBG_REG = 0;

    // kdco 1M
    calib_env.rf.kdco_1m[0] = HS_DAIF->KDCO_LUT_1M_REG0;
    calib_env.rf.kdco_1m[1] = HS_DAIF->KDCO_LUT_1M_REG1;

    // kdco 2M
    calib_env.rf.kdco_2m[0] = HS_DAIF->KDCO_LUT_2M_REG0;
    calib_env.rf.kdco_2m[1] = HS_DAIF->KDCO_LUT_2M_REG1;

    // AGC save (saved in calib_rf_agc)
//    calib_env.rf.agc_pif_os = HS_DAIF->AGC_CFG3 & DAIF_PIF_OS_MASK;

    // DCDC save
    for(i=0; i<63; ++i)
        calib_env.rf.dcoc_lut[i] = HS_DAIF->DCOC_LUT[i];

    // TXFE
    for(i=0; i<18; ++i)
        calib_env.rf.txfe_pa_gain[i] = HS_DAIF->PA_GAIN_REG[i];

    // afc en = 0
    REGW(&HS_DAIF->VCO_CTRL0, MASK_2REG(DAIF_AFC_EN_MO,0, DAIF_AFC_EN_ME,1));

    // PA ctrl by RW
    // SEL=0: REG ctrl PA
    // SEL=1: RW ctrl PA
    REGW1(&HS_DAIF->PA_CNS, DAIF_PA_GAIN_IDX_SEL_MASK);
    // REGW(&HS_DAIF->PA_CNS, MASK_2REG(DAIF_PA_GAIN_IDX_SEL,0, DAIF_PA_GAIN_IDX_REG,18));

    // save PA_CNS value
    calib_env.rf.pa_cns = HS_DAIF->PA_CNS;
}

#if 0
/**
 * @brief calib_sys_store()
 *
 * store: xtal24m_ldo, rc, rc24m, (rc32k)
 *
 * @return None
 **/
static void calib_sys_store(void)
{
    // rc
    calib_env.sys.rc_tia_lpf = HS_DAIF->TIA_LPF_CFG;

    // rc32m
    REGW(&HS_PMU->CLK_CTRL_1, MASK_2REG(PMU_CTUNE_OSC_SEL, 1,
                PMU_REG_CTUNE_OSC, REGR(&HS_DAIF->RC32M_TUN,MASK_POS(DAIF_RC_32M_TUNE))));
}
#endif

/**
 * @brief  calib rc32k accuracy check
 *
 * @return accuracy
 **/
static uint32_t calib_rc32k_count_by_xtal32m(uint32_t win_32k_num)
{
    //check rc32k clock again with intr enabled
    REGWA(&HS_DAIF->CLK_CHK_CNS, MASK_5REG(
                DAIF_START,1, DAIF_INTR_EN,1, DAIF_INTR_ST,0, DAIF_REF_SEL,0,
                DAIF_WIN_CNT_THRSH, win_32k_num)); // win_cnt: 32k period number
    // wait rc 32k check done -- 2
    while(HS_DAIF->CLK_CHK_CNS & DAIF_START_MASK);

    // got rc 32k check status
    return HS_DAIF->CLK_CHK_STATUS; // in win_cnt, how many 32MHz number
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief  calib rc32k accuracy check
 *
 * @return ppm
 **/
int calib_rc32k_accuracy_check(uint32_t win_32k_num)
{
    // >>> 16*(1.0/32768.0)/(1.0/32000000)
    // 15625.0 (62ppm)
    // >>> 32*(1.0/32768.0)/(1.0/32000000)
    // 31250.0 (32ppm)
    // >>> 64*(1.0/32768.0)/(1.0/32000000)
    // 62500.0 (15ppm)

    int clk_num_32m;
    int clk_num_32m_std = win_32k_num * 32000000LL / 32768;

    CPM_ANA_CLK_ENABLE();

    REGW1(&HS_DAIF->CLK_ENS, DAIF_CC_CLK_EN_MASK);

    clk_num_32m = calib_rc32k_count_by_xtal32m(win_32k_num);

    REGW0(&HS_DAIF->CLK_ENS, DAIF_CC_CLK_EN_MASK);

    CPM_ANA_CLK_RESTORE();

    return 1000000 * (clk_num_32m - clk_num_32m_std) / clk_num_32m_std;
}

#ifdef CONFIG_HS6621
/**
 * @brief  calib rc32k to 62ppm
 **/
void calib_rc32k(void)
{
    // >>> 2*(1.0/32768.0)/(1.0/32000000)
    // 1953.125
    // >>> 100*(1.0/32768.0)/(1.0/32000000)
    // 97656.25
    //
    // CTUNE: 2-window
    // RTUNE: 100-window
    //

    uint32_t ct  = 0x7F;
    uint32_t cb  = 0x00;
    uint32_t ctn = 0;
    uint32_t cbn = 0;
    uint32_t c   = 0x40;
    uint32_t rt  = 0xFF;
    uint32_t rb  = 0x00;
    uint32_t rtn = 0;
    uint32_t rbn = 0;
    uint32_t r   = 0x80;
    uint32_t n;

    CPM_ANA_CLK_ENABLE();

    // Eanble
    REGW1(&HS_DAIF->CLK_ENS, DAIF_CC_CLK_EN_MASK);

    // REG ctrl
    REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_RC_32K_RCTUNE_SEL, 1));

    while(1)
    {
        REGW(&HS_PMU->CLK_CTRL_2, MASK_2REG(PMU_RC_32K_RTUNE,r, PMU_RC_32K_CTUNE,c));
        co_delay_10us(6);

        n = calib_rc32k_count_by_xtal32m(2);

        if (n > 1953)
        {
            ct  = c;
            ctn = n;
        }
        else if (n == 1953)
        {
            break;
        }
        else
        {
            cb  = c;
            cbn = n;
        }

        c = (cb + ct) >> 1;

        if (ct - cb == 0)
        {
            break;
        }
        else if (ct - cb == 1)
        {
            ctn = (1953 > ctn) ? (1953 - ctn) : (ctn - 1953);
            cbn = (1953 > cbn) ? (1953 - cbn) : (cbn - 1953);
            c   = (ctn > cbn) ? cb : ct;
            break;
        }
    }

    while(1)
    {
        REGW(&HS_PMU->CLK_CTRL_2, MASK_2REG(PMU_RC_32K_RTUNE,r, PMU_RC_32K_CTUNE,c));
        co_delay_10us(6);

        n = calib_rc32k_count_by_xtal32m(100);

        if (n > 97656)
        {
            rt  = r;
            rtn = n;
        }
        else if (n == 97656)
        {
            break;
        }
        else
        {
            rb  = r;
            rbn = n;
        }

        r = (rb + rt) >> 1;

        if (rt - rb == 0)
        {
            break;
        }
        else if (rt - rb == 1)
        {
            rtn = (97656 > rtn) ? (97656 - rtn) : (rtn - 97656);
            rbn = (97656 > rbn) ? (97656 - rbn) : (rbn - 97656);
            r   = (rtn > rbn) ? rb : rt;
            break;
        }

    }

    REGW(&HS_PMU->CLK_CTRL_2, MASK_2REG(PMU_RC_32K_RTUNE,r, PMU_RC_32K_CTUNE,c));

    // Disable
    REGW0(&HS_DAIF->CLK_ENS, DAIF_CC_CLK_EN_MASK);

    CPM_ANA_CLK_RESTORE();

    // Must delay 1 32k to switch it
    co_delay_10us(6);
}
#else
#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief  calib rc32k to 62ppm
 **/
void calib_rc32k(void)
{
    CPM_ANA_CLK_ENABLE();

    // Eanble
    REGW1(&HS_DAIF->CLK_ENS, DAIF_RC_32K_TUNE_CLK_EN_MASK);

    // PMU reg bypass disable
    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_RC_32K_RCTUNE_SEL, 0));
    co_delay_10us(6);

    // un-reset
    REGW1(&HS_DAIF->RC32K_TUN, DAIF_SWRSTN_MASK);

    //rc 32k tune
    // do rc 32k tune 1
    REGW1(&HS_DAIF->RC32K_TUN, DAIF_START_MASK);

    // wait rc32k tune 1 done
    while(HS_DAIF->RC32K_TUN & DAIF_START_MASK);

    // Save
    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_RC_32K_RCTUNE_SEL, 1));
    HREGW(&HS_HIB->CONFIG,   MASK_1REG(HIB_CONFIG_RTUNE_RC32K, REGR(&HS_DAIF->RC32K_TUN_OUT, MASK_POS(DAIF_RTUNE))));
    HREGW(&HS_HIB->CONFIG_1, MASK_1REG(HIB_CONFIG_CTUNE_RC32K, REGR(&HS_DAIF->RC32K_TUN_OUT, MASK_POS(DAIF_CTUNE))));

    // reset
    REGW0(&HS_DAIF->RC32K_TUN, DAIF_SWRSTN_MASK);

    // Disable
    REGW0(&HS_DAIF->CLK_ENS, DAIF_RC_32K_TUNE_CLK_EN_MASK);

    CPM_ANA_CLK_RESTORE();

    // Must delay 1 32k to switch it
    co_delay_10us(6);
}
#endif //}BASE_COMPONENT_END
#endif

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief calib sys rc32m
 **/
void calib_sys_rc32m(void)
{
    // clock
    REGW1(&HS_DAIF->CLK_ENS, DAIF_RC_32M_TUNE_CLK_EN_MASK | DAIF_RC_CALIB_CLK_EN_MASK);

    // FSM
    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_CTUNE_OSC_SEL, 0));

    //rc 32m tune
    // do rc 32m tune 1
    REGW1(&HS_DAIF->RC32M_TUN, DAIF_RC_32M_TUNE_START_MASK);
    // wait rc32m tune 1 done
    while(HS_DAIF->RC32M_TUN & DAIF_RC_32M_TUNE_START_MASK);

    // save
    REGW(&HS_PMU->CLK_CTRL_1, MASK_2REG(PMU_CTUNE_OSC_SEL, 1,
                PMU_REG_CTUNE_OSC, REGR(&HS_DAIF->RC32M_TUN,MASK_POS(DAIF_RC_32M_TUNE))));
}

/**
 * @brief calib sys rc
 **/
void calib_sys_rc(void)
{
    // clock
    REGW1(&HS_DAIF->CLK_ENS, DAIF_RC_32M_TUNE_CLK_EN_MASK | DAIF_RC_CALIB_CLK_EN_MASK);

    // SWctrl
    REGW(&HS_DAIF->PD_CFG1, MASK_2REG(DAIF_PD_LDO_IF_MO,0, DAIF_PD_LDO_IF_ME,1));

    //rc calib
    REGW1(&HS_DAIF->RX_RCCAL_CTRL, DAIF_START_MASK);
    // wait rc calib done
    while(HS_DAIF->RX_RCCAL_CTRL & DAIF_START_MASK);

    // rc calib result
    uint32_t rc_sum = REGR(&HS_DAIF->RX_RCCAL_STAT, MASK_POS(DAIF_CNT_T1)) + REGR(&HS_DAIF->RX_RCCAL_STAT, MASK_POS(DAIF_CNT_T2));
    rc_sum /= 2;

    // t1+t2  RC value  lpf_ctune  tia_ctune  rc_tune_auadc
    // 538    -32%      0          0          0
    // 522    -28%      1          1          1
    // 506    -24%      10         10         10
    // 490    -20%      11         11         11
    // 474    -16%      100        100        100
    // 458    -12%      101        101        101
    // 442    -8%       110        110        110
    // 426    -4%       111        111        111
    // 410    0         1000       1000       1000
    // 394    4%        1001       1001       1001
    // 378    8%        1010       1010       1010
    // 362    12%       1011       1011       1011
    // 346    16%       1100       1100       1100
    // 330    20%       1101       1101       1101
    // 314    24%       1110       1110       1110
    // 298    28%       1111       1111       1111
    RC_IF_GREATER_DO(538-8, 0,  0,  0 )
    RC_EI_GREATER_DO(522-8, 1,  1,  1 )
    RC_EI_GREATER_DO(506-8, 2,  2,  2 )
    RC_EI_GREATER_DO(490-8, 3,  3,  3 )
    RC_EI_GREATER_DO(474-8, 4,  4,  4 )
    RC_EI_GREATER_DO(458-8, 5,  5,  5 )
    RC_EI_GREATER_DO(442-8, 6,  6,  6 )
    RC_EI_GREATER_DO(426-8, 7,  7,  7 )
    RC_EI_GREATER_DO(410-8, 8,  8,  8 )
    RC_EI_GREATER_DO(394-8, 9,  9,  9 )
    RC_EI_GREATER_DO(378-8, 10, 10, 10)
    RC_EI_GREATER_DO(362-8, 11, 11, 11)
    RC_EI_GREATER_DO(346-8, 12, 12, 12)
    RC_EI_GREATER_DO(330-8, 13, 13, 13)
    RC_EI_GREATER_DO(314-8, 14, 14, 14)
    RC_EL_GREATER_DO(       15, 15, 15)

    // HWctrl
    REGW(&HS_DAIF->PD_CFG1, MASK_2REG(DAIF_PD_LDO_IF_MO,0, DAIF_PD_LDO_IF_ME,0));

    // save
    calib_env.sys.rc_tia_lpf = HS_DAIF->TIA_LPF_CFG;
}
#endif //}BASE_COMPONENT_END

/**
 * @brief calib_rf()
 *
 * @return 
 **/
void calib_rf(void)
{
    if(IS_FPGA())
        return;

    pmu_ana_enable(true, PMU_ANA_RF);

    // enable clock
    CPM_ANA_CLK_ENABLE_NOIRQ();
    uint32_t clk_ens_save = HS_DAIF->CLK_ENS;
    REGW1(&HS_DAIF->CLK_ENS, DAIF_PLL_AFC_CLK_EN_MASK | DAIF_PLL_AMP_CLK_EN_MASK | DAIF_RX_AGC_CLK_EN_MASK |
                DAIF_PLL_VTRACK_CLK_EN_MASK | DAIF_PLL_LUT_CLK_EN_MASK |
                DAIF_PLL_CLK_AFC_EN_MASK | DAIF_SDM_CLK_EN_MASK | DAIF_PLL_CLK_REF_EN_MASK);

    // patch
    calib_patch(calib_repair_env.temperature);

    // do calib
    calib_rf_afc();
    calib_rf_dcoc();
#if !(defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION==1)) // A1 BUG: AGC calib can't finish
    calib_rf_agc();
#endif
    calib_rf_txfe();

    // store calib value
    calib_rf_store();

    // patch
    calib_patch(calib_repair_env.temperature);

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION==1)
    // fix AGC issue
    calib_env.rf.agc_pif_os = 0x13A;
    REGW(&HS_DAIF->AGC_CFG2, MASK_2REG(DAIF_PIF_OS_FLAG,1, DAIF_PIF_OS_REG,calib_env.rf.agc_pif_os));
#else
    REGW(&HS_DAIF->VCO_CTRL0, MASK_1REG(DAIF_VTRACK_EN,1));
#endif

    // restore clock
    HS_DAIF->CLK_ENS = clk_ens_save;
    CPM_ANA_CLK_RESTORE_NOIRQ();
}

/**
 * @brief  calib rf pa save
 **/
void calib_rf_pa_save(void)
{
    // save PA_CNS value
    calib_env.rf.pa_cns = HS_DAIF->PA_CNS;
}

#ifdef CONFIG_CALIB_FLASH_SAVE
/**
 * @brief  calib flash cfg tag get
 *
 * @return
 **/
static int calib_flash_cfg_tag_get(void)
{
    return calib_repair_env.temperature>70 ? CFG_TAG_CALIB_DATA_HIGH_T : CFG_TAG_CALIB_DATA_NORMAL_T;
}

/**
 * @brief  calib flash is saved
 *
 * @return
 **/
static bool calib_flash_is_saved(void)
{
    int res = cfg_get(CFG_SECTION_SYS, calib_flash_cfg_tag_get(), NULL, NULL);
    return res>=0 ? true : false;
}

/**
 * @brief  calib flash save force
 **/
static void calib_flash_save_force(void)
{
    calib_flash_data_t data;

    data.save = calib_env;
    data.pmu.ct_fdoubler = REGR(&HS_PMU->CLK_CTRL_1, MASK_POS(PMU_REG_CTUNE_FDOUB));
    data.hib.rtune_rc32k = HREGR(&HS_HIB->CONFIG,    MASK_POS(HIB_CONFIG_RTUNE_RC32K));
    data.hib.ctune_rc32k = HREGR(&HS_HIB->CONFIG_1,  MASK_POS(HIB_CONFIG_CTUNE_RC32K));

    data.crc16 = 0;
    data.crc16 = co_crc16_ccitt(0, &data, CALIB_FLASH_LENGTH);

    if (CALIB_FLASH_EXTRA_LENGTH)
    {
        uint8_t *pdata = (uint8_t *)&data;
        cfg_put(CFG_SECTION_SYS, calib_flash_cfg_tag_get(), pdata, CFG_TAG_LENGTH_MAX);
        cfg_put(CFG_SECTION_SYS, calib_flash_cfg_tag_get()+1, pdata+CFG_TAG_LENGTH_MAX, CALIB_FLASH_EXTRA_LENGTH);
    }
    else
    {
        cfg_put(CFG_SECTION_SYS, calib_flash_cfg_tag_get(), &data, CALIB_FLASH_LENGTH);
    }
}

/**
 * @brief  calib flash save
 **/
void calib_flash_save(void)
{
    if(!calib_flash_is_saved())
        calib_flash_save_force();
}

/**
 * @brief  calib flash restore
 *
 * @return
 **/
bool calib_flash_restore(void)
{
    calib_flash_data_t data;
    uint16_t length, crc16_flash, crc16_calc;
    int res;

    if (CALIB_FLASH_EXTRA_LENGTH)
    {
        uint8_t *pdata = (uint8_t *)&data;

        length = CFG_TAG_LENGTH_MAX;
        res = cfg_get(CFG_SECTION_SYS, calib_flash_cfg_tag_get(), pdata, &length);
        if(res < 0) return false;

        length = CALIB_FLASH_EXTRA_LENGTH;
        res = cfg_get(CFG_SECTION_SYS, calib_flash_cfg_tag_get()+1, pdata+CFG_TAG_LENGTH_MAX, &length);
        if(res < 0) return false;
    }
    else
    {
        length = CALIB_FLASH_LENGTH;
        res = cfg_get(CFG_SECTION_SYS, calib_flash_cfg_tag_get(), &data, &length);
        if(res < 0) return false;
    }

    crc16_flash = data.crc16;
    data.crc16 = 0;
    crc16_calc = co_crc16_ccitt(0, &data, CALIB_FLASH_LENGTH);
    if(crc16_flash != crc16_calc) return false;

    calib_env = data.save;

    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_REG_CTUNE_FDOUB, data.pmu.ct_fdoubler));
    HREGW(&HS_HIB->CONFIG,    MASK_1REG(HIB_CONFIG_RTUNE_RC32K, data.hib.rtune_rc32k));
    HREGW(&HS_HIB->CONFIG_1,  MASK_1REG(HIB_CONFIG_CTUNE_RC32K, data.hib.ctune_rc32k));

    calib_sys_restore();
    calib_rf_restore();

    return true;
}
#endif

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief calib_rf_restore()
 *
 * @return 
 **/
void calib_rf_restore(void)
{
    if(IS_FPGA())
        return;

    int i;

    CPM_ANA_CLK_ENABLE();
    uint32_t clk_ens_save = HS_DAIF->CLK_ENS;
    REGW1(&HS_DAIF->CLK_ENS, DAIF_PLL_AFC_CLK_EN_MASK | DAIF_PLL_AMP_CLK_EN_MASK | DAIF_RX_AGC_CLK_EN_MASK |
                DAIF_PLL_VTRACK_CLK_EN_MASK | DAIF_PLL_LUT_CLK_EN_MASK |
                DAIF_PLL_CLK_AFC_EN_MASK | DAIF_SDM_CLK_EN_MASK | DAIF_PLL_CLK_REF_EN_MASK);

    // vco afc
    for(i=0; i<80; i++)
    {
        REGWA(&HS_DAIF->PLL_LUT_DBG, MASK_3REG(DAIF_PLL_LUT_WR,1, DAIF_PLL_LUT_IDX,i, DAIF_PLL_LUT_DATA,calib_env.rf.vco_afc_lut[i]));
        while(HS_DAIF->PLL_LUT_DBG & DAIF_PLL_LUT_WR_MASK);
    }
    REGW(&HS_DAIF->VCO_CTRL0, MASK_2REG(DAIF_AFC_EN_MO,0, DAIF_AFC_EN_ME,1));

    // vco amp
    REGW(&HS_DAIF->PLL_CTRL2, MASK_2REG(DAIF_ICON_VCO_MO,calib_env.rf.vco_amp, DAIF_ICON_VCO_ME,1));

    // kdco 1M
    HS_DAIF->KDCO_LUT_1M_REG0 = calib_env.rf.kdco_1m[0];
    HS_DAIF->KDCO_LUT_1M_REG1 = calib_env.rf.kdco_1m[1];

    // kdco 2M
    HS_DAIF->KDCO_LUT_2M_REG0 = calib_env.rf.kdco_2m[0];
    HS_DAIF->KDCO_LUT_2M_REG1 = calib_env.rf.kdco_2m[1];

    // AGC restore
#ifdef CONFIG_HS6621
    REGW(&HS_DAIF->AGC_CFG2, MASK_2REG(DAIF_PIF_OS_FLAG,1, DAIF_PIF_OS_REG,calib_env.rf.agc_pif_os));
#else
    REGW(&HS_DAIF->AGC_CFG2, MASK_2REG(DAIF_PIF_OS_FLAG,1, DAIF_PIF_OS_REG,calib_env.rf.agc_pif_os));
    REGW(&HS_DAIF->AGC_CFG3, MASK_1REG(DAIF_PIF_OS_REG_2M, calib_env.rf.agc_pif_os_2m));
#endif

    // DCOC restore
    for(i=0; i<63; ++i)
        HS_DAIF->DCOC_LUT[i] = calib_env.rf.dcoc_lut[i];
    REGW(&HS_DAIF->DCOC_CFG,  MASK_4REG(DAIF_DIN_DCOC_I_ME,0, DAIF_DIN_DCOC_Q_ME,0, DAIF_DCOC_CALI,0, DAIF_EN_DCOC,1));
    REGW(&HS_DAIF->DCOC_CFG1, MASK_2REG(DAIF_OFFSET_I_ME,0, DAIF_OFFSET_Q_ME,0));

    // TXFE
    for(i=0; i<18; ++i)
        HS_DAIF->PA_GAIN_REG[i] = calib_env.rf.txfe_pa_gain[i];

    // restore PA
    HS_DAIF->PA_CNS = calib_env.rf.pa_cns;

    // modify default value
    calib_patch(calib_repair_env.temperature);

    // reenable vtrack
    REGW(&HS_DAIF->VCO_CTRL0, MASK_1REG(DAIF_VTRACK_EN,1));

    HS_DAIF->CLK_ENS = clk_ens_save;
    CPM_ANA_CLK_RESTORE();
}

/**
 * @brief  calib sys restore
 **/
void calib_sys_restore(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // rc
    HS_DAIF->TIA_LPF_CFG = calib_env.sys.rc_tia_lpf;

    CPM_ANA_CLK_RESTORE();
}

#endif //}BASE_COMPONENT_END

/** @} */

