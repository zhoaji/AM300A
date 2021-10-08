/**
 * @file adc.h
 * @brief ADC driver
 * @date Tue 16 May 2017 02:52:30 PM CST
 * @author liqiang
 *
 * @defgroup ADC ADC
 * @ingroup PERIPHERAL
 * @brief ADC driver
 * @details
 *
 * General Purpose (GP) ADC
 *
 * The HS6621 is equipped with a high-speed low power 12-bit general purpose Analog-to-Digital Converter
 * (GPADC). It can operate in unipolar (single ended) mode as well as in bipolar (differential) mode.
 * The ADC has its own voltage regulator (LDO) of 1.2 V, which represents the full scale reference voltage.
 *
 * Features:
 * - 12-bit dynamic ADC with 65 ns conversion time
 * - Maximum sampling rate 16M sample/s
 * - Ultra-low power (5 uA typical supply current at 100k sample/s)
 * - Single-ended as well as differential input with two input scales
 * - Eight single-ended or four differential external input channels
 * - Battery monitoring function
 * - Chopper function
 * - Offset and zero scale adjust
 * - Common-mode input level adjust
 *
 * @{
 *
 * @example example_adc.c
 * This is an example of how to use the adc
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"


/*********************************************************************
 * MACROS
 */

/// Q: fractional bits of fixed-point format.
#define ADC_Q       (12)

/// ADC sample data type.In fixed-point format of Q4.11.
typedef int16_t adc_sample_data_t;

/// ADC Channel type
typedef enum {
    ADC_CHANNEL_CHIP_TEMPERATURE   = 0, /* b0000 channel temperature. bit */
    ADC_CHANNEL_CHIP_BATTERY       ,    /* b0001 channel battery. */
    ADC_CHANNEL_CHIP_VCM           ,    /* b0010 channel vcm. */
    ADC_CHANNEL_EXTERN_CH0         ,    /* b0011 channel 0. */
    ADC_CHANNEL_EXTERN_CH1         ,    /* b0100 channel 1. */
    ADC_CHANNEL_EXTERN_CH2         ,    /* b0101 channel 2. */
    ADC_CHANNEL_EXTERN_CH3         ,    /* b0110 channel 3. */
    ADC_CHANNEL_EXTERN_CH4         ,    /* b0111 channel 4. */
    ADC_CHANNEL_EXTERN_CH5         ,    /* b1000 channel 5. */
    ADC_CHANNEL_EXTERN_CH6         ,    /* b1001 channel 6. */
    ADC_CHANNEL_EXTERN_CH7         ,    /* b1010 channel 7. */
    ADC_CHANNEL_VIP_CHG            ,    /* b1011 channel vip chg. */
    ADC_CHANNEL_VCC_VBAT_CHG       ,    /* b1100 channel vcc vbat chg. */

    ADC_CHANNEL_NUM
} adc_channel_t;

/// ADC Gtune_gp values
typedef enum {
    ADC_GTUNE_GP_0P5             = 0,
    ADC_GTUNE_GP_1                 ,
    ADC_GTUNE_GP_2                 ,
    ADC_GTUNE_GP_4                 ,

    ADC_GTUNE_GP_MAX
} adc_gtune_gp_t;

/// ADC Scal_gp value
typedef enum {
    ADC_SCAL_GP_DISABLE          = 0,
    ADC_SCAL_GP_ENABLE              ,
    ADC_SCAL_GP_MAX
} adc_scal_gp_t;

/// ADC PGA Gain
typedef enum {
    ADC_PGA_GAIN_0P125        = 0,  /* when measuring voltage, the range is 0-4.2v. */
    ADC_PGA_GAIN_0P25            ,  /* when measuring voltage, the range is 0-3.2v. */
    ADC_PGA_GAIN_0P5             ,  /* when measuring voltage, the range is 0-1.6v. */
    ADC_PGA_GAIN_1               ,  /* when measuring voltage, the range is 0-0.8v. */
    ADC_PGA_GAIN_2               ,  /* when measuring voltage, the range is 0-0.4v. */
    ADC_PGA_GAIN_4               ,  /* when measuring voltage, the range is 0-0.2v. */
    ADC_PGA_GAIN_MAX
} adc_pga_gain_t;

