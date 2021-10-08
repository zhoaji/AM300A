
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_adc.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_adc.h"

void bsp_adc_config(void)
{
	adc_init();

}

uint8_t bsp_adc_add_channel(adc_channel_config_t *channel)
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

    return true;
}

uint8_t bsp_adc_del_channel(adc_channel_t inp_gp)
{
    uint16_t cur_inp_gp = register_get(&HS_GPADC->ADC_CFG2, MASK_POS(GPADC_SEQ_VECT));
    cur_inp_gp &= ~(1<<inp_gp);
    register_set(&HS_GPADC->ADC_CFG2, MASK_1REG(GPADC_SEQ_VECT, cur_inp_gp));

    return true;
}

int16_t Get_sample_adc(adc_channel_t ch_p, adc_callback_t cb)
{
    adc_channel_config_t channel_config;
//    adc_config_t config;
//    adc_init();
//    adc_start();
    
//    config.trigger_mode   = ADC_TRIGGER_MODDE_SW;
//    config.trigger_res    = ADC_TRIG_RES_CHANNEL;
//    config.trigger_count  = 1;
//    config.hw_trigger_sel = ADC_HW_TIMER0_0;
//    adc_config(&config);
		
		pmu_ana_enable(true, PMU_ANA_ADC);
	
    channel_config.callback = cb;
    channel_config.inp_gp   = ch_p;
    channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
    channel_config.clk_sel  = ADC_CLK_SEL_16MHZ;
    //channel_config.count    = sample_num;
    channel_config.gtune_gp = ADC_GTUNE_GP_0P5; //s_adc_gain_table[gain].gtune_gp;
    channel_config.scal_gp  = ADC_SCAL_GP_ENABLE;//s_adc_gain_table[gain].scal_gp;
    channel_config.vcm_gp   = ADC_VCM_SEL_000mV;
    bsp_adc_add_channel(&channel_config);

    if (cb != NULL) return 0;
    
//    int32_t sum = 0;
    int16_t reg_data = 0;
    uint32_t inp_gp = (1<<ch_p);
    
//    for(int32_t i = 0; i < sample_num; i++)
//    {
        HS_GPADC->ADC_CFG0 = GPADC_ADC_START_MASK;
        while((HS_GPADC->INTR & inp_gp) != inp_gp);
        HS_GPADC->INTR = HS_GPADC->INTR;
        reg_data =(int16_t) *(uint32_t*)(HS_GPADC_BASE + 4 * (0x1C + channel_config.inp_gp)); //(int16_t)(adc_channel_read_data(channel_config.inp_gp)&0x0000FFFF);
//        sum += reg_data;
        //adc_log("sample= %08x, %08x\n", reg_data, sum);
//    }
    bsp_adc_del_channel(channel_config.inp_gp);
//    adc_stop();
		pmu_ana_enable(false, PMU_ANA_ADC);
//    adc_env.state = ADC_STATE_READY;

//    sum = sum/sample_num;
//    adc_log("adc_battery_voltage_read sample= %08x, %f\n", sum, adc_data2float(channel_config.inp_gp, sum));

//		log_debug("Voltage = %F\n", (float)reg_data*1000*0.8/2048.0);

    return reg_data;
}


void adc_sample_one_channel_irq(adc_channel_t ch_p, adc_callback_t cb)
{
	adc_channel_config_t channel_config;
	adc_config_t config;
	adc_init_irq();
	adc_start();
	
	config.trigger_mode   = ADC_TRIGGER_MODDE_SW;
	config.trigger_res    = ADC_TRIG_RES_CHANNEL;
	config.trigger_count  = 1;
	config.hw_trigger_sel = ADC_HW_TIMER0_0;
	adc_config(&config);

	channel_config.callback = cb;
	channel_config.inp_gp   = ch_p;
	channel_config.inn_gp   = ADC_CHANNEL_CHIP_VCM;
	channel_config.clk_sel  = ADC_CLK_SEL_16MHZ;
	//channel_config.count    = sample_num;
	channel_config.gtune_gp = ADC_GTUNE_GP_0P5;//s_adc_gain_table[gain].gtune_gp;
	channel_config.scal_gp  = ADC_SCAL_GP_ENABLE;//s_adc_gain_table[gain].scal_gp;
	channel_config.vcm_gp   = ADC_VCM_SEL_000mV;
	adc_add_channel(&channel_config);
	adc_start_convert();
}



