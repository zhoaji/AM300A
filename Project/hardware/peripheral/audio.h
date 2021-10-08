/**
 * @file audio.h
 * @brief Audio
 * @date Fri, Aug  9, 2019 10:42:35 AM
 * @author liqiang
 *
 * @defgroup AUDIO AUDIO
 * @ingroup PERIPHERAL
 * @brief Audio
 * @details
 *
 * The HS6621 has an audio PGA and ADC inside. The structure is as shown below,
 * the PGA has a single input, which can be switched from GPIO[0] to GPIO[6].
 * The PGA gain varies from -14dB to 4dB in line-in mode, and from -3dB to 15dB
 * in mic-in mode. The PGA cascades ADC，dynamic range is 95.7dB,  SNDR is 75.8dB,
 * and SNR is 90.6dB.
 *
 * @{
 *
 * @example example_audio.c
 * This is an example of how to use the audio
 *
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef __cplusplus
extern "C"
{ /*}*/
#endif

/*********************************************************************
 * INCLUDES
 */


/*********************************************************************
 * MACROS
 */
/**
 * audio i2s tx/rx data line to i2s directly
 */
#define audio_i2s_data_lineto_i2s_directly()   \
    do { HS_AUDIO->IF_CTRL &= ~AU_I2S_CON_CTRL_MASK; } while (0)

/**
 * audio i2s tx/rx data line to i2s via gpio
 */
#define audio_i2s_data_lineto_i2s_via_gpio()   \
    do { HS_AUDIO->IF_CTRL |= AU_I2S_CON_CTRL_MASK; } while (0)

/*********************************************************************
 * TYPEDEFS
 */
/// au_pga_gain----CODEC_ANA_CTRL_1<AU_PGA_GAIN>, pga gain line-in mode
typedef enum
{
    AUDIO_LINE_IN_PGA_GAIN_N14DB = 0,
    AUDIO_LINE_IN_PGA_GAIN_N11DB    ,
    AUDIO_LINE_IN_PGA_GAIN_N8DB     ,
    AUDIO_LINE_IN_PGA_GAIN_N5DB     ,
    AUDIO_LINE_IN_PGA_GAIN_N2DB     ,
    AUDIO_LINE_IN_PGA_GAIN_1DB      ,
    AUDIO_LINE_IN_PGA_GAIN_4DB      ,
    AUDIO_LINE_IN_PGA_GAIN_7DB      ,
    AUDIO_LINE_IN_PGA_GAIN_10DB     ,
    AUDIO_LINE_IN_PGA_GAIN_13DB     ,
    AUDIO_LINE_IN_PGA_GAIN_16DB     ,
    AUDIO_LINE_IN_PGA_GAIN_19DB     ,
}audio_line_in_pga_gain_t;

/// au_pga_gain----CODEC_ANA_CTRL_1<AU_PGA_GAIN>, pga gain min-in mode
typedef enum
{
    AUDIO_MIC_IN_PGA_GAIN_N3DB = 0,
    AUDIO_MIC_IN_PGA_GAIN_N0DB    ,
    AUDIO_MIC_IN_PGA_GAIN_3DB     ,
    AUDIO_MIC_IN_PGA_GAIN_6DB     ,
    AUDIO_MIC_IN_PGA_GAIN_9DB     ,
    AUDIO_MIC_IN_PGA_GAIN_12DB    ,
    AUDIO_MIC_IN_PGA_GAIN_15DB    ,
    AUDIO_MIC_IN_PGA_GAIN_18DB    ,
    AUDIO_MIC_IN_PGA_GAIN_21DB    ,
    AUDIO_MIC_IN_PGA_GAIN_24DB    ,
    AUDIO_MIC_IN_PGA_GAIN_27DB    ,
    AUDIO_MIC_IN_PGA_GAIN_30DB    ,
}auido_mic_in_pga_gain_t;

/// au_sel_pgain----CODEC_ANA_CTRL_1<AU_SEL_PGAIN>, 
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
typedef enum
{
    AUDIO_SELECT_PAG_INPUT_VIP_MIC_P = 0,
    AUDIO_SELECT_PAG_INPUT_GPIO2        , // gpio2 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO3        , // gpio3 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO7        , // gpio7 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO8        , // gpio8 on chip
}audio_select_pga_input_t;
#else
/// use min or,the gpio1---gpio7
typedef enum
{
    AUDIO_SELECT_PAG_INPUT_VIP_MIC_P = 0,
    AUDIO_SELECT_PAG_INPUT_GPIO1        , // gpio1 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO2        , // gpio2 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO3        , // gpio3 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO4        , // gpio4 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO5        , // gpio5 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO6        , // gpio6 on chip
    AUDIO_SELECT_PAG_INPUT_GPIO7        , // gpio7 on chip
}audio_select_pga_input_t;
#endif
/// au_en_pga_singlein----CODEC_ANA_CTRL_1<AU_EN_PGA_SINGLEIN>, 
typedef enum
{
    /// use mic_1, differential
    AUDIO_EN_PGA_SINGLE_IN_VIN_MIC_1 = 0,
    /// use inner ref, single-ended
    AUDIO_EN_PGA_SINGLE_IN_INNER_REF    ,
}audio_en_pga_single_in_t;

