
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: emg_wave.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/
#include "algorithm.h"
#include "emg_wave.h"
#include "bsp_adc.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "handler.h"
#include "stim_control.h"

#define EMG_OFF_AD_CH		ADC_CHANNEL_EXTERN_CH5

int16_t ref_off_buf[EMG_OFF_BUF_LEN];
QUEUE_S16	ref_off_fifo;

int16_t emg_a_off_p_buf[EMG_OFF_BUF_LEN];
QUEUE_S16	emg_a_off_p_fifo;

int16_t emg_a_off_n_buf[EMG_OFF_BUF_LEN];
QUEUE_S16	emg_a_off_n_fifo;

int16_t emg_b_off_p_buf[EMG_OFF_BUF_LEN];
QUEUE_S16	emg_b_off_p_fifo;

int16_t emg_b_off_n_buf[EMG_OFF_BUF_LEN];
QUEUE_S16	emg_b_off_n_fifo;

uint16_t emg_a_raw_buf[EMG_BUF_LEN];
QUEUE_U16	emg_a_raw_fifo;

uint16_t emg_b_raw_buf[EMG_BUF_LEN];
QUEUE_U16	emg_b_raw_fifo;

//uint16_t emg_fifter_buf[EMG_BUF_LEN];
//QUEUE_U16	emg_fifter_fifo;

EMG_Typedef emg_wave;

/************************************************
	@Function			: get_emg_lead_off_adc_value
	@Description	:	采集EMG电极状态数据
	@parameter		: ch , EMG电极状态ADC采集通道
	@Return				: None
	@Remark				: None
*/
static void change_emg_off_ad_channel(EMG_LEAD_OFF_CH ch)
{
//	gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_LOW);
//	for(i=0;i<15;i++) __asm("nop");
	if(((uint8_t)ch & 0x4) >> 2) gpio_write(BITMASK(PIN_OFF_CHC), GPIO_LOW);
	else gpio_write(BITMASK(PIN_OFF_CHC), GPIO_HIGH);
	if(((uint8_t)ch & 0x2) >> 1) gpio_write(BITMASK(PIN_OFF_CHB), GPIO_LOW);
	else gpio_write(BITMASK(PIN_OFF_CHB), GPIO_HIGH);
	if(((uint8_t)ch & 0x1) >> 0) gpio_write(BITMASK(PIN_OFF_CHA), GPIO_LOW);
	else gpio_write(BITMASK(PIN_OFF_CHA), GPIO_HIGH);
//	for(i=0;i<15;i++) __asm("nop");
//	gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_HIGH);
}

/************************************************
	@Function			: emg_lead_off_adc_sample_handler
	@Description	:	EMG电极状态ADC采样回调函数
	@parameter		: exent , 回调事件
	@Return				: 采集到的ADC值
	@Remark				: None
*/
static void emg_lead_off_adc_sample_handler(adc_event_t exent)
{
	static EMG_LEAD_OFF_CH channel = REF_OFF_CH;
	static int16_t adc_value = 0;
	
	gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_HIGH);
	
	adc_value = (int16_t)(adc_channel_read_data(EMG_OFF_AD_CH)&0x0000FFFF);

//	battery.voltage_adc_mv = (uint16_t)((battery.adc_value * 0.8 / 2048.0) * 1000);
//	battery.voltage_bat_mv = battery.voltage_adc_mv * 2;
//	battery.vol_level = 3;  // 0 ~ 3 level 
	
	adc_del_channel(EMG_OFF_AD_CH);
	
	if(++channel >= EMG_OFF_CH_NUM) channel = REF_OFF_CH;
	switch(channel)
	{
		case REF_OFF_CH: 			QUEUE_WRITE(ref_off_fifo, adc_value); change_emg_off_ad_channel(EMG_A_OFF_P_CH);break;
		case EMG_A_OFF_P_CH:	QUEUE_WRITE(emg_a_off_p_fifo, adc_value); change_emg_off_ad_channel(EMG_A_OFF_N_CH);break;
		case EMG_A_OFF_N_CH:	QUEUE_WRITE(emg_a_off_n_fifo, adc_value); change_emg_off_ad_channel(EMG_B_OFF_P_CH);break;
		case EMG_B_OFF_P_CH:	QUEUE_WRITE(emg_b_off_p_fifo, adc_value); change_emg_off_ad_channel(EMG_B_OFF_N_CH);break;
		case EMG_B_OFF_N_CH:	QUEUE_WRITE(emg_b_off_n_fifo, adc_value); change_emg_off_ad_channel(REF_OFF_CH);break;
		default:break;
	}	
}

