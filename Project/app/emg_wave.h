
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: emg_wave.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __EMG_WAVE_H__
#define __EMG_WAVE_H__

#include <stdint.h>
#include <stdbool.h> 
#include "queue.h"

#define EMG_OFF_BUF_LEN		64
//extern int16_t ref_off_buf[EMG_OFF_BUF_LEN];
//extern QUEUE_S16	ref_off_fifo;

//extern int16_t emg_a_off_p_buf[EMG_OFF_BUF_LEN];
//extern QUEUE_S16	emg_a_off_p_fifo;

//extern int16_t emg_a_off_n_buf[EMG_OFF_BUF_LEN];
//extern QUEUE_S16	emg_a_off_n_fifo;

//extern int16_t emg_b_off_p_buf[EMG_OFF_BUF_LEN];
//extern QUEUE_S16	emg_b_off_p_fifo;

//extern int16_t emg_b_off_n_buf[EMG_OFF_BUF_LEN];
//extern QUEUE_S16	emg_b_off_n_fifo;

#define EMG_BUF_LEN			256
//extern uint16_t emg_a_raw_buf[EMG_BUF_LEN];
extern QUEUE_U16	emg_a_raw_fifo;

//extern uint16_t emg_b_raw_buf[EMG_BUF_LEN];
extern QUEUE_U16	emg_b_raw_fifo;

//extern uint16_t emg_fifter_buf[EMG_BUF_LEN];
//extern QUEUE_U16	emg_fifter_fifo;

#define PP_MAX_DETECTOR		0x00
#define PP_AVG_DETECTOR		0x01
#define RMS_DETECTOR			0x02

typedef struct{
	uint8_t emg_wave_en;
	uint8_t emg_wave_org_en;
	
	uint8_t probe_status;	// 0x01: A通道脱落		0x02:B通道脱落   0x03: AB通道脱落
	
	uint16_t emg_a;				// A通道EMG数据
	uint16_t emg_b;				// B通道EMG数据
	uint16_t raw_emg_a;		// A通道EMG原始数据
	uint16_t raw_emg_b;		// B通道EMG原始数据
	
	uint8_t detector_type;			// 检波类型
	
}EMG_Typedef;

typedef enum {
	REF_OFF_CH = 0,  	// REF_OFF
	EMG_A_OFF_P_CH,			// EMG_A_OFF+
	EMG_A_OFF_N_CH,			// EMG_A_OFF-
	EMG_B_OFF_P_CH,			// EMG_B_OFF+
	EMG_B_OFF_N_CH,			// EMG_B_OFF-
	EMG_OFF_CH_NUM,
}EMG_LEAD_OFF_CH;

typedef enum {
	EMG_CH_A = 0,  	
	EMG_CH_B,			
	EMG_CH_NUM,			
}EMG_CH;

extern EMG_Typedef emg_wave;

void get_emg_lead_off_adc_value(void);
void get_emg_raw_adc_value(void);
void emg_calculate_handler(void);
void emg_init(void);
#endif