/// auadc_gain----CODEC_ANA_CTRL_1<AU_AUADC_GAIN>, 
typedef enum
{
    AUDIO_ADC_GAIN_N6DB = 0,
    AUDIO_ADC_GAIN_0DB     ,
    AUDIO_ADC_GAIN_6DB     ,
    AUDIO_ADC_GAIN_12DB    ,
}audio_adc_gain_t;

/// Audio configuration
typedef struct
{
    /// typedef enum auido_mic_in_pga_gain_t 
    auido_mic_in_pga_gain_t pga_gain;
    /// typedef enum audio_select_pga_input_t
    audio_select_pga_input_t sel_pga_input;
    /// typedef enum audio_en_pga_single_in_t
    audio_en_pga_single_in_t sig_or_diff;
    /// typedef enum audio_adc_gain_t
    audio_adc_gain_t adc_gain;
}audio_config_t;

/// dmic clock out 
typedef enum
{
    AUDIO_DMIC_CLOCK_SEL_3M = 0,
    AUDIO_DMIC_CLOCK_SEL_2P4M  ,
}audio_dmic_clock_sel_t;

/// dmic clock out 
typedef enum
{
    AUDIO_DMIC_CLOCK_OUT = 0,
    AUDIO_DMIC_CLOCK_N_OUT  ,
}audio_dmic_clock_CTRL_t;
    

/// Audio dmic configuration
typedef struct
{
    /// dmic clock sel
    audio_dmic_clock_sel_t clk;
    /// dmic clock ctrl
    audio_dmic_clock_CTRL_t ctrl;    
}audio_dmic_config_t;

/// audio adc deci filter
typedef enum
{
    AUDIO_ADC_DECI_FILT_11  = 0,
    AUDIO_ADC_DECI_FILT_12     ,
    AUDIO_ADC_DECI_FILT_13     ,
    AUDIO_ADC_DECI_FILT_21     ,
    AUDIO_ADC_DECI_FILT_22     ,
    AUDIO_ADC_DECI_FILT_23     ,
    AUDIO_ADC_DECI_FILT_31     ,
    AUDIO_ADC_DECI_FILT_32     ,
    AUDIO_ADC_DECI_FILT_33     ,
    AUDIO_ADC_DECI_FILT_41     ,
    AUDIO_ADC_DECI_FILT_42     ,
    AUDIO_ADC_DECI_FILT_43     ,
    AUDIO_ADC_DECI_FILT_51     ,
    AUDIO_ADC_DECI_FILT_52     ,
    AUDIO_ADC_DECI_FILT_53     ,
}audio_adc_deci_filt_t;

/// audio adc iir filter
typedef enum
{
    AUDIO_ADC_IIR_FILT_11  = 0,
    AUDIO_ADC_IIR_FILT_12     ,
    AUDIO_ADC_IIR_FILT_13     ,
    AUDIO_ADC_IIR_FILT_21     ,
    AUDIO_ADC_IIR_FILT_22     ,
    AUDIO_ADC_IIR_FILT_23     ,
    AUDIO_ADC_IIR_FILT_31     ,
    AUDIO_ADC_IIR_FILT_32     ,
    AUDIO_ADC_IIR_FILT_33     ,
}audio_adc_iir_filt_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/**
 * @brief audio initialize
 *
 * @return None
 **/
void audio_init(void);

/**
 * @brief audio open clock for analog mic.
 *
 * @return None
 **/
void audio_open(void);

/**
 * @brief audio open for dmic.
 *
 * @return None
 **/
void audio_dmic_open(const audio_dmic_config_t* config);

/**
 * @brief Audio configuration
 *
 * @param[in] config audio config structure
 *
 * @return None
 **/
void audio_set_config(const audio_config_t *config);

/**
 * @brief audio close clock.
 *
 * @return None
 **/
void audio_close(void);

/**
 * @brief  audio set calibarate param
 *
 * @param[in] au_sel_bias  au sel bias
 **/
void audio_set_calibarate_param(uint8_t au_sel_bias);

/**
 * @brief Audio setting adc vol
 *
 * @param[in] val: audio vol control value
 *
 * @return None
 **/
void audio_set_adc_vol_ctrl(uint32_t val);

/**
 * @brief Audio setting deci filt
 *
 * @param[in] deci_filt audio adc deci filter
 * @param[in] val: audio vol control value
 *
 * @return None
 **/
void audio_set_deci_filt(audio_adc_deci_filt_t deci_filt, uint32_t val);

/**
 * @brief Audio setting iir filt
 *
 * @param[in] iir_filt: audio adc iir filter
 * @param[in] val audio vol control value
 *
 * @return None
 **/
void audio_set_iir_filt(audio_adc_iir_filt_t iir_filt, uint32_t val);

/**
 * @brief Audio setting debug bus
 *
 * @return None
 **/
void audio_set_debug_bus(void);

#ifdef __cplusplus
/*{*/ }
#endif

#endif

/** @} */

