/**
 * @file audio.c
 * @brief 
 * @date Fri, Aug  9, 2019 10:45:08 AM
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
#include "audio.h"


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint8_t au_sel_bias;
}audio_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static audio_env_t audio_env = {
    .au_sel_bias = 5,
};

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
 * @brief audio initialize
 *
 * @return None
 **/
void audio_init(void)
{
}

/**
 * @brief audio open clock.
 *
 * @return None
 **/
void audio_open(void)
{
#ifndef CONFIG_HS6621
    REGW1(&HS_PMU->ANA_PD, PMU_ANA_REG_PD_BUF_DIG_MASK);
    HS_PMU_UPD_RDY();

    REGW0(&HS_PMU->ANA_PD, PMU_ANA_PD_LDO_ANADIG_MASK);
    HS_PMU_UPD_RDY();

    CPM_ANA_AHB_CLK_ENABLE();
    REGW1(&HS_DAIF->CLK_CFG, DAIF_EN_CKO16M_XTAL_AUDIO_MASK);
    CPM_ANA_CLK_RESTORE();
    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_SEL_XTAL32M_GM, 15));
#endif
    /// open audio clock
    REGW(&HS_PSO->AUDIO_CFG, MASK_3REG(CPM_AUDIO_GATE_EN, 0, CPM_AUDIO_12M_GATE_EN, 0, CPM_AUDIO_SOFT_RESET, 0));
    HS_PSO_UPD_RDY();

    /// pga, adc, ldo, bias, 1: control by reg. 0: control by fsm 
    REGW(&HS_AUDIO->CODEC_ANA_PWR_1, MASK_2REG(AU_AUDIO_ADC_CTRL, 1, AU_AUDIO_LDO_CTRL, 1));
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AU_PGA, 0));

    /// 1) open ldo, bias
    /// t0 open ldo_28
#ifndef CONFIG_HS6621
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_2REG(AU_PD_AUDIO_IREF, 0, AU_SEL_INNERIREF, 1));
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_1REG(AU_EN_VREG_IGEN, 0));//2V
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_2REG(AU_EN_VREG_2P8LDO, 0, AU_EN_VREG_1P2LDO, 0));//2V
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_1REG(AU_SEL_INNERIREF_AUVREF, 0));
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_1REG(AU_SEL_BIAS, audio_env.au_sel_bias));
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_2REG(AU_CTRL_AU_AULDO28, 3, AU_CTRL_AU_AULDO12,3));
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_1REG(AU_AULDO28MOD, 0));//0:2.0V, 1:3.3V
#endif
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_1REG(AU_PD_AU_AULDO28, 0));
    /// <<HS6621 audio digital interface guide>>, need 1ms delay
    co_delay_us(10);
    /// t1 open auref, open auref bypass
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_2REG(AU_PD_AUREF, 0, AU_EN_REF_BYP, 1));

    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AU_BIAS_GEN, 0));
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_EN_PGA_SINGLEIN,  0));  
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AU_PGAVREF, 1));
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PGA_GAIN, 1));
    co_delay_ms(100);

#ifdef CONFIG_HS6621
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_2REG(AU_PA_AU_AULDO12, 0, AU_PD_AU_DLDO12, 0));
#else
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_2, MASK_1REG(AU_PA_AU_AULDO12,  0));
#endif

    co_delay_us(10);
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AU_PGA, 1));
    co_delay_us(10);
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AU_PGA, 0));
    co_delay_us(10);

    /// t4 open clk
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AU_CLK, 0));
    /// t4 delay 1ms
    co_delay_ms(50);
    /// t5 close auref bypass
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_EN_REF_BYP, 0));
    /// t5 delay 1ms
    co_delay_us(10);

    /// 2) open pga adc
    /// t2 open adc bias
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AUADC_BIAS, 0));
    /// t2 delay 1ms
    co_delay_us(10);
    /// t3 open adc core
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_PD_AUADC_CORE, 0));
    /// t3 delay
    co_delay_us(10);
    /// t4 reset adc
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_RST_ADC, 1));
    /// t4 delay
    co_delay_us(10);
    /// t5 set reset_adc
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_1REG(AU_RST_ADC, 0));

    REGW(&HS_AUDIO->ADC_CTRL, MASK_1REG(AU_ADC_ANTI_CLIP, 1));