/************************************************
	@Function			: emg_lead_off_check
	@Description	:	EMG电极状态检测
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
static void emg_lead_off_check(void)
{
	
}

/************************************************
	@Function			: get_emg_lead_off_adc_value
	@Description	:	采集EMG电极状态数据
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void get_emg_lead_off_adc_value(void)
{
	gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_LOW);
	adc_sample_one_channel_irq(EMG_OFF_AD_CH, (adc_callback_t)emg_lead_off_adc_sample_handler);
}


/************************************************
	@Function			: get_emg_raw_adc_value
	@Description	:	采集EMG电极状态数据
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void get_emg_raw_adc_value(void)
{
	static EMG_CH channel = EMG_CH_A;
	static uint16_t data = 0;
	
	if(++channel >= EMG_CH_NUM) channel = EMG_CH_A;
	
	data = spi_read();
	
	switch(channel)
	{
		case EMG_CH_A: 
			QUEUE_WRITE(emg_a_raw_fifo, data);			
			gpio_write(BITMASK(PIN_EMG_CH_SW), GPIO_HIGH);  // change to B channel
			break;
		
		case EMG_CH_B: 
			QUEUE_WRITE(emg_b_raw_fifo, data);
			gpio_write(BITMASK(PIN_EMG_CH_SW), GPIO_LOW);  // change to A channel
			break;
		
		default: break;
	}
}

/************************************************
	@Function			: emg_algorithm_handler
	@Description	:	EMG相关算法处理函数
	@parameter		: channel , EMG通道
									fifo , 数据缓存
	@Return				: None
	@Remark				: None
*/
static void emg_algorithm_handler(EMG_CH channel, QUEUE_U16 *fifo)
{
	uint16_t dat_tmp = 0;
	
	uint32_t len = QUEUE_STOCK_P(fifo);
	
	if(len)
	{
		switch(emg_wave.detector_type)  
		{
			case PP_MAX_DETECTOR:  // 峰值最大值检波
				for(uint16_t i = 0; i < len; i++)	
					dat_tmp = EMG_arithmetic_average(channel, QUEUE_READ_P(fifo));	
				break;
			
			case PP_AVG_DETECTOR:  // 峰值平均值检波 
				for(uint16_t i = 0; i < len; i++) 
					dat_tmp = EMG_arithmetic_RMS(channel, QUEUE_READ_P(fifo));
				break;
			
			case RMS_DETECTOR:	// 均方根检波
				for(uint16_t i = 0; i < len; i++) 
					dat_tmp = emg_arithmetic_pp(channel, QUEUE_READ_P(fifo));
				break;
			
			default:break;
		}
		
//		if(channel == EMG_CH_A) gpio_write(BITMASK(8), GPIO_LOW);
//		else gpio_write(BITMASK(8), GPIO_HIGH);
		
		if(dat_tmp == 0xFFFF) return;
		
//		printf("%d - %d\r\n", channel, dat_tmp);
		
		if(EMG_CH_A == channel) emg_wave.emg_a = dat_tmp;
		else if(EMG_CH_B == channel) 
		{
			emg_wave.emg_b = dat_tmp;
			
//			if(stim_a_control.stim_section || stim_b_control.stim_section)
//			{
//				emg_wave_packet_send(0x9999, 0x9999);
//			}
//			else
//			{
				// send emg value cmd
				if(emg_wave.emg_wave_en == 1) 
					emg_wave_packet_send(emg_wave.emg_a, emg_wave.emg_b);
//			}
		}
	}
}

/************************************************
	@Function			: debug_emg_send_raw_data
	@Description	:	上传原始数据
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void debug_emg_send_raw_data(EMG_CH channel, QUEUE_U16 *fifo)
{
	uint16_t data = 0;
	static uint8_t buff[10] = {0};
	
	uint32_t len = QUEUE_STOCK_P(fifo);
	if(len < 10) return;
	
	for(uint16_t i = 0; i < (uint8_t)(len / 10); i++)	
	{
		for(uint8_t j = 0; j < 5; j++)
		{
			data = QUEUE_READ_P(fifo);
			buff[j * 2] = (uint8_t)(data >> 8);
			buff[j * 2 + 1] = (uint8_t)data;
		}
		
		emg_org_wave_data_packet_send(buff);
	}
}

/************************************************
	@Function			: emg_calculate_handler
	@Description	:	计算EMG值
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void emg_calculate_handler(void)
{
	if(stim_a_control.stim_section || stim_b_control.stim_section)
		return;
	
	if(!emg_wave.emg_wave_org_en)
	{
		emg_algorithm_handler(EMG_CH_A, &emg_a_raw_fifo);
		emg_algorithm_handler(EMG_CH_B, &emg_b_raw_fifo);
		
		emg_lead_off_check();  
	}
	else
	{
		debug_emg_send_raw_data(EMG_CH_A, &emg_a_raw_fifo);
//		debug_emg_send_raw_data(EMG_CH_B, &emg_b_raw_fifo);
	}
}

/************************************************
	@Function			: emg_init
	@Description	:	EMG相关数据初始化
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void emg_init(void)
{
	// fifo init
	QUEUE_INIT(emg_a_raw_fifo, emg_a_raw_buf, EMG_BUF_LEN);
	QUEUE_INIT(emg_b_raw_fifo, emg_b_raw_buf, EMG_BUF_LEN);
	QUEUE_INIT(ref_off_fifo, ref_off_buf, EMG_OFF_BUF_LEN);
	QUEUE_INIT(emg_a_off_p_fifo, emg_a_off_p_buf, EMG_OFF_BUF_LEN);
	QUEUE_INIT(emg_a_off_n_fifo, emg_a_off_n_buf, EMG_OFF_BUF_LEN);
	QUEUE_INIT(emg_b_off_p_fifo, emg_b_off_p_buf, EMG_OFF_BUF_LEN);
	QUEUE_INIT(emg_b_off_n_fifo, emg_b_off_n_buf, EMG_OFF_BUF_LEN);
	
	// sttaus init
	memset(&emg_wave, 0, sizeof(emg_wave));
	
}




