/**
 * @file adc.c
 * @brief 
 * @date Tue 16 May 2017 02:52:22 PM CST
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
//#include "co.h"
#include "adc.h"
#include "math.h"

/*********************************************************************
 * MACROS
 */
// enable log
//#define DEBUG_ADC_LOG 
#ifdef DEBUG_ADC_LOG
#include "co_debug.h"
#define adc_log(format, ...) log_debug("[ADC] "format, ## __VA_ARGS__)
#else
#define adc_log(format, ...)
#endif

/// adc calibrate sample count
#define ADC_CAL_MEASURE_COUNT          10
/// adc temperature sample count
#define ADC_TEMPERATEURE_SAMPLE_COUNT  2
/// adc battery sample count
#define ADC_BATTERY_SAMPLE_COUNT       5

/// Driver state machine possible states.
typedef enum {
    ADC_STATE_INIT     = 0,                             /**< Not initialized.           */
    ADC_STATE_CALIBRATE   ,                             /**< calibrating.               */
    ADC_STATE_READY       ,                             /**< Ready.                     */
    ADC_STATE_START       ,                             /**< Start.                     */
    ADC_STATE_ACTIVE      ,                             /**< Converting.                */
    ADC_STATE_COMPLETE    ,                             /**< Conversion complete.       */
    ADC_STATE_ERROR       ,                             /**< Conversion complete.       */
    ADC_STATE_STOP        ,                             /**< Stopped.                   */
} adc_state_t;

/// adc handle data type.
typedef enum {
    ADC_DATA_HANDLE_BLOCK     = 0x01,                      /**< block handle data.           */
    ADC_DATA_HANDLE_IRQ       = 0x02,                      /**< use irq handle data.         */
    ADC_DATA_HANDLE_DMA       = 0x04,                      /**< use dma handle data.         */
} adc_data_handle_type_t;

#define CALIB_PIN_INPUT_VOL          (1.00)

#ifdef CONFIG_HS6621
#define ADC_REG_STEP                 (0x40)
#else
#define ADC_REG_STEP                 (0x4)
#endif

/*********************************************************************
 * TYPEDEFS
 */

/// adc channel env type
typedef struct
{
    int32_t          data;
    uint16_t         count;
    uint16_t         numbers;
    adc_callback_t   callback;
    adc_channel_t    inp_gp;
    uint8_t          use;
}adc_channel_env_t;

/// adv evn type
typedef struct
{
    //adc_temp_calib_t temp_calib;
    adc_state_t      state;
    //uint8_t          ch_numbers;
    //adc_channel_env_t channel[ADC_CHANNEL_NUM];
    adc_callback_t   callback[ADC_CHANNEL_NUM];
    uint8_t          handle_type; // 0: block handle; 1 irq handle; 2 dma handle
    adc_cal_table_t  cal_table;
    uint8_t          is_use; // the cal_table by setting or not
 }adc_env_t;

/// adc gtune and scal for PGA gain.
typedef struct
{
    adc_gtune_gp_t   gtune_gp;
    adc_scal_gp_t    scal_gp;
}adc_gtune_scal_t;
/*********************************************************************
 * CONSTANTS
 */

static const uint32_t s_adc_cal_table_default[] =	{
    0x00000000, /* cmpn_s_vos_g0_lut_reg0  */
    0x00000000, /* cmpn_s_vos_g0_lut_reg1  */
    0x00000000, /* cmpn_s_vos_g0_lut_reg2  */
    0x00000000, /* cmpn_s_vos_g1_lut_reg0  */
    0x00000000, /* cmpn_s_vos_g1_lut_reg1  */
    0x00000000, /* cmpn_s_vos_g1_lut_reg2  */
    0x00000000, /* cmpn_s_vos_g2_lut_reg0  */
    0x00000000, /* cmpn_s_vos_g2_lut_reg1  */
    0x00000000, /* cmpn_s_vos_g2_lut_reg2  */
    0x08000800, /* cmpn_s_gain_g0_lut_reg0 */
    0x08000800, /* cmpn_s_gain_g0_lut_reg1 */
    0x08000800, /* cmpn_s_gain_g0_lut_reg2 */
    0x08000800, /* cmpn_s_gain_g1_lut_reg0 */
    0x08000800, /* cmpn_s_gain_g1_lut_reg1 */
    0x08000800, /* cmpn_s_gain_g1_lut_reg2 */
    0x08000800, /* cmpn_s_gain_g2_lut_reg0 */
    0x08000800, /* cmpn_s_gain_g2_lut_reg1 */
    0x08000800, /* cmpn_s_gain_g2_lut_reg2 */
    0x0A000A00, /* cmpn_s_vcm_g0_lut_reg0  */
    0x0A000A00, /* cmpn_s_vcm_g0_lut_reg1  */
    0x0A000A00, /* cmpn_s_vcm_g0_lut_reg2  */
    0x0A000A00, /* cmpn_s_vcm_g1_lut_reg0  */
    0x0A000A00, /* cmpn_s_vcm_g1_lut_reg1  */
    0x0A000A00, /* cmpn_s_vcm_g1_lut_reg2  */
    0x0A000A00, /* cmpn_s_vcm_g2_lut_reg0  */
    0x0A000A00, /* cmpn_s_vcm_g2_lut_reg1  */
    0x0A000A00, /* cmpn_s_vcm_g2_lut_reg2  */
    0x00000000, /* cmpn_d_vos_lut_reg0  */
    0x00000000, /* cmpn_d_vos_lut_reg1  */
    0x00000000, /* cmpn_d_vos_lut_reg2  */
    0x08000800, /* cmpn_d_gain_lut_reg0 */
    0x08000800, /* cmpn_d_gain_lut_reg0 */
    0x08000800, /* cmpn_d_gain_lut_reg1 */
    0x00000572, /* temp_calib_offset */
};

/// the gain,use gtune and scal
static const adc_gtune_scal_t s_adc_gain_table[] = {
{ADC_GTUNE_GP_0P5, ADC_SCAL_GP_ENABLE}, // ADC_PGA_GAIN_0P125
{ADC_GTUNE_GP_1,   ADC_SCAL_GP_ENABLE}, // ADC_PGA_GAIN_0P25
{ADC_GTUNE_GP_0P5, ADC_SCAL_GP_DISABLE}, // ADC_PGA_GAIN_0P5
{ADC_GTUNE_GP_1,   ADC_SCAL_GP_DISABLE}, // ADC_PGA_GAIN_1
{ADC_GTUNE_GP_2,   ADC_SCAL_GP_DISABLE}, // ADC_PGA_GAIN_2
{ADC_GTUNE_GP_4,   ADC_SCAL_GP_DISABLE}, // ADC_PGA_GAIN_4
};

/*********************************************************************
 * LOCAL VARIABLES
 */

/// the adc evn variables
static adc_env_t adc_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */
#if 0
static void adc_open_debug_bus(void)
{
    HS_SYS->MON = 0x0100e;
    HS_DAIF->DBG_REG = 0x1E;

    // 24 is clock.. 15--23 data
    //pinmux_config(24, PINMUX_DBG_MODE_CFG);

    log_debug("MON=%08x, DBG_REG=%08x\n", HS_SYS->MON, HS_DAIF->DBG_REG );
}

static void adc_print_debug_bus(void)
{
    // 18bit: clock, 17-9bit: data
    log_debug("DBG_REG=%08x\n",HS_DAIF->DBG_REG );
}
#endif
int16_t expand_integer_16(uint16_t data, uint8_t data_width)
{
    uint16_t data_mask = (1u << data_width) - 1;
    uint16_t signed_mask = 1u << (data_width - 1);
    uint16_t expand_mask = ((1u << (16 - data_width)) - 1) << data_width;

    data &= data_mask;

    if(!(data & signed_mask))
        return (int16_t)data;

    return (int16_t)(data|expand_mask);
}

static int32_t expand_integer_32(uint32_t data, uint8_t data_width)
{
    uint32_t data_mask = (1u << data_width) - 1;
    uint32_t signed_mask = 1u << (data_width - 1);
    uint32_t expand_mask = ((1u << (32 - data_width)) - 1) << data_width;

    data &= data_mask;

    if(!(data & signed_mask))
        return (int32_t)data;

    return (int32_t)(data|expand_mask);
}

uint32_t adc_channel_read_data(adc_channel_t channel)
{
    // only convert low 16bit
    switch(channel)
    {
        case ADC_CHANNEL_CHIP_TEMPERATURE:     return (uint32_t)HS_GPADC->CH_0_DATA;
        case ADC_CHANNEL_CHIP_BATTERY:         return (uint32_t)HS_GPADC->CH_1_DATA;
        case ADC_CHANNEL_CHIP_VCM:             return (uint32_t)HS_GPADC->CH_2_DATA;
        case ADC_CHANNEL_EXTERN_CH0:           return (uint32_t)HS_GPADC->CH_3_DATA;
        case ADC_CHANNEL_EXTERN_CH1:           return (uint32_t)HS_GPADC->CH_4_DATA;
        case ADC_CHANNEL_EXTERN_CH2:           return (uint32_t)HS_GPADC->CH_5_DATA;
        case ADC_CHANNEL_EXTERN_CH3:           return (uint32_t)HS_GPADC->CH_6_DATA;
        case ADC_CHANNEL_EXTERN_CH4:           return (uint32_t)HS_GPADC->CH_7_DATA;
        case ADC_CHANNEL_EXTERN_CH5:           return (uint32_t)HS_GPADC->CH_8_DATA;
        case ADC_CHANNEL_EXTERN_CH6:           return (uint32_t)HS_GPADC->CH_9_DATA;
        case ADC_CHANNEL_EXTERN_CH7:           return (uint32_t)HS_GPADC->CH_A_DATA;
        case ADC_CHANNEL_VIP_CHG:              return (uint32_t)HS_GPADC->CH_B_DATA;
        case ADC_CHANNEL_VCC_VBAT_CHG:         return (uint32_t)HS_GPADC->CH_C_DATA;
        default:        return 0;
    }
}