#ifdef CONFIG_HS6621
    CPM_ANA_AHB_CLK_ENABLE();
    //REGW(&HS_DAIF->ANA_TST_CTRL, MASK_2REG(DAIF_ANA_TST_EN, 1, DAIF_ANA_TST_DC, 0xF)); // test code
    REGW1(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_SEL_AUDIODIV_MASK);//set audio clock
    CPM_ANA_CLK_RESTORE();
#endif
}

/**
 * @brief audio clock clock.
 *
 * @return None
 **/
void audio_close(void)
{
    //CPM_ANA_AHB_CLK_ENABLE();
    //REGW(&HS_DAIF->ANA_TST_CTRL, MASK_2REG(DAIF_ANA_TST_EN, 0, DAIF_ANA_TST_DC, 0));
    //CPM_ANA_CLK_RESTORE();

    REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_2REG(AU_DMIC_CLK_CTRL, 0, AU_DMIC_CLK_SEL, 0));

    /// disable digital mic
    REGW(&HS_AUDIO->ADC_CTRL, MASK_1REG(AU_DMIC_EN, 0));
    REGW(&HS_AUDIO->ADC_CTRL, MASK_1REG(AU_ADC_ANTI_CLIP, 0));
    
    /// reset audio
    REGW(&HS_PSO->AUDIO_CFG, MASK_1REG(CPM_AUDIO_SOFT_RESET, 1));
    HS_PSO_UPD_RDY();
    /// close audio clock
    REGW(&HS_PSO->AUDIO_CFG, MASK_2REG(CPM_AUDIO_GATE_EN, 1, CPM_AUDIO_12M_GATE_EN, 1));
    HS_PSO_UPD_RDY();

#ifdef CONFIG_HS6621
    CPM_ANA_AHB_CLK_ENABLE();
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_SEL_AUDIODIV_MASK);//set audio clock
    CPM_ANA_CLK_RESTORE();
#else
    CPM_ANA_AHB_CLK_ENABLE();
    REGW0(&HS_DAIF->CLK_CFG, DAIF_EN_CKO16M_XTAL_AUDIO_MASK);
    CPM_ANA_CLK_RESTORE();

    REGW1(&HS_PMU->ANA_PD, PMU_ANA_PD_LDO_ANADIG_MASK);
    HS_PMU_UPD_RDY();
    REGW0(&HS_PMU->ANA_PD, PMU_ANA_REG_PD_BUF_DIG_MASK);
    HS_PMU_UPD_RDY();
#endif
}

/**
 * @brief Audio configuration
 *
 * @param[in] config audio config structure
 *
 * @return None
 **/
void audio_set_config(const audio_config_t *config)
{
    REGW(&HS_AUDIO->CODEC_ANA_CTRL_1, MASK_4REG(AU_PGA_GAIN, config->pga_gain,
                                                AU_SEL_PGAIN, config->sel_pga_input,
                                                AU_EN_PGA_SINGLEIN, config->sig_or_diff,
                                                AU_AUADC_GAIN, config->adc_gain));
    
    audio_set_adc_vol_ctrl(0x01608674);
    //audio_set_adc_vol_ctrl(0x01646416);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_11, 0xcbed6256);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_12, 0xe7b7117f);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_13, 0x117f);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_21, 0xd59c5ea3);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_22, 0xcbe12186);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_23, 0x2186);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_31, 0xc56b6500);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_32, 0xf3d60cb5);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_33, 0x0cb5);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_41, 0xde5f5b59);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_42, 0xe567118f);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_43, 0x118f);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_51, 0xc1866709);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_52, 0x0b1e0cb5);
    audio_set_deci_filt(AUDIO_ADC_DECI_FILT_53, 0x0cb5);
}