/// ADC internal VCM (Common Mode Voltage) selection.
typedef enum {
    ADC_VCM_SEL_000mV                 = 0, /* b0000. */
    ADC_VCM_SEL_125mV                 ,    /* b0001. */
    ADC_VCM_SEL_250mV                 ,    /* b0010. */
    ADC_VCM_SEL_375mV                 ,    /* b0011. */
    ADC_VCM_SEL_500mV                 ,    /* b0100. */
    ADC_VCM_SEL_625mV                 ,    /* b0101. */
    ADC_VCM_SEL_750mV                 ,    /* b0110. */
    ADC_VCM_SEL_875mV                 ,    /* b0111. */
    ADC_VCM_SEL_1000mV                ,    /* b1000. */
    ADC_VCM_SEL_1125mV                ,    /* b1001. */
    ADC_VCM_SEL_1250mV                ,    /* b1010. */
    ADC_VCM_SEL_1375mV                ,    /* b1011. */
    ADC_VCM_SEL_1500mV                ,    /* b1100. */
    ADC_VCM_SEL_1625mV                ,    /* b1101. */
    ADC_VCM_SEL_1750mV                ,    /* b1110. */
    ADC_VCM_SEL_1875mV                ,    /* b1111. */
    ADC_VCM_MAX
} adc_vcm_t;

/// ADC trigger selection.
typedef enum
{
    ADC_HW_TIMER0_0   = 0,
    ADC_HW_TIMER0_1      ,
    ADC_HW_TIMER0_2      ,
    ADC_HW_TIMER0_3      ,
    ADC_HW_TIMER1_0      ,
    ADC_HW_TIMER1_1      ,
    ADC_HW_TIMER1_2      ,
    ADC_HW_TIMER1_3      ,
    ADC_HW_TIMER2_0      ,
    ADC_HW_TIMER2_1      ,
    ADC_HW_TIMER2_2      ,
    ADC_HW_TIMER2_3      ,
    ADC_HW_TIMER0_EXT    ,
    ADC_HW_TIMER1_EXT    ,
    ADC_HW_TIMER2_EXT    ,
    ADC_HW_TIMER3_EXT    ,
}adc_hw_trigger_sel_t;

/// ADC Trigger Edge
typedef enum {
    ADC_TRIGGER_EDGE_POS                 = 0,
    ADC_TRIGGER_EDGE_NEG                 ,
    ADC_TRIGGER_EDGE_DUAL                ,
    ADC_TRIGGER_EDGE_NOTUSED             ,   /* Same as EDGE_POS  */
} adc_trigger_edge_t;
    
/// Trigger mode
typedef enum
{
    ADC_TRIGGER_MODDE_SW      = 0,
    ADC_TRIGGER_MODDE_HW_POS     ,
    ADC_TRIGGER_MODDE_HW_NEG     ,
    ADC_TRIGGER_MODDE_HW_BOTH    ,
}adc_trigger_mode_t;

/// ADC trigger resouce
typedef enum
{
    ADC_TRIG_RES_CHANNEL  = 0,
    ADC_TRIG_RES_SEQUENCE    ,
}adc_trigger_res_t;

/// adc convert count
typedef enum {
    ADC_TRIGGER_CONVERT_CONTINUOUS          = 0, /* repeat forever once triggered. */
    ADC_TRIGGER_CONVERT_ONESHOT                , /* one shot of group per event. */
    ADC_TRIGGER_CONVERT_BY_COUNT               , /* the count times when conver end,2<x<65535. */
} adc_convert_count_t;

/// adc scan dir
typedef enum
{
    ADC_SCAN_DIR_LSB = 0,
    ADC_SCAN_DIR_MSB    ,
}adc_scan_dir_t;

/// ADC sample frequency
typedef enum
{
    ADC_CLK_SEL_2MHZ  = 0,
    ADC_CLK_SEL_4MHZ  = 1,
    ADC_CLK_SEL_8MHZ  = 2,
    ADC_CLK_SEL_16MHZ = 3,
}adc_clk_sel_t;

/// ADC event type
typedef enum
{
    ADC_EVENT_CH_T_DONE     = 1<<0,
    ADC_EVENT_CH_V_DONE     = 1<<1,
    ADC_EVENT_VCM_DONE      = 1<<2,
    ADC_EVENT_CH_0_DONE     = 1<<3,
    ADC_EVENT_CH_1_DONE     = 1<<4,
    ADC_EVENT_CH_2_DONE     = 1<<5,
    ADC_EVENT_CH_3_DONE     = 1<<6,
    ADC_EVENT_CH_4_DONE     = 1<<7,
    ADC_EVENT_CH_5_DONE     = 1<<8,
    ADC_EVENT_CH_6_DONE     = 1<<9,
    ADC_EVENT_CH_7_DONE     = 1<<10,
    ADC_EVENT_CHG_REF_DONE  = 1<<11,
    ADC_EVENT_VBAT_CHG_DONE = 1<<12,
    ADC_EVENT_ONE_SEQ_DONE  = 1<<13,
    ADC_EVENT_ALL_SEQ_DONE  = 1<<14,
    ADC_EVENT_OVERRIDE      = 1<<15,
    ADC_EVENT_DMA_DONE      = 1<<16,
}adc_event_t;
/*********************************************************************
 * TYPEDEFS
 */