#if 0
/**
 * @brief Get calibration parameters from calibration table.
 *
 * @note  The calibration table is stored at a fixed position in Flash during
 *        FT test.
 *
 * @param param             Pointer to where the returned parameters is to be
 *                          stored.
 * @param table             Pointer to the calibration table.
 * @param is_differential   The parameters to get is for differential or not.
 * @param gain              The PGA gain corresponding to the parameters to be
 *                          returned.
 */
static void adc_lld_get_cal_param_from_table(adc_cal_param_t *param,
        adc_cal_table_t *table,
        bool is_differential,
        adc_pga_gain_t gain) {
    switch (gain) {
        case ADC_PGA_GAIN_0P125:
            param->vos_mod  = is_differential ? table->d_vos_mod_lut4  : table->s_vos_mod_g0_lut4;
            param->gain_mod = is_differential ? table->d_gain_mod_lut4 : table->s_gain_mod_g0_lut4;
            param->vcm_mod  = is_differential ? 0x0800                 : table->s_vcm_mod_g0_lut4;
            break;
        case ADC_PGA_GAIN_0P25:
            param->vos_mod  = is_differential ? table->d_vos_mod_lut5  : table->s_vos_mod_g0_lut5;
            param->gain_mod = is_differential ? table->d_gain_mod_lut5 : table->s_gain_mod_g0_lut5;
            param->vcm_mod  = is_differential ? 0x0800                 : table->s_vcm_mod_g0_lut5;
            break;
        case ADC_PGA_GAIN_0P5:
            param->vos_mod  = is_differential ? table->d_vos_mod_lut0  : table->s_vos_mod_g0_lut0;
            param->gain_mod = is_differential ? table->d_gain_mod_lut0 : table->s_gain_mod_g0_lut0;
            param->vcm_mod  = is_differential ? 0x0800                 : table->s_vcm_mod_g0_lut0;
            break;
        case ADC_PGA_GAIN_1:
            param->vos_mod  = is_differential ? table->d_vos_mod_lut1  : table->s_vos_mod_g0_lut1;
            param->gain_mod = is_differential ? table->d_gain_mod_lut1 : table->s_gain_mod_g0_lut1;
            param->vcm_mod  = is_differential ? 0x0800                 : table->s_vcm_mod_g0_lut1;
            break;
        case ADC_PGA_GAIN_2:
            param->vos_mod  = is_differential ? table->d_vos_mod_lut2  : table->s_vos_mod_g0_lut2;
            param->gain_mod = is_differential ? table->d_gain_mod_lut2 : table->s_gain_mod_g0_lut2;
            param->vcm_mod  = is_differential ? 0x0800                 : table->s_vcm_mod_g0_lut2;
            break;
        case ADC_PGA_GAIN_4:
            param->vos_mod  = is_differential ? table->d_vos_mod_lut3  : table->s_vos_mod_g0_lut3;
            param->gain_mod = is_differential ? table->d_gain_mod_lut3 : table->s_gain_mod_g0_lut3;
            param->vcm_mod  = is_differential ? 0x0800                 : table->s_vcm_mod_g0_lut3;
            break;
        default:
            param->vos_mod  = 0x0000;
            param->gain_mod = 0x0800;
            param->vcm_mod  = 0x0800;
            break;
    }
}

static bool adc_is_cal_param_valid(adc_cal_param_t *param) {

    uint16_t vos_mod = param->vos_mod;
    uint16_t gain_mod = param->gain_mod;
    uint16_t vcm_mod = param->vcm_mod;

    vos_mod &= 0xF800;
    vos_mod >>= 11;
    if (!(vos_mod == 0x0000 || vos_mod == 0x001F)) // is upper 5 bits (sign bits) of param->vos_od are all the same.
        return false;

    if (gain_mod == 0) // must not be 0
        return false;
    if ((gain_mod & 0xF000) != 0x0000) // upper 4 bits should be 0.
        return false;

    if (vcm_mod == 0)
        return false;
    if ((vcm_mod & 0xF000) != 0x0000)
        return false;

    return true;
}

static bool adc_is_cal_table_valid(adc_cal_table_t *table) {

    if (table == 0)
        return false;

    adc_cal_param_t param;
    uint8_t differential;
    adc_pga_gain_t gain;

    for (differential = 0; differential < 2; differential++) {
        for (gain = ADC_PGA_GAIN_0P125; gain < ADC_PGA_GAIN_MAX; gain++) {
            adc_lld_get_cal_param_from_table(&param, table, differential, gain);
            if (!adc_is_cal_param_valid(&param))
                return false;
        }
    }

    return true;
}
#endif
/**
 * @brief Calibrate the vos_mod compensation parameter. ref <<GPADC calibration spec>> 4.3
 * @note
 * ------------------------------------------------------------
 * |   signal       |    value        |         note          |
 * +----------------+-----------------+-----------------------+
 * |  scal_gp       |     0/1         |  1:gtune_gp/4         |
 * +----------------+-----------------+-----------------------+
 * |  gtune_gp<1:0> |00/01/10/11      |00:0.5;   01:1         |
 * |                |                 |10:2  ;   11:4         |
 * +----------------+-----------------+-----------------------+
 * |  inp_gp<3:0>   |  4'b0010        |        VCM            |
 * +----------------+-----------------+-----------------------+
 * |  inn_gp<3:0>   |  4'b0010        |        VCM            |
 * +----------------+-----------------+-----------------------+
 * |                |  4'b0000        |   VCM =    0mv        |
 * |  vcm_gp<3:0>   |  4'b1000        |   VCM = 1000mv        |
 * |                |  4'b1111        |   VCM = 1875mv        |
 * +----------------+-----------------+-----------------------+
 *
 * formula: vos_mod[11:0] = vo_filter[11:0]*0.8.(d1 path)
 * 
 */
static void adc_calibrate_single_mode_vos_mod(void)
{
    adc_log("adc calibrate single mode vos mode start\n");

    adc_vcm_t vcm_gp[2] = {ADC_VCM_SEL_000mV, ADC_VCM_SEL_750mV};
    adc_channel_config_t channel_config;
    channel_config.callback = NULL;
    channel_config.inp_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_2MHZ;

    for(uint8_t vcm_i = 0; vcm_i < sizeof(vcm_gp)/sizeof(vcm_gp[0]); vcm_i++)
    {
        float vcm = vcm_gp[vcm_i]*0.125;
        for(uint8_t gain_i = 0; gain_i < ADC_PGA_GAIN_MAX; gain_i++)
        {
            int32_t sample_data[ADC_CAL_MEASURE_COUNT];
            int32_t calib_result = 0;
            uint32_t inp_gp = (1<<channel_config.inp_gp);

            channel_config.gtune_gp = s_adc_gain_table[gain_i].gtune_gp;
            channel_config.scal_gp = s_adc_gain_table[gain_i].scal_gp;
            channel_config.vcm_gp = vcm_gp[vcm_i];
            //channel_config.count    = ADC_CAL_MEASURE_COUNT;

            // set channel config
            adc_add_channel(&channel_config);
    
            adc_log("vcm = %x, scale=%x, gtune=%x\n", vcm_gp[vcm_i], s_adc_gain_table[gain_i].scal_gp, s_adc_gain_table[gain_i].gtune_gp);

            //volatile uint32_t gpadc_out = 0;
            //volatile uint32_t gpadc_out_array[8] = {0};

            // start convert
            for(uint32_t count_i =0 ; count_i <ADC_CAL_MEASURE_COUNT; count_i++)
            {
                // start convert
                HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;
                
                // wait the result
                while(!(HS_GPADC->INTR & inp_gp));
                HS_GPADC->INTR = inp_gp;

                // handle adc data
                sample_data[count_i] = expand_integer_32(adc_channel_read_data(channel_config.inp_gp), 16);
                calib_result += sample_data[count_i];

                adc_log("%08x ", sample_data[count_i]);
                if((count_i > 0)&&(7 == (count_i % 8)))
                {
                    adc_log("\n");
                }
            }
            adc_log("\n");
            adc_del_channel(channel_config.inp_gp);

            float vout = 0;
            float vos_mode_temp = 0;
            vout = (float)calib_result/ADC_CAL_MEASURE_COUNT/2048.0;
            ///gain = pow(2, gain_i)*0.125
            vos_mode_temp = (vout - vcm*1.25)*pow(2, gain_i)*0.125;
            int16_t vos_mode = (int16_t)(vos_mode_temp*2048.0);

            adc_log("vout = %f, vos_mode = 0x%x(%f)\n\n", vout, vos_mode, vos_mode_temp);

            // set result to reg
            volatile uint32_t *p_s_vos_g_lut_reg = &HS_GPADC->CMPN_S_VOS_G0_LUT_REG0 + (vcm_i * ADC_REG_STEP);
            uint32_t rd_data = *p_s_vos_g_lut_reg;
            switch(gain_i)
            {
                case ADC_PGA_GAIN_0P125:
                    p_s_vos_g_lut_reg += 2;
                    rd_data = *p_s_vos_g_lut_reg;
                    *p_s_vos_g_lut_reg = (rd_data & 0xFFFF0000) | (vos_mode & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_0P25:
                    p_s_vos_g_lut_reg += 2;
                    rd_data = *p_s_vos_g_lut_reg;
                    *p_s_vos_g_lut_reg = ((vos_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_0P5:
                    rd_data = *p_s_vos_g_lut_reg;
                    *p_s_vos_g_lut_reg = (rd_data & 0xFFFF0000) | (vos_mode & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_1:
                    rd_data = *p_s_vos_g_lut_reg;
                    *p_s_vos_g_lut_reg = ((vos_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_2:
                    p_s_vos_g_lut_reg += 1;
                    rd_data = *p_s_vos_g_lut_reg;
                    *p_s_vos_g_lut_reg = (rd_data & 0xFFFF0000) | (vos_mode & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_4:
                    p_s_vos_g_lut_reg += 1;
                    rd_data = *p_s_vos_g_lut_reg;
                    *p_s_vos_g_lut_reg = ((vos_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                    break;
            }
        }
    }
}

/**
 * @brief Calibrate the gain_mod compensation parameter. ref <<GPADC calibration spec>> 1 & 3.2
 * @note
 * ------------------------------------------------------------
 * |   signal       |    value        |         note          |
 * +----------------+-----------------+-----------------------+
 * |  scal_gp       |     0           |                       |
 * +----------------+-----------------+-----------------------+
 * |  gtune_gp<1:0> |     00          |     pga = 0.5         |
 * +----------------+-----------------+-----------------------+
 * |  inp_gp<3:0>   |  4'b0001        |        battery/3      |
 * +----------------+-----------------+-----------------------+
 * |  inn_gp<3:0>   |  4'b0010        |        VCM            |
 * +----------------+-----------------+-----------------------+
 * |  vcm_gp<3:0>   |  4'b0000        |      VCM = 0mv        |
 * +----------------+-----------------+-----------------------+
 *
 * formula: vout = vo_filter[11:0]*6/2048
 *          gain_mod'[11:0] = vout[11:0]*0.8*12'b0.01000111001-->(1/3.6v<<11 = 0x239)
 *          gain_mod[11:0]  = 2 - gain_mod'[11:0].(d1 path)
 * 
 */
static void adc_calibrate_single_mode_gain_mod(void)
{
    ///uint16_t gain_mode; //1 integer bit + 11 fraction bits
    adc_log("single gain mode calibration\n");

    adc_vcm_t vcm_gp[1] = {ADC_VCM_SEL_000mV};

    uint8_t vcm_i = 0;
    float vcm = (float)vcm_gp[vcm_i]*0.125;
    uint8_t pga_gain_max = ADC_PGA_GAIN_0P5;
    float vout[2][pga_gain_max+1];

    adc_channel_config_t channel_config;

    channel_config.callback = NULL;
    channel_config.inp_gp   = ADC_CHANNEL_EXTERN_CH3;//gpio3 connects to 1.0V source
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_2MHZ;

    for(uint8_t chl = 0; chl<2;chl++)
    {
        if(chl == 1)
        {
            channel_config.inp_gp   = ADC_CHANNEL_EXTERN_CH2;//gpio2 connects to GND
        }
        ///vcm=0mv, max_gain=0.5
        for(uint8_t gain_i = 0; gain_i <= pga_gain_max; gain_i++)
        {
            int32_t sample_data[ADC_CAL_MEASURE_COUNT];
            int32_t calib_result = 0;
            uint32_t inp_gp = (1<<channel_config.inp_gp);

            channel_config.gtune_gp = s_adc_gain_table[gain_i].gtune_gp;
            channel_config.scal_gp = s_adc_gain_table[gain_i].scal_gp;
            channel_config.vcm_gp = vcm_gp[vcm_i];
            //channel_config.count    = ADC_CAL_MEASURE_COUNT;

            // set channel config
            adc_add_channel(&channel_config);

            adc_log("vcm = %x, scale=%x, gtune=%x\n", \
                     vcm_gp[vcm_i],                   \
                     s_adc_gain_table[gain_i].scal_gp,\
                     s_adc_gain_table[gain_i].gtune_gp);

            // start convert
            for(uint32_t count_i =0 ; count_i <ADC_CAL_MEASURE_COUNT; count_i++)
            {
                // start convert
                HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;

                // wait the result
                while(!(HS_GPADC->INTR & inp_gp));
                HS_GPADC->INTR = inp_gp;

                // handle adc data
                sample_data[count_i] = expand_integer_32(adc_channel_read_data(channel_config.inp_gp), 16);
                calib_result += sample_data[count_i];
            }
            adc_log("\r\n");

            adc_del_channel(channel_config.inp_gp);

            vout[chl][gain_i] = (float)calib_result/ADC_CAL_MEASURE_COUNT/2048.0;
            adc_log("vout = %f\n\n", vout[chl][gain_i]);
        }
    }

    for(uint8_t gain_i = 0; gain_i <= pga_gain_max; gain_i++)
    {
        // set result to reg
        float gain_mode_temp = vout[0][gain_i]-vout[1][gain_i];
        adc_log("gain_mode_temp = %f\n", gain_mode_temp);
        gain_mode_temp = (CALIB_PIN_INPUT_VOL-vcm)*1.25/gain_mode_temp;
        adc_log("(CALIB_PIN_INPUT_VOL-vcm)*1.25/gain_mode_temp = %f\n", gain_mode_temp);
        uint32_t gain_mode = (uint32_t)(gain_mode_temp*2048.0);

        adc_log("gain_mode = %08x, %f\n\n", gain_mode, (float)gain_mode/2048.0);

        // set result to reg
        volatile uint32_t* p_s_gain_g_lut_reg = &HS_GPADC->CMPN_S_GAIN_G0_LUT_REG0 + (vcm_i * ADC_REG_STEP);
        uint32_t rd_data = *p_s_gain_g_lut_reg;
        switch(gain_i)
        {
            case ADC_PGA_GAIN_0P125:
                p_s_gain_g_lut_reg += 2;
                rd_data = *p_s_gain_g_lut_reg;
                *p_s_gain_g_lut_reg = (rd_data & 0xFFFF0000) | (gain_mode & 0x0000FFFF);
                break;
            case ADC_PGA_GAIN_0P25:
                p_s_gain_g_lut_reg += 2;
                rd_data = *p_s_gain_g_lut_reg;
                *p_s_gain_g_lut_reg = ((gain_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                break;
            case ADC_PGA_GAIN_0P5:
                rd_data = *p_s_gain_g_lut_reg;
                *p_s_gain_g_lut_reg = (rd_data & 0xFFFF0000) | (gain_mode & 0x0000FFFF);
                break;
            case ADC_PGA_GAIN_1:
                rd_data = *p_s_gain_g_lut_reg;
                *p_s_gain_g_lut_reg = ((gain_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                break;
            case ADC_PGA_GAIN_2:
                p_s_gain_g_lut_reg += 1;
                rd_data = *p_s_gain_g_lut_reg;
                *p_s_gain_g_lut_reg = (rd_data & 0xFFFF0000) | (gain_mode & 0x0000FFFF);
                break;
            case ADC_PGA_GAIN_4:
                p_s_gain_g_lut_reg += 1;
                rd_data = *p_s_gain_g_lut_reg;
                *p_s_gain_g_lut_reg = ((gain_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                break;
        }
    }

    ///vcm=0mv, max_gain=0.5
    ///gain_mode_1 = gain_mode_2 = gain_mode_4 = gain_mode_0p5
    volatile uint32_t *p_s_gain_g_lut_reg = &HS_GPADC->CMPN_S_GAIN_G0_LUT_REG0;
    uint32_t rd_data = *p_s_gain_g_lut_reg;
    *p_s_gain_g_lut_reg = ((rd_data<<16)&0xFFFF0000)|(rd_data&0x0000FFFF);
    *(p_s_gain_g_lut_reg+1) = *p_s_gain_g_lut_reg;

    ///vcm=750mv, max_gain=1
    ///set the gain_mode of vcm_750mv eqaul to the value of vcm=000mv
    HS_GPADC->CMPN_S_GAIN_G1_LUT_REG0 = HS_GPADC->CMPN_S_GAIN_G0_LUT_REG0;
    HS_GPADC->CMPN_S_GAIN_G1_LUT_REG1 = HS_GPADC->CMPN_S_GAIN_G0_LUT_REG1;
    HS_GPADC->CMPN_S_GAIN_G1_LUT_REG2 = HS_GPADC->CMPN_S_GAIN_G0_LUT_REG2;

    ///vcm=1250mv, it is forbidden to use, keeps their default value.
}

/**
 * @brief Calibrate the vcm_mod compensation parameter. ref <<GPADC calibration spec>> 1 & 3.3
 * @note
 * ------------------------------------------------------------
 * |   signal       |    value        |         note          |
 * +----------------+-----------------+-----------------------+
 * |  scal_gp       |     0           |                       |
 * +----------------+-----------------+-----------------------+
 * |  gtune_gp<1:0> |     00          |     pga = 0.5         |
 * +----------------+-----------------+-----------------------+
 * |  inp_gp<3:0>   |  4'b0110        |        GPIO3          |
 * +----------------+-----------------+-----------------------+
 * |  inn_gp<3:0>   |  4'b0010        |        VCM            |
 * +----------------+-----------------+-----------------------+
 * |  vcm_gp<3:0>   |  4'b1000        |   VCM = 1000mv        |
 * +----------------+-----------------+-----------------------+
 *
 * formula: vout = vo_filter[11:0]*2/2048+1
 *          vcm_mod[11:0] = 13'b01.00000000000 - vout[15:0]
 * 
 */
static void adc_calibrate_single_mode_vcm_mod(void)
{
    ///uint16_t vcm_mode; //1 integer bit + 11 fraction bits

    adc_log("single vcm calibration\n\n");

    adc_vcm_t vcm_gp[2] = {ADC_VCM_SEL_000mV, ADC_VCM_SEL_750mV};
    adc_channel_config_t channel_config;
    
    channel_config.callback = NULL;
    channel_config.inp_gp   = ADC_CHANNEL_EXTERN_CH3;//connects to 1.0V source
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_2MHZ;

    for(uint8_t vcm_i = 1; vcm_i < sizeof(vcm_gp)/sizeof(vcm_gp[0]); vcm_i++)
    {
        float vcm = (float)vcm_gp[vcm_i]*0.125;
        uint8_t pga_gain_max = 1==vcm_i?ADC_PGA_GAIN_1:ADC_PGA_GAIN_0P5;

        ///vcm=0mv, max_gain=0.5
        ///vcm=750mv, max_gain=1
        for(uint8_t gain_i = 0; gain_i <= pga_gain_max; gain_i++)
        {
            int32_t sample_data[ADC_CAL_MEASURE_COUNT];
            int32_t calib_result = 0;
            uint32_t inp_gp = (1<<channel_config.inp_gp);

            channel_config.gtune_gp = s_adc_gain_table[gain_i].gtune_gp;
            channel_config.scal_gp = s_adc_gain_table[gain_i].scal_gp;
            channel_config.vcm_gp = vcm_gp[vcm_i];
            //channel_config.count    = ADC_CAL_MEASURE_COUNT;

            // set channel config
            adc_add_channel(&channel_config);

            adc_log("vcm = %x, scale=%x, gtune=%x\n",\
                    vcm_gp[vcm_i], s_adc_gain_table[gain_i].scal_gp,\
                    s_adc_gain_table[gain_i].gtune_gp);

            // start convert
            for(uint32_t count_i =0 ; count_i <ADC_CAL_MEASURE_COUNT; count_i++)
            {
                // start convert
                HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;

                // wait the result
                while(!(HS_GPADC->INTR & inp_gp));
                HS_GPADC->INTR = inp_gp;

                // handle adc data
                sample_data[count_i] = expand_integer_32(adc_channel_read_data(channel_config.inp_gp), 16);
                calib_result += sample_data[count_i];
                //adc_log("%08x ", sample_data[count_i]);
                //if((count_i > 0)&&(7 == (count_i % 8)))
                //{
                //    adc_log("\n");
                //}
            }
            //adc_log("\n");
            adc_del_channel(channel_config.inp_gp);

            float vout = (float)calib_result/ADC_CAL_MEASURE_COUNT/2048.0;
            adc_log("vout = %f\n", vout);
            float vcm_mode_temp = vout-vcm*1.25;
            vcm_mode_temp = (CALIB_PIN_INPUT_VOL*1.25-vcm_mode_temp)/vcm;
            adc_log("vcm_mode_temp=%f\n", vcm_mode_temp);
            uint32_t vcm_mode = (uint32_t)(vcm_mode_temp*2048.0);
            adc_log("vcm_mode=%x\n\n", vcm_mode);

            // set result to reg
            volatile uint32_t *p_s_vcm_g_lut_reg = &HS_GPADC->CMPN_S_VCM_G0_LUT_REG0 + (vcm_i * ADC_REG_STEP);
            uint32_t rd_data = *p_s_vcm_g_lut_reg;

            switch(gain_i)
            {
                case ADC_PGA_GAIN_0P125:
                    p_s_vcm_g_lut_reg += 2;
                    rd_data = *p_s_vcm_g_lut_reg;
                    *p_s_vcm_g_lut_reg = (rd_data & 0xFFFF0000) | (vcm_mode & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_0P25:
                    p_s_vcm_g_lut_reg += 2;
                    rd_data = *p_s_vcm_g_lut_reg;
                    *p_s_vcm_g_lut_reg = ((vcm_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_0P5:
                    rd_data = *p_s_vcm_g_lut_reg;
                    *p_s_vcm_g_lut_reg = (rd_data & 0xFFFF0000) | (vcm_mode & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_1:
                    rd_data = *p_s_vcm_g_lut_reg;
                    *p_s_vcm_g_lut_reg = ((vcm_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_2:
                    p_s_vcm_g_lut_reg += 1;
                    rd_data = *p_s_vcm_g_lut_reg;
                    *p_s_vcm_g_lut_reg = (rd_data & 0xFFFF0000) | (vcm_mode & 0x0000FFFF);
                    break;
                case ADC_PGA_GAIN_4:
                    p_s_vcm_g_lut_reg += 1;
                    rd_data = *p_s_vcm_g_lut_reg;
                    *p_s_vcm_g_lut_reg = ((vcm_mode<<16) & 0xFFFF0000) | (rd_data & 0x0000FFFF);
                    break;
            }
        }
    }

    ///vcm=0mv, keeps their default value.

    ///vcm=750mv, max_gain=1
    ///vcm_mode_2 = vcm_mode_4 = vcm_mode_1
    volatile uint32_t *p_s_vcm_g_lut_reg = &HS_GPADC->CMPN_S_VCM_G1_LUT_REG0;
    volatile uint32_t rd_data = (*p_s_vcm_g_lut_reg>>16)&0x0000FFFF;
    *(p_s_vcm_g_lut_reg+1) = ((rd_data<<16)&0xFFFF0000) | (rd_data&0x0000FFFF);

    ///vcm=1250mv, it is forbidden to use, keeps their default value.
}

static void adc_calibrate_single_mode(void)
{
    #ifdef DEBUG_ADC_LOG
    uint8_t i = 0;
    volatile uint32_t *p_reg;
    #endif

    adc_log("adc calibrate single mode start:\n");

    // 0. set adc config
    adc_config_t config;
    config.trigger_mode = ADC_TRIGGER_MODDE_SW;
    config.trigger_res = ADC_TRIG_RES_CHANNEL;
    config.trigger_count = 1;
    config.hw_trigger_sel = ADC_HW_TIMER0_0;
    adc_config(&config);
    
    // 1. calibrate vos mod
    adc_calibrate_single_mode_vos_mod();
    
    // 2. calibrate gain mod
    adc_calibrate_single_mode_gain_mod();
    // 3. calibrate vcm mod
    adc_calibrate_single_mode_vcm_mod();

    #ifdef DEBUG_ADC_LOG
    p_reg = &HS_GPADC->CMPN_S_VOS_G0_LUT_REG0;
    for(i = 0; i < 3; i++)
    {
        adc_log("cmpn_s_vos_g%d_lut_reg0=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP));
        adc_log("cmpn_s_vos_g%d_lut_reg1=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 1));
        adc_log("cmpn_s_vos_g%d_lut_reg2=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 2));
    }

    p_reg = &HS_GPADC->CMPN_S_GAIN_G0_LUT_REG0;
    for(i = 0; i < 3; i++)
    {
        adc_log("cmpn_s_gain_g%d_lut_reg0=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP));
        adc_log("cmpn_s_gain_g%d_lut_reg1=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 1));
        adc_log("cmpn_s_gain_g%d_lut_reg2=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 2));
    }

    p_reg = &HS_GPADC->CMPN_S_VCM_G0_LUT_REG0;
    for(i = 0; i < 3; i++)
    {
        adc_log("cmpn_s_vcm_g%d_lut_reg0=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP));
        adc_log("cmpn_s_vcm_g%d_lut_reg1=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 1));
        adc_log("cmpn_s_vcm_g%d_lut_reg2=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 2));
    }

    adc_log("adc calibrate single mode stop:\n\n");
    #endif
}

static void adc_calibrate_diff_mode_vos_mod(void)
{
    volatile uint32_t* p_d_vos_lut_reg = &HS_GPADC->CMPN_D_VOS_LUT_REG0;
    volatile uint32_t* p_s_vos_g0_lut_reg = &HS_GPADC->CMPN_S_VOS_G0_LUT_REG0;

    *(p_d_vos_lut_reg + 0) = *(p_s_vos_g0_lut_reg + 0);
    *(p_d_vos_lut_reg + 1) = *(p_s_vos_g0_lut_reg + 1);
    *(p_d_vos_lut_reg + 2) = *(p_s_vos_g0_lut_reg + 2);
}

static void adc_calibrate_diff_mode_gain_mod(void)
{
    volatile uint32_t* p_d_gain_lut_reg = &HS_GPADC->CMPN_D_GAIN_LUT_REG0;
    volatile uint32_t* p_s_gain_g0_lut_reg = &HS_GPADC->CMPN_S_GAIN_G0_LUT_REG0;

    *(p_d_gain_lut_reg + 0) = *(p_s_gain_g0_lut_reg + 0);
    *(p_d_gain_lut_reg + 1) = *(p_s_gain_g0_lut_reg + 1);
    *(p_d_gain_lut_reg + 2) = *(p_s_gain_g0_lut_reg + 2);
}

static void adc_calibrate_diff_mode(void)
{
    #ifdef DEBUG_ADC_LOG
    volatile uint32_t *p_reg;

    adc_log("adc calibrate diff mode, start:\n");
    #endif
    
    adc_calibrate_diff_mode_vos_mod();
    adc_calibrate_diff_mode_gain_mod();

    #ifdef DEBUG_ADC_LOG
    adc_log("adc diff mode, after calib:\n");
    p_reg = &HS_GPADC->CMPN_D_VOS_LUT_REG0;
    adc_log("cmpn_d_vos_lut_reg0=0x%08x\n", *(p_reg + 0));
    adc_log("cmpn_d_vos_lut_reg1=0x%08x\n", *(p_reg + 1));
    adc_log("cmpn_d_vos_lut_reg2=0x%08x\n", *(p_reg + 2));

    p_reg = &HS_GPADC->CMPN_D_GAIN_LUT_REG0;
    adc_log("cmpn_d_gain_lut_reg0=0x%08x\n", *(p_reg + 0));
    adc_log("cmpn_d_gain_lut_reg1=0x%08x\n", *(p_reg + 1));
    adc_log("cmpn_d_gain_lut_reg2=0x%08x\n", *(p_reg + 2));

    adc_log("adc calibrate diff mode, stop:\n\n");
    #endif
}

static int32_t adc_calibrate_temperature(void)
{
    adc_log("adc_calibrate_temperature start\n");
    
    adc_sample_data_t sample_data[ADC_CAL_MEASURE_COUNT];
    int32_t calib_result = 0;
    adc_channel_config_t channel_config;
    
    channel_config.callback = NULL;
    channel_config.inp_gp   = ADC_CHANNEL_CHIP_TEMPERATURE;
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_2MHZ;
    channel_config.gtune_gp = ADC_GTUNE_GP_4;
    channel_config.scal_gp  = ADC_SCAL_GP_DISABLE;
    channel_config.vcm_gp   = ADC_VCM_SEL_750mV;
    //channel_config.count    = ADC_CAL_MEASURE_COUNT;

    // set channel config
    adc_add_channel(&channel_config);

    adc_log("vcm = %x, scale=%x, gtune=%x\n",
             channel_config.vcm_gp,\
             channel_config.scal_gp,\
             channel_config.gtune_gp);

    // start convert
    for(uint8_t count_i =0 ; count_i <ADC_CAL_MEASURE_COUNT; count_i++)
    {
        // start convert
        HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;
        // wait the result
        while(!(HS_GPADC->INTR & (1<<channel_config.inp_gp)));
        HS_GPADC->INTR = (1<<channel_config.inp_gp);
        // handle adc data
        sample_data[count_i] = adc_channel_read_data(channel_config.inp_gp)&0x0000FFFF;
        calib_result += (int32_t)sample_data[count_i];
    }
    adc_del_channel(channel_config.inp_gp);

    // avg result
    float vout = (float)calib_result/ADC_CAL_MEASURE_COUNT*0.8;
    // set result to reg
    int32_t t_calib = (int32_t)vout;

    adc_log("t_calib = %08x, %f\n", t_calib, vout/2048.0);
    adc_log("adc_calibrate_temperature stop\n\n");

    return t_calib;
}

static float adc_change2temperature(float d3_out)
{
    float ret;

    ret = ((float)(adc_env.cal_table.temp_calib_offset.offset - d3_out)/2048.0)*1000.0/1.824 + 25;

    return ret;
}

static float adc_data2float(adc_channel_t inp_gp, int32_t data)
{
    float ret = 0.0;
    //adc_log("adc data ch=%x, data=%08x\n", inp_gp, data);
    switch(inp_gp)
    {
        case ADC_CHANNEL_CHIP_TEMPERATURE:
            ret = adc_change2temperature(data);
            break;
        case ADC_CHANNEL_CHIP_BATTERY:
            ret = (float)data*0.8/2048.0*3.0;
            break;
        default:
            ret = (float)data*0.8/2048.0;
            break;
    }
    return ret;
}
#if 0
static void adc_handle_override(uint32_t intr_mask)
{
    uint8_t i=0;
    for(i=0; i< ADC_CHANNEL_NUM;i++)
    {
        if (adc_env.channel[i].use == 1 && adc_env.channel[i].callback != NULL)
        {
            adc_env.channel[i].callback(ADC_EVENT_OVERRIDE, 0);
        }
    }
}


static void adc_handle_dma(uint32_t intr_mask)
{
    uint8_t i=0;
    for(i=0; i< ADC_CHANNEL_NUM;i++)
    {
        if (adc_env.channel[i].use == 1 && adc_env.channel[i].callback != NULL)
        {
            adc_env.channel[i].callback(ADC_EVENT_DMA_DONE, 0);
        }
    }
}

/**
 * @brief Manually read converted data to buffer.
 *
 * @param[in] src    channels mask
 */
static void adc_handle_channel_data(const uint32_t src) {
    uint8_t ch_idx = 0;
    float channel_data = 0.0;
    
    adc_env.state = ADC_STATE_COMPLETE;

    // find channel index
    for(ch_idx=0; ch_idx< ADC_CHANNEL_NUM;ch_idx++)
    {
        if ((1<<ch_idx) & src)
        {
            break;
        }
    }

    if (ch_idx == ADC_CHANNEL_NUM) return;

    uint16_t reg_data = (adc_sample_data_t)(adc_channel_read_data(adc_env.channel[ch_idx].inp_gp)&0x0000FFFF);
    adc_env.channel[ch_idx].data += reg_data;
    adc_env.channel[ch_idx].numbers--;
    //adc_log("sample= %d, %08x, %08x\n", adc_env.channel[ch_idx].numbers, reg_data, adc_env.channel[ch_idx].data);
    
    if (adc_env.channel[ch_idx].numbers == 0) {
        adc_callback_t cb = NULL;
        adc_env.channel[ch_idx].data /= adc_env.channel[ch_idx].count;
        
        channel_data = adc_data2float(adc_env.channel[ch_idx].inp_gp, adc_env.channel[ch_idx].data);
    
        if (adc_env.channel[ch_idx].callback != NULL) {
            cb = adc_env.channel[ch_idx].callback;
        }
        
        // delete channel
        adc_del_channel(adc_env.channel[ch_idx].inp_gp);
        if (adc_env.ch_numbers == 0)
        {
            adc_stop_convert();
            adc_stop();
        }
        if (cb != NULL)
        {
            cb((adc_event_t)(1<<ch_idx), channel_data);
        }
    }
    else if (adc_env.ch_numbers == 1)
    {
        adc_start_convert();
    }
}

static void adc_handle_eos_data(const uint32_t src) {
    uint8_t i=0;
    adc_env.state = ADC_STATE_COMPLETE;
    for(i=0; i< ADC_CHANNEL_NUM;i++)
    {
        if (adc_env.channel[i].use == 1)
        {
            adc_handle_channel_data((1<<i));
        }
    }
    if (adc_env.ch_numbers == 0)
    {
       adc_stop_convert();
       adc_stop();
    }
    else {
        adc_start_convert();
    }
}
#endif
void ADC_IRQHandler(void)
{
    if (adc_env.state == ADC_STATE_CALIBRATE)
    {
        return;
    }

    uint32_t intr_mask = HS_GPADC->INTR_MSK;
    HS_GPADC->INTR_MSK = 0;

    uint32_t src = HS_GPADC->INTR & GPADC_INTR_ALL_MASK;
    HS_GPADC->INTR = src;
#if 0
    if (src & GPADC_OVR_MASK) {
        // handle the overrid
        adc_handle_override(intr_mask);
    }
    else if (src & GPADC_DMA_MASK) {
        // handle dma end
        adc_handle_dma(intr_mask);
    }
    else if ((src & GPADC_EOS_MASK) || (src & GPADC_EOA_MASK)) {
        // end of group conversion:
        adc_handle_eos_data(intr_mask);
    }
    else {
        // end of channel conversion
        adc_handle_channel_data(src);
    }
#else
    uint8_t i=0;
    for(i=0; i< ADC_CHANNEL_NUM;i++)
    {
        if (adc_env.callback[i] != NULL && 
            ((src & GPADC_EOS_MASK) || (src & GPADC_EOA_MASK) || (src & GPADC_DMA_MASK) || (src & GPADC_OVR_MASK) 
             || (src & (1<<i)))
            )
        {
            adc_env.callback[i](src, 0);
        }
    }
#endif
    HS_GPADC->INTR_MSK = intr_mask;

}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */
/**
 * @brief   Low level ADC driver initialization.
 *
 */
void adc_init(void) {
    adc_env.state = ADC_STATE_READY;
    adc_env.handle_type = ADC_DATA_HANDLE_BLOCK;

    pmu_ana_enable(true, PMU_ANA_ADC);
    volatile uint32_t* p_reg = &HS_GPADC->CMPN_S_VOS_G0_LUT_REG0;
    uint32_t *p_cal_table = NULL;
    if (adc_env.is_use)
    {
        p_cal_table = (uint32_t *)(&adc_env.cal_table);
    }
    else
    {
        p_cal_table = (uint32_t *)s_adc_cal_table_default;
        adc_env.cal_table.temp_calib_offset.offset = s_adc_cal_table_default[33];
    }

    for(uint8_t i = 0; i < 32; i = i + 3)
    {
        *(p_reg + ((i/3)*ADC_REG_STEP) + 0) = p_cal_table[i+0];
        *(p_reg + ((i/3)*ADC_REG_STEP) + 1) = p_cal_table[i+1];
        *(p_reg + ((i/3)*ADC_REG_STEP) + 2) = p_cal_table[i+2];
    }

#if 0
    uint8_t i = 0;
    p_reg = &HS_GPADC->CMPN_S_VOS_G0_LUT_REG0;

    for (i = 0; i < 3; i++) {
        adc_log("cmpn_s_vos_g%d_lut_reg0=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP));
        adc_log("cmpn_s_vos_g%d_lut_reg1=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 1));
        adc_log("cmpn_s_vos_g%d_lut_reg2=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 2));
    }

    p_reg = &HS_GPADC->CMPN_S_GAIN_G0_LUT_REG0;
    for (i = 0; i < 3; i++) {
        adc_log("cmpn_s_gain_g%d_lut_reg0=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP));
        adc_log("cmpn_s_gain_g%d_lut_reg1=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 1));
        adc_log("cmpn_s_gain_g%d_lut_reg2=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 2));
    }

    p_reg = &HS_GPADC->CMPN_S_VCM_G0_LUT_REG0;
    for (i = 0; i < 3; i++) {
        adc_log("cmpn_s_vcm_g%d_lut_reg0=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP));
        adc_log("cmpn_s_vcm_g%d_lut_reg1=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 1));
        adc_log("cmpn_s_vcm_g%d_lut_reg2=0x%08x\n", i, *(p_reg + i*ADC_REG_STEP + 2));
    }
#endif

    pmu_ana_enable(false, PMU_ANA_ADC);
}

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
 *  @param cal_table   Pointer to @p adc_cal_table_t object where the
 *                     calibration parameters are to be returned.
 */

void adc_calibrate(adc_cal_table_t *cal_table)
{
    ///set the defalut value to the calibration register
    cal_table->cmpn_s_vos_g0_lut_reg0  = 0x00000000;
    cal_table->cmpn_s_vos_g0_lut_reg1  = 0x00000000;
    cal_table->cmpn_s_vos_g0_lut_reg2  = 0x00000000;
    cal_table->cmpn_s_vos_g1_lut_reg0  = 0x00000000;
    cal_table->cmpn_s_vos_g1_lut_reg1  = 0x00000000;
    cal_table->cmpn_s_vos_g1_lut_reg2  = 0x00000000;
    cal_table->cmpn_s_vos_g2_lut_reg0  = 0x00000000;
    cal_table->cmpn_s_vos_g2_lut_reg1  = 0x00000000;
    cal_table->cmpn_s_vos_g2_lut_reg2  = 0x00000000;
    cal_table->cmpn_s_gain_g0_lut_reg0 = 0x08000800;
    cal_table->cmpn_s_gain_g0_lut_reg1 = 0x08000800;
    cal_table->cmpn_s_gain_g0_lut_reg2 = 0x08000800;
    cal_table->cmpn_s_gain_g1_lut_reg0 = 0x08000800;
    cal_table->cmpn_s_gain_g1_lut_reg1 = 0x08000800;
    cal_table->cmpn_s_gain_g1_lut_reg2 = 0x08000800;
    cal_table->cmpn_s_gain_g2_lut_reg0 = 0x08000800;
    cal_table->cmpn_s_gain_g2_lut_reg1 = 0x08000800;
    cal_table->cmpn_s_gain_g2_lut_reg2 = 0x08000800;
    cal_table->cmpn_s_vcm_g0_lut_reg0  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g0_lut_reg1  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g0_lut_reg2  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g1_lut_reg0  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g1_lut_reg1  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g1_lut_reg2  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g2_lut_reg0  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g2_lut_reg1  = 0x0A000A00;
    cal_table->cmpn_s_vcm_g2_lut_reg2  = 0x0A000A00;
    cal_table->cmpn_d_vos_lut_reg0     = 0x00000000;
    cal_table->cmpn_d_vos_lut_reg1     = 0x00000000;
    cal_table->cmpn_d_vos_lut_reg2     = 0x00000000;
    cal_table->cmpn_d_gain_lut_reg0    = 0x08000800;
    cal_table->cmpn_d_gain_lut_reg1    = 0x08000800;
    cal_table->cmpn_d_gain_lut_reg2    = 0x08000800;

    adc_set_calibarate_param(cal_table);

    adc_init();
    adc_start();

    //adc_open_debug_bus();
    
    adc_env.state = ADC_STATE_CALIBRATE;
    adc_calibrate_single_mode();
    adc_calibrate_diff_mode();
    adc_env.cal_table.temp_calib_offset.offset = adc_calibrate_temperature();

    // copy from reg to calibrate table
    volatile uint32_t* p_reg = &HS_GPADC->CMPN_S_VOS_G0_LUT_REG0;
 
    adc_log("adc_calibrate result\n");
    
    if (cal_table != NULL) {
        uint32_t *p_cal_table = (uint32_t *)cal_table;

        cal_table->temp_calib_offset.offset = adc_env.cal_table.temp_calib_offset.offset;
    
        for(uint8_t i = 0; i < 32; i = i + 3)
        {
            p_cal_table[i+0]  = *(p_reg + ((i/3)*ADC_REG_STEP) + 0);
            p_cal_table[i+1]  = *(p_reg + ((i/3)*ADC_REG_STEP) + 1);
            p_cal_table[i+2]  = *(p_reg + ((i/3)*ADC_REG_STEP) + 2);
            adc_log("%08x, %f, %f\n", p_cal_table[i+0],
                                        (float)expand_integer_32(p_cal_table[i+0]>>16, ADC_Q)/2048.0,
                                        (float)expand_integer_32(p_cal_table[i+0]&0xFFFF, ADC_Q)/2048.0);
            adc_log("%08x, %f, %f\n", p_cal_table[i+1],
                                        (float)expand_integer_32(p_cal_table[i+1]>>16, ADC_Q)/2048.0,
                                        (float)expand_integer_32(p_cal_table[i+1]&0xFFFF, ADC_Q)/2048.0);
            adc_log("%08x, %f, %f\n", p_cal_table[i+2],
                                        (float)expand_integer_32(p_cal_table[i+2]>>16, ADC_Q)/2048.0,
                                        (float)expand_integer_32(p_cal_table[i+2]&0xFFFF, ADC_Q)/2048.0);
        }

        adc_log("temp calib: %08x\n\n", cal_table->temp_calib_offset.offset);
    }

    adc_stop();

    adc_env.state = ADC_STATE_READY;
}

void adc_config(const adc_config_t *config)
{
    // cfg1
    register_set(&HS_GPADC->ADC_CFG1, MASK_9REG(GPADC_TRIG_MODE,      config->trigger_mode,
                GPADC_TRIG_RES,       config->trigger_res,
                GPADC_DMA_EN,         0,
                GPADC_AUTO_DELAY,     1,
                GPADC_TRIG_HW_SEL,    config->hw_trigger_sel,
                GPADC_ALIGN,          0,
                GPADC_APPEND_CH_ID,   0,
                GPADC_SWAP_EN,        0,
                GPADC_SMP_FP_BYPASS1, 0));

    // cfg2
    register_set(&HS_GPADC->ADC_CFG2, MASK_2REG(GPADC_SCANDIR,        ADC_SCAN_DIR_LSB,
                                                GPADC_SEQ_LIFE,       config->trigger_count));

    register_set(&HS_GPADC->CMPN_CTRL, MASK_2REG(GPADC_COMPEN_EN,     1,
                                                 GPADC_SEL_COMPEN_ME, 0));

#ifndef CONFIG_HS6621
    register_set(&HS_GPADC->FILTER_CTRL, MASK_1REG(GPADC_OUT_VAILD_CNT,     0xF));
#endif
    // set interrupt
    HS_GPADC->INTR = GPADC_INTR_ALL_MASK;
}

HS_DMA_CH_Type *adc_dma_config(HS_DMA_CH_Type *dma, const adc_dma_config_t *config, uint32_t *src_addr)
{
    if(dma == NULL)
        dma = dma_allocate();

    if(dma)
    {
        bool res;
        dma_id_t id;
        //uint32_t src_addr;
        dma_config_t dconfig;
        dma_slave_buswidth_t buswidth = DMA_SLAVE_BUSWIDTH_32BITS;

        id = ADC_DMA_ID;
        *src_addr = (uint32_t)&HS_GPADC->CH_DMA_DATA;

        dconfig.slave_id       = id;
        dconfig.direction      = DMA_DEV_TO_MEM;
        dconfig.src_addr_width = buswidth;
        dconfig.dst_addr_width = buswidth;
        dconfig.src_burst      = DMA_BURST_LEN_1UNITS;
        dconfig.dst_burst      = DMA_BURST_LEN_1UNITS;
        dconfig.dev_flow_ctrl  = false;
        dconfig.priority       = 0;
        dconfig.callback       = config->callback;

        dconfig.lli.enable     = false;
        dconfig.lli.use_fifo   = config->use_fifo;
        dconfig.lli.src_addr   = *src_addr;
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

        // Enable ADC DMA
        register_set1(&HS_GPADC->ADC_CFG1, GPADC_DMA_EN_MASK);
        register_set1(&HS_GPADC->ADC_SW_TRIGGER, GPADC_SW_TRIGGER_TRUE_MASK);
        HS_GPADC->INTR_MSK = GPADC_DMA_MASK;

        NVIC_SetPriority(ADC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(ADC_IRQn);
        NVIC_EnableIRQ(ADC_IRQn);

        adc_log("adc dma config %08x, %08x\n", 
            HS_GPADC->ADC_CFG1, 
            HS_GPADC->ADC_SW_TRIGGER);
    }

    return dma;
}

uint8_t adc_start(void)
{
    if (adc_env.state >= ADC_STATE_START) return false;
    
    if (adc_env.state != ADC_STATE_CALIBRATE && adc_env.handle_type != ADC_DATA_HANDLE_BLOCK)
    {
        // enable sys interrupt
        NVIC_SetPriority(ADC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(ADC_IRQn);
        NVIC_EnableIRQ(ADC_IRQn);
    }

    // enable clock
    pmu_ana_enable(true, PMU_ANA_ADC);
#if 0
    REGW1(&HS_DAIF->CLK_ENS, DAIF_ADC_CLK_EN_MASK);
    NVIC_DisableIRQ(ADC_IRQn);
#endif

    register_set(&HS_GPADC->ADC_CFG2, MASK_1REG(GPADC_SEQ_VECT, 0));
    HS_GPADC->ADC_CFG0 = GPADC_ADC_STOP_MASK;

    // set interrupt
    HS_GPADC->INTR_MSK = 0;
    HS_GPADC->INTR = GPADC_INTR_ALL_MASK;
    
    pmu_lowpower_prevent(PMU_LP_ADC);
    
    adc_env.state = ADC_STATE_START;

    //adc_log("AHB=%08x, CLK=%08x, ENS=%08x\n\n", HS_PSO->ANA_IF_AHB_CFG, HS_PSO->ANA_IF_CLK_CFG, HS_DAIF->CLK_ENS);
    return true;
}

uint8_t adc_stop(void)
{   
    // disable interrupt
    NVIC_DisableIRQ(ADC_IRQn);

    // stop clock
    pmu_ana_enable(false, PMU_ANA_ADC);

    pmu_lowpower_allow(PMU_LP_ADC);	 

    return true;
}

/**
 * @brief   Starts an ADC conversion.
 * @details Starts an conversion operation.
 *
 */
void adc_start_convert(void)
{ 
    if ((adc_env.state == ADC_STATE_START) || ((adc_env.state == ADC_STATE_COMPLETE))) {
        // start convert
        HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;

        adc_env.state = ADC_STATE_ACTIVE;
    }
}

/**
 * @brief   Stops an ongoing conversion.
 * @details This function stops the currently ongoing conversion and returns
 *          the driver in the @p ADC_READY state. If there was no conversion
 *          being processed then the function does nothing.
 */
void adc_stop_convert(void)
{
    if (adc_env.state <= ADC_STATE_START) return;

    adc_env.state = ADC_STATE_STOP;
    
     // stop adc convert
    HS_GPADC->ADC_CFG0 = GPADC_ADC_STOP_MASK;

    adc_env.state = ADC_STATE_READY;
}

uint8_t adc_add_channel(adc_channel_config_t *channel)
{
    uint8_t auto_compensation = 1;
    //uint8_t vcm_gp = channel->vcm_gp;
    uint16_t inp_gp = register_get(&HS_GPADC->ADC_CFG2, MASK_POS(GPADC_SEQ_VECT));
    
    volatile uint32_t *p_lut_reg = &HS_GPADC->CH_0_CFG;

    inp_gp |= (1<<channel->inp_gp);
    register_set(&HS_GPADC->CMPN_CTRL, MASK_1REG(GPADC_COMPEN_EN, 1));

    register_set(&HS_GPADC->ADC_CFG2, MASK_1REG(GPADC_SEQ_VECT, inp_gp));

    //ch_x_cfg
    register_set(p_lut_reg+channel->inp_gp, MASK_7REG(GPADC_GPADC_CLKSEL, channel->clk_sel,
                                                  GPADC_INN_GP, channel->inn_gp,
                                                  GPADC_VCM_GP, channel->vcm_gp,
                                                  GPADC_GTUNE_GP, channel->gtune_gp,
                                                  GPADC_SCAL_GP, channel->scal_gp,
                                                  GPADC_AUTO_COMPEN, auto_compensation,
                                                  GPADC_SWAP_GP, 0));

    ///vcm <= g0_thrsh_0x05(625mV), using the vos_mode and gain_mode of vcm_000mV
    ///vcm <= g1_thrsh_0x0F(1875mV), using the vos_mode and gain_mode of vcm_750mV
    register_set(&HS_GPADC->VCM_CFG, MASK_2REG(GPADC_G0_THRSH, 5, GPADC_G1_THRSH, 0xF));

#if 0
    adc_env.channel[channel->inp_gp].callback = channel->callback;
    adc_env.channel[channel->inp_gp].inp_gp = channel->inp_gp;
    adc_env.channel[channel->inp_gp].use = 1;
    adc_env.channel[channel->inp_gp].count = channel->count;
    adc_env.channel[channel->inp_gp].numbers = channel->count;
    adc_env.channel[channel->inp_gp].data = 0;

    adc_env.ch_numbers++;
#else
    adc_env.callback[channel->inp_gp] = channel->callback;
#endif
    HS_GPADC->INTR |= (1<<channel->inp_gp);

    if (channel->callback != NULL)
    {
        adc_env.handle_type = ADC_DATA_HANDLE_IRQ;
    }
    else
    {
       NVIC_DisableIRQ(ADC_IRQn);
    }
    
    if(adc_env.state != ADC_STATE_CALIBRATE && adc_env.handle_type != ADC_DATA_HANDLE_BLOCK) {
        HS_GPADC->INTR_MSK |= (1<<channel->inp_gp);
#if 0
        if (adc_env.ch_numbers >1)
        {
            register_set(&HS_GPADC->ADC_CFG1, MASK_1REG(GPADC_TRIG_RES, ADC_TRIG_RES_SEQUENCE));
            HS_GPADC->INTR_MSK = GPADC_EOS_MASK;
        }
#endif
    }

    adc_log("adc config %08x, %08x, %08x, %08x, %08x\n", 
            HS_GPADC->ADC_CFG0, 
            HS_GPADC->ADC_CFG1, 
            HS_GPADC->ADC_CFG2, 
            HS_GPADC->CMPN_CTRL,
            *p_lut_reg+channel->inp_gp);
    return true;
}

uint8_t adc_del_channel(adc_channel_t inp_gp)
{
    // clear interrupt
    HS_GPADC->INTR_MSK &= ~(1<<inp_gp);

    uint16_t cur_inp_gp = register_get(&HS_GPADC->ADC_CFG2, MASK_POS(GPADC_SEQ_VECT));
    cur_inp_gp &= ~(1<<inp_gp);
    register_set(&HS_GPADC->ADC_CFG2, MASK_1REG(GPADC_SEQ_VECT, cur_inp_gp));

#if 0
    memset(&adc_env.channel[inp_gp], 0 , sizeof(adc_channel_env_t));

    if(adc_env.ch_numbers >0)
        adc_env.ch_numbers--;
    if (adc_env.ch_numbers == 0)
    { 
       HS_GPADC->INTR_MSK =0;
    }
#else
    adc_env.callback[inp_gp] = 0;
#endif
    return true;
}

int16_t adc_temperature_read(adc_callback_t cb)
{
    adc_channel_config_t channel_config;
    adc_config_t config;
    adc_init();
    adc_start();
    
    config.trigger_mode   = ADC_TRIGGER_MODDE_SW;
    config.trigger_res    = ADC_TRIG_RES_CHANNEL;
    config.trigger_count  = 1;
    config.hw_trigger_sel = ADC_HW_TIMER0_0;
    adc_config(&config);

    channel_config.callback = cb;
    channel_config.inp_gp   = ADC_CHANNEL_CHIP_TEMPERATURE;
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_16MHZ;
    //channel_config.count    = ADC_TEMPERATEURE_SAMPLE_COUNT;
    channel_config.gtune_gp = ADC_GTUNE_GP_4;
    channel_config.scal_gp  = ADC_SCAL_GP_DISABLE;
    channel_config.vcm_gp   = ADC_VCM_SEL_750mV;
    adc_add_channel(&channel_config);

    if (cb != NULL) return 0;
    
    int32_t sum = 0;
    int16_t reg_data = 0;
    uint32_t inp_gp = (1<<ADC_CHANNEL_CHIP_TEMPERATURE);
    
    for(int32_t i = 0; i < ADC_TEMPERATEURE_SAMPLE_COUNT; i++)
    {
        HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;
        while((HS_GPADC->INTR & inp_gp) != inp_gp);
        HS_GPADC->INTR = HS_GPADC->INTR;
        reg_data = (int16_t)(adc_channel_read_data(channel_config.inp_gp)&0x0000FFFF);
        sum += reg_data;
        //adc_log("sample= %08x, %08x\n", reg_data, sum);
    }
    adc_del_channel(ADC_CHANNEL_CHIP_TEMPERATURE);
    adc_stop();
    adc_env.state = ADC_STATE_READY;

    float vout = (float)sum/ADC_TEMPERATEURE_SAMPLE_COUNT*0.8;
    adc_log("adc_temperature_read sample= %f, %f\n", vout, adc_data2float(ADC_CHANNEL_CHIP_TEMPERATURE, vout));

    return (int16_t)adc_data2float(ADC_CHANNEL_CHIP_TEMPERATURE, vout);
}

int16_t adc_battery_voltage_read(adc_callback_t cb)
{
    adc_channel_config_t channel_config;
    adc_config_t config;
    adc_init();
    adc_start();
    
    config.trigger_mode   = ADC_TRIGGER_MODDE_SW;
    config.trigger_res    = ADC_TRIG_RES_CHANNEL;
    config.trigger_count  = 1;
    config.hw_trigger_sel = ADC_HW_TIMER0_0;
    adc_config(&config);

    channel_config.callback = cb;
    channel_config.inp_gp   = ADC_CHANNEL_CHIP_BATTERY;
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_2MHZ;
    //channel_config.count    = ADC_BATTERY_SAMPLE_COUNT;
    channel_config.gtune_gp = ADC_GTUNE_GP_0P5;
    channel_config.scal_gp  = ADC_SCAL_GP_DISABLE;
    channel_config.vcm_gp   = ADC_VCM_SEL_000mV;
    adc_add_channel(&channel_config);

    if (cb != NULL) return 0;

    int32_t sum = 0;
    int16_t reg_data = 0;
    uint32_t inp_gp = (1<<ADC_CHANNEL_CHIP_BATTERY);
    
    for(int32_t i = 0; i < ADC_BATTERY_SAMPLE_COUNT; i++)
    {
        HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;
        while((HS_GPADC->INTR & inp_gp) != inp_gp);
        HS_GPADC->INTR = HS_GPADC->INTR;
        reg_data = (int16_t)(adc_channel_read_data(channel_config.inp_gp)&0x0000FFFF);
        sum += reg_data;
        //adc_log("sample= %08x, %08x\n", reg_data, sum);
    }
    adc_del_channel(ADC_CHANNEL_CHIP_BATTERY);
    adc_stop();
    adc_env.state = ADC_STATE_READY;

    sum = sum/ADC_BATTERY_SAMPLE_COUNT;
    adc_log("adc_battery_voltage_read sample= %08x, %f\n", sum, adc_data2float(ADC_CHANNEL_CHIP_BATTERY, sum));

    return (int16_t)(adc_data2float(ADC_CHANNEL_CHIP_BATTERY, sum)*1000);
}

int16_t adc_battery_voltage_read_by_single_pin(adc_channel_t ch_p, adc_pga_gain_t gain, adc_callback_t cb, int sample_num)
{
    adc_channel_config_t channel_config;
    adc_config_t config;
    adc_init();
    adc_start();
    
    config.trigger_mode   = ADC_TRIGGER_MODDE_SW;
    config.trigger_res    = ADC_TRIG_RES_CHANNEL;
    config.trigger_count  = 1;
    config.hw_trigger_sel = ADC_HW_TIMER0_0;
    adc_config(&config);

    channel_config.callback = cb;
    channel_config.inp_gp   = ch_p;
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_2MHZ;
    //channel_config.count    = sample_num;
    channel_config.gtune_gp = s_adc_gain_table[gain].gtune_gp;
    channel_config.scal_gp  = s_adc_gain_table[gain].scal_gp;
    channel_config.vcm_gp   = ADC_VCM_SEL_000mV;
    adc_add_channel(&channel_config);

    if (cb != NULL) return 0;
    
    int32_t sum = 0;
    int16_t reg_data = 0;
    uint32_t inp_gp = (1<<ch_p);
    
    for(int32_t i = 0; i < sample_num; i++)
    {
        HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;
        while((HS_GPADC->INTR & inp_gp) != inp_gp);
        HS_GPADC->INTR = HS_GPADC->INTR;
        reg_data = (int16_t)(adc_channel_read_data(channel_config.inp_gp)&0x0000FFFF);
        sum += reg_data;
        //adc_log("sample= %08x, %08x\n", reg_data, sum);
    }
    adc_del_channel(channel_config.inp_gp);
    adc_stop();
    adc_env.state = ADC_STATE_READY;

    sum = sum/sample_num;
    adc_log("adc_battery_voltage_read sample= %08x, %f\n", sum, adc_data2float(channel_config.inp_gp, sum));

    return (int16_t)(adc_data2float(channel_config.inp_gp, sum)*1000);
}

int16_t adc_battery_voltage_read_by_dif_pin(adc_channel_t ch_p, adc_channel_t ch_n, adc_pga_gain_t gain, adc_callback_t cb, int sample_num)
{
    adc_channel_config_t channel_config;
    adc_config_t config;
    adc_init();
    adc_start();
    
    config.trigger_mode   = ADC_TRIGGER_MODDE_SW;
    config.trigger_res    = ADC_TRIG_RES_CHANNEL;
    config.trigger_count  = 1;
    config.hw_trigger_sel = ADC_HW_TIMER0_0;
    adc_config(&config);

    channel_config.callback = cb;
    channel_config.inp_gp   = ch_p;
    channel_config.inn_gp   = ch_n;
    channel_config.clk_sel  = ADC_CLK_SEL_2MHZ;
    //channel_config.count    = sample_num;
    channel_config.gtune_gp = s_adc_gain_table[gain].gtune_gp;
    channel_config.scal_gp  = s_adc_gain_table[gain].scal_gp;
    channel_config.vcm_gp   = ADC_VCM_SEL_000mV;
    adc_add_channel(&channel_config);

    if (cb != NULL) return 0;
    
    int32_t sum = 0;
    int16_t reg_data = 0;
    uint32_t inp_gp = (1<<ch_p);
    //adc_log("adc ch1_cfg= %08x\n", HS_GPADC->CH_1_CFG);
    
    for(int32_t i = 0; i < sample_num; i++)
    {
        HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;
        while((HS_GPADC->INTR & inp_gp) != inp_gp);
        HS_GPADC->INTR = HS_GPADC->INTR;
        reg_data = (int16_t)(adc_channel_read_data(channel_config.inp_gp)&0x0000FFFF);
        sum += reg_data;
        //adc_log("sample= %08x, %08x\n", reg_data, sum);
    }
    adc_del_channel(channel_config.inp_gp);
    adc_stop();
    adc_env.state = ADC_STATE_READY;

    sum = sum/sample_num;
    adc_log("adc_battery_voltage_read sample= %08x, %f\n", sum, adc_data2float(channel_config.inp_gp, sum));

    return (int16_t)(adc_data2float(channel_config.inp_gp, sum)*1000);
}

void adc_set_calibarate_param(adc_cal_table_t *cal_table)
{
    if (cal_table == NULL) return;
    
    memcpy(&adc_env.cal_table, cal_table, sizeof(adc_cal_table_t));
    adc_env.is_use = 1;
/*    
    pmu_ana_enable(true, PMU_ANA_ADC);
    volatile uint32_t* p_reg = &HS_GPADC->CMPN_S_VOS_G0_LUT_REG0;
    uint32_t *p_cal_table = (uint32_t *)(cal_table);
    for(uint8_t i = 0; i < 32; i = i + 3)
    {
        *(p_reg + ((i/3)*ADC_REG_STEP) + 0) = p_cal_table[i+0];
        *(p_reg + ((i/3)*ADC_REG_STEP) + 1) = p_cal_table[i+1];
        *(p_reg + ((i/3)*ADC_REG_STEP) + 2) = p_cal_table[i+2];
    }
    //adc_env.temp_calib.offset = cal_table->temp_calib_offset.offset;
    pmu_ana_enable(false, PMU_ANA_ADC);
*/
}

bool adc_is_running(void)
{
    if ( adc_env.state > ADC_STATE_READY)
    {
        return true;
    }
    else
    {
        return false;
    }
}
/** @} */