/**
 * @brief  audio set calibarate param
 *
 * @param[in] au_sel_bias  au sel bias
 **/
void audio_set_calibarate_param(uint8_t au_sel_bias)
{
    audio_env.au_sel_bias = au_sel_bias;
}

/**
 * @brief Audio setting adc vol
 *
 * @param[in] audio vol control value
 *
 * @return None
 **/
void audio_set_adc_vol_ctrl(uint32_t val)
{
    REGW(&HS_AUDIO->ADC_CTRL, MASK_1REG(AU_ADC_ANTI_CLIP, 0));
    REGW(&HS_AUDIO->ADC_VOL_CTRL, 0xFFFFFFFF, val);
    
    if(REGR(&HS_AUDIO->ADC_CTRL, MASK_POS(AU_ADC_EN)))
    {
        // must delay 100us and enable au_adc_en, for modify AU_ADC_ANTI_CLIP
        co_delay_us(100); 
        REGW(&HS_AUDIO->ADC_CTRL, MASK_1REG(AU_ADC_ANTI_CLIP, 1));
    }
}

/**
 * @brief Audio setting deci filt
 *
 * @param[in] audio vol control value
 *
 * @return None
 **/
void audio_set_deci_filt(audio_adc_deci_filt_t deci_filt, uint32_t val)
{
    volatile uint32_t *p_adc_deci_filt_reg = &HS_AUDIO->ADC_DECI_FILT_11 + deci_filt;
    *p_adc_deci_filt_reg = val;
}

/**
 * @brief Audio setting iir filt
 *
 * @param[in] audio vol control value
 *
 * @return None
 **/
void audio_set_iir_filt(audio_adc_iir_filt_t iir_filt, uint32_t val)
{
    volatile uint32_t *p_adc_iir_filt_reg = &HS_AUDIO->ADC_IIR_FILT_11 + iir_filt;
    *p_adc_iir_filt_reg = val;
}

/**
 * @brief audio open for dmic.
 *
 * @return None
 **/
void audio_dmic_open(const audio_dmic_config_t* config)
{
    /// open audio clock
    REGW(&HS_PSO->AUDIO_CFG, MASK_3REG(CPM_AUDIO_GATE_EN, 0, CPM_AUDIO_12M_GATE_EN, 0, CPM_AUDIO_SOFT_RESET, 0));
    HS_PSO_UPD_RDY();

    /// select digital mic 
    REGW(&HS_AUDIO->ADC_CTRL, MASK_1REG(AU_DMIC_EN, 1));
    /// set clock
    REGW(&HS_AUDIO->CODEC_CLK_CTRL_1, MASK_2REG(AU_DMIC_CLK_CTRL, config->ctrl, AU_DMIC_CLK_SEL, config->clk));
}

void audio_set_debug_bus(void)
{
    HS_SYS->MON = 0x0801;

    pinmux_config(15, PINMUX_DBG_MODE_CFG);
    pinmux_config(16, PINMUX_DBG_MODE_CFG);
    pinmux_config(17, PINMUX_DBG_MODE_CFG);
    pinmux_config(22, PINMUX_DBG_MODE_CFG);
    pmu_pin_driven_current_set(BITMASK(15), PMU_PIN_DRIVEN_CURRENT_2MA);
    pmu_pin_driven_current_set(BITMASK(16), PMU_PIN_DRIVEN_CURRENT_2MA);
    pmu_pin_driven_current_set(BITMASK(17), PMU_PIN_DRIVEN_CURRENT_2MA);
    pmu_pin_driven_current_set(BITMASK(22), PMU_PIN_DRIVEN_CURRENT_2MA);
}

/** @} */