/// the param of temperature 
typedef struct
{
    /// the temperature offset 
    int32_t offset;
}adc_temp_calib_t;

///ADC calibration parameter table storing all the @p adc_cal_param_t parameters.
typedef struct {
    // single-ended
    // V	vos
    union {
        __IO uint32_t  cmpn_s_vos_g0_lut_reg0;
        struct {
            __IO uint32_t   s_vos_mod_g0_lut0   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g0_lut1   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g0_lut_reg1;
        struct {
            __IO uint32_t   s_vos_mod_g0_lut2   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g0_lut3   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g0_lut_reg2;
        struct {
            __IO uint32_t   s_vos_mod_g0_lut4   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g0_lut5   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g1_lut_reg0;
        struct {
            __IO uint32_t   s_vos_mod_g1_lut0   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g1_lut1   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g1_lut_reg1;
        struct {
            __IO uint32_t   s_vos_mod_g1_lut2   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g1_lut3   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g1_lut_reg2;
        struct {
            __IO uint32_t   s_vos_mod_g1_lut4   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g1_lut5   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g2_lut_reg0;
        struct {
            __IO uint32_t   s_vos_mod_g2_lut0   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g2_lut1   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g2_lut_reg1;
        struct {
            __IO uint32_t   s_vos_mod_g2_lut2   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g2_lut3   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vos_g2_lut_reg2;
        struct {
            __IO uint32_t   s_vos_mod_g2_lut4   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vos_mod_g2_lut5   : 12;
            __IO uint32_t                       :  4;
        };
    };

    // gain
    union {
        __IO uint32_t  cmpn_s_gain_g0_lut_reg0;
        struct {
            __IO uint32_t   s_gain_mod_g0_lut0   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g0_lut1   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g0_lut_reg1;
        struct {
            __IO uint32_t   s_gain_mod_g0_lut2   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g0_lut3   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g0_lut_reg2;
        struct {
            __IO uint32_t   s_gain_mod_g0_lut4   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g0_lut5   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g1_lut_reg0;
        struct {
            __IO uint32_t   s_gain_mod_g1_lut0   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g1_lut1   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g1_lut_reg1;
        struct {
            __IO uint32_t   s_gain_mod_g1_lut2   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g1_lut3   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g1_lut_reg2;
        struct {
            __IO uint32_t   s_gain_mod_g1_lut4   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g1_lut5   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g2_lut_reg0;
        struct {
            __IO uint32_t   s_gain_mod_g2_lut0   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g2_lut1   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g2_lut_reg1;
        struct {
            __IO uint32_t   s_gain_mod_g2_lut2   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g2_lut3   : 12;
            __IO uint32_t                        :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_gain_g2_lut_reg2;
        struct {
            __IO uint32_t   s_gain_mod_g2_lut4   : 12; 
            __IO uint32_t                        :  4;
            __IO uint32_t   s_gain_mod_g2_lut5   : 12;
            __IO uint32_t                        :  4;
        };
    };

    // vcm
    union {
        __IO uint32_t  cmpn_s_vcm_g0_lut_reg0;
        struct {
            __IO uint32_t   s_vcm_mod_g0_lut0   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g0_lut1   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g0_lut_reg1;
        struct {
            __IO uint32_t   s_vcm_mod_g0_lut2   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g0_lut3   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g0_lut_reg2;
        struct {
            __IO uint32_t   s_vcm_mod_g0_lut4   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g0_lut5   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g1_lut_reg0;
        struct {
            __IO uint32_t   s_vcm_mod_g1_lut0   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g1_lut1   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g1_lut_reg1;
        struct {
            __IO uint32_t   s_vcm_mod_g1_lut2   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g1_lut3   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g1_lut_reg2;
        struct {
            __IO uint32_t   s_vcm_mod_g1_lut4   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g1_lut5   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g2_lut_reg0;
        struct {
            __IO uint32_t   s_vcm_mod_g2_lut0   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g2_lut1   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g2_lut_reg1;
        struct {
            __IO uint32_t   s_vcm_mod_g2_lut2   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g2_lut3   : 12;
            __IO uint32_t                       :  4;
        };
    };
    union {
        __IO uint32_t  cmpn_s_vcm_g2_lut_reg2;
        struct {
            __IO uint32_t   s_vcm_mod_g2_lut4   : 12; 
            __IO uint32_t                       :  4;
            __IO uint32_t   s_vcm_mod_g2_lut5   : 12;
            __IO uint32_t                       :  4;
        };
    };

    // differential
    union {
        __IO uint32_t   cmpn_d_vos_lut_reg0;
        struct {
            __IO uint32_t   d_vos_mod_lut0   : 12;
            __IO uint32_t                    :  4;
            __IO uint32_t   d_vos_mod_lut1   : 12;
            __IO uint32_t                    :  4;
        };
    };
    union {
        __IO uint32_t   cmpn_d_vos_lut_reg1;
        struct {
            __IO uint32_t   d_vos_mod_lut2   : 12;
            __IO uint32_t                    :  4;
            __IO uint32_t   d_vos_mod_lut3   : 12;
            __IO uint32_t                    :  4;
        };
    };
    union {
        __IO uint32_t   cmpn_d_vos_lut_reg2;
        struct {
            __IO uint32_t   d_vos_mod_lut4   : 12;
            __IO uint32_t                    :  4;
            __IO uint32_t   d_vos_mod_lut5   : 12;
            __IO uint32_t                    :  4;
        };
    };
    union {
        __IO uint32_t   cmpn_d_gain_lut_reg0;
        struct {
            __IO uint32_t   d_gain_mod_lut0  : 12;
            __IO uint32_t                    :  4;
            __IO uint32_t   d_gain_mod_lut1  : 12;
            __IO uint32_t                    :  4;
        };
    };
    union {
        __IO uint32_t   cmpn_d_gain_lut_reg1;
        struct {
            __IO uint32_t   d_gain_mod_lut2  : 12;
            __IO uint32_t                    :  4;
            __IO uint32_t   d_gain_mod_lut3  : 12;
            __IO uint32_t                    :  4;
        };
    };
    union {
        __IO uint32_t   cmpn_d_gain_lut_reg2;
        struct {
            __IO uint32_t   d_gain_mod_lut4  : 12;
            __IO uint32_t                    :  4;
            __IO uint32_t   d_gain_mod_lut5  : 12;
            __IO uint32_t                    :  4;
        };
    };
    /// temperature calibrate offset
    adc_temp_calib_t temp_calib_offset;
} adc_cal_table_t;

/// ADC calibration parameters specific to PGA gain used to compensate converted data.
typedef struct {
    /// @brief voltage-offset modifier in V, in fixed-point format of Q0.11.
    int16_t vos_mod;  
    /// @brief PGA (Pre-Gain-Amplifier) gain modifier, in fixed-point format of UQ1.11.
    int16_t gain_mod; 
    /// @brief VCM modifier, in fixed-point format of UQ1.11. 
    int16_t vcm_mod;  
} adc_cal_param_t;

/// ADC event callback
typedef void (*adc_callback_t)(uint32_t event, float adc_data);

/// ADC dma configuration
typedef struct
{
    /// DMA fifo enable
    bool           use_fifo;
    /// Fifo buffer
    void          *buffer;
    /// Fifo buffer length
    uint32_t       buffer_len;
    /// DMA block link list item
    dma_llip_t    *block_llip;
    /// DMA block link list item number
    uint32_t       block_num;
    /// Event callback
    dma_callback_t callback;
}adc_dma_config_t;

/// ADC configuration
typedef struct
{
    /// Trigger mode: software/hardware/continuous
    adc_trigger_mode_t   trigger_mode;
    /// trigger res
    adc_trigger_res_t    trigger_res;
    /// In hardware triiger mode, select the trigger hardware
    adc_hw_trigger_sel_t hw_trigger_sel;
    ///  conver count
    uint16_t             trigger_count;
}adc_config_t;

/// adc channel config
typedef struct
{
    /// ADC channel(s)
    adc_channel_t        inp_gp;
    /// ADC channel(s)
    adc_channel_t        inn_gp;
    /// adc pga gain
    adc_gtune_gp_t       gtune_gp;
    /// scal gp
    adc_scal_gp_t        scal_gp;
    /// vcm
    adc_vcm_t            vcm_gp;
    /// Sample frequency
    adc_clk_sel_t        clk_sel;
    /// Event callback
    adc_callback_t       callback;

    /// sample count
    //uint16_t             count;
}adc_channel_config_t;
/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/**
 * @brief ADC initialize
 *
 * @return None
 **/
void adc_init(void);

/**
 * @brief ADC configuration
 *
 * @param[in] config adc config structure
 *
 * @return None
 **/
void adc_config(const adc_config_t *config);

/**
 * @brief ADC DMA configuration
 *
 * @param[in] dma  NULL: ADC will allocate a new dma; Not NULL: use it as adc dma
 * @param[in] config  ADC dma configuration structure
 * @param[out] src_addr  the adc output address
 *
 * @retval NULL No DMA valid or error happen
 * @retval object Current used DMA object
 **/
HS_DMA_CH_Type *adc_dma_config(HS_DMA_CH_Type *dma, const adc_dma_config_t *config, uint32_t *src_addr);

/**
 * @brief ADC setting channel confing
 * @param[in] channel  ADC channel
 *
 * @return success or fail
 **/
uint8_t adc_add_channel(adc_channel_config_t *channel);

/**
 * @brief ADC del channel confing
 * @param[in] inp_gp  ADC channel
 *
 * @return success or fail
 **/
uint8_t adc_del_channel(adc_channel_t inp_gp);

/**
 * @brief ADC open pum clock and irq
 *
 * @return success or fail
 **/
uint8_t adc_start(void);

/**
 * @brief ADC close pum clock and irq
 *
 * @return success or fail
 **/
uint8_t adc_stop(void);

/**
 * @brief ADC stop convert
 *
 * @return None
 **/
void adc_start_convert(void);

/**
 * @brief ADC start convert
 *
 * @return None
 **/
void adc_stop_convert(void);

/**
 * @brief ADC calib value set
 *
 * @param[in] cal_table  calib adc value
 *
 * @return None
 **/
void adc_set_calibarate_param(adc_cal_table_t *cal_table);

/**
 * @brief adc read current chip temperature (+-10C error)
 *
 * @param[in] cb call the callback when adc sample done
 *
 * @return temperature
 **/
int16_t adc_temperature_read(adc_callback_t cb);

/**
 * @brief adc read voltage with channel
 *
 * @param[in] cb call the callback when adc sample done
 *
 * @return voltage (mV)
 **/
int16_t adc_battery_voltage_read(adc_callback_t cb);

/**
 * @brief adc read voltage with channel, use single_ended
 *
 * @param[in] ch_p adc channel.
 * @param[in] gain adc gain. or the measure range(V)
 * @param[in] cb call the callback when adc sample done
 * @param[in] sample_num measure count
 *
 * @return voltage (mV)
 **/
int16_t adc_battery_voltage_read_by_single_pin(adc_channel_t ch_p, adc_pga_gain_t gain, adc_callback_t cb, int sample_num);

/**
 * @brief adc read voltage with channel, use differential mode
 *
 * @param[in] ch_p adc channel, adc_inp.
 * @param[in] ch_n adc channel. adc_nnp
 * @param[in] gain adc gain. or the measure range(V)
 * @param[in] cb call the callback when adc sample done
 * @param[in] sample_num measure count
 *
 * @return voltage (mV)
 **/
int16_t adc_battery_voltage_read_by_dif_pin(adc_channel_t ch_p, adc_channel_t ch_n, adc_pga_gain_t gain, adc_callback_t cb, int sample_num);

/**
 *  @brief Calibrate ADC and initialize ADC compensation parameters.
 *         Used during CP/FT.
 *
 *  @note  Before call this function, it should have prepared the
 *         specified voltages(4.2v), and the pad_gpio<3> grounding.
 *
 *         When all the calibration parameters are calibrated, burn the table
 *         to flash memory at address @p ADC_CAL_TABLE.
 *         @p ADC_CAL_TABLE should be well defined before FT test.
 *
 *         0. ADC_CAL_TABLE should be defined pointing to an @p adc_cal_table_t.
 *         1. Calibrate all parameters in @p adc_cal_table_t.
 *         2. Store the @p adc_cal_table_t object to @p ADC_CAL_TABLE in Flash.
 *
 *  @param[in] cal_table   Pointer to @p adc_cal_table_t object where the
 *                         calibration parameters are to be returned.
 * @return None
 */
void adc_calibrate(adc_cal_table_t *cal_table);

/**
 * @brief ADC check status, start convert or stop
 *
 * @return bool
 **/
bool adc_is_running(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */


