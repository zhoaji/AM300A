/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: stim_control.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __STIM_CON_H__
#define __STIM_CON_H__

#include <stdint.h>

#define FALSE			0x00
#define TRUE			0x01

#define STIMOVER	0x00   	// �̼������׶�
#define RASETIME	0x01	 	// ����ʱ��׶�
#define STIMTIME	0x02		// �̼�����׶�
#define FALLTIME	0x03		// �½�ʱ��׶�

#define FREQUENCY_MIN			1		
#define FREQUENCY_MAX			120

#define PULSE_WIDTH_MIN		50		
#define PULSE_WIDTH_MAX		450

#define RASETIME_MIN			0
#define RASETIME_MAX			180

#define FALLTIME_MIN			0
#define FALLTIME_MAX			180

#define STIMTIME_MIN			0
#define STIMTIME_MAX			60
#define STIMTIME_UNLIMIT 	99

#define RESTTIME_MIN			0
#define RESTTIME_MAX			120

#define INTENSITY_MIN			0
#define INTENSITY_MAX			90
	
#define LEAD_OFF				0x01
//#define B_LEAD_OFF				0x02
//#define AB_LEAD_OFF				0x03

#define STIM_CH_A		0x01
#define STIM_CH_B		0x02
#define STIM_CH_AB	0x03

typedef enum {
	CH_A = 0,
	CH_B,
	CH_NUM,
}STIM_CH;

typedef enum {
	step_a_prepare = 0,
	step_a_up,
	step_a_delay,
	step_b_prepare,
	step_b_up,
	step_b_delay,
	step_over
}pulse_step;

typedef enum {
	step_prepare_a = 0,
	step_up_a,
	step_delay_a,
	step_prepare_b,
	step_up_b,
	step_delay_b,
	step_stop
}double_pulse_step;

typedef struct{
	
	uint8_t intensity;	// �̼�ǿ��
	uint16_t intensity_dac; // DAC�̼�ǿ�� ��λ��mA
	
	uint16_t intensity_ampl_change;  // �̼�ǿ�ȱ仯�ķ��ȣ�x100��
	uint16_t intensity_temp_dac;		 // ����ó��ĵ�ǰ����DAC��ֵ��x100��
	
	uint8_t intensity_changed_flag;	 // �̼�ǿ�ȱ����־λ�������������˴̼�ǿ�ȣ�
	
	uint16_t stim_50ms_count;				 // �����½�ʱ��ÿ50ms�ļ�����ÿ50ms�仯һ�Σ�
	
//	uint16_t pulse_low_time_cnt;		 // ����͵�ƽʱ����������ڸ��������¸��������ʱ�䣩
	
	uint8_t	start_in_half;					// �Ƿ��Դ̼�ǿ�ȵ�1/2Ϊ��1������Ĵ̼�ǿ��
	
	uint8_t updata_rase_and_fall_time;  // ˢ�������½�ʱ��ʹ��λ
	
	uint16_t period_time;						// �̼�����ʱ�䣬��λ����
	
	uint16_t pw_period_time;					// ��������ʱ�䣬��λ��50us
	
	uint16_t pw_period_cnt;				// ��������ʱ���������λ��50us
	
	uint16_t pw_50us_cnt;						// ����������ʱ���������λ��50us
	
	uint8_t stim_section;						// ��������׶�
	
	uint8_t probe_status;			// 0x00:����  0x01:����
	
}Stim_control_Typedef;


typedef struct{
	uint16_t frequency;  		// ����Ƶ�� 1~120Hz   ���� 1		�ݶ�
	uint16_t pulse_width;		// ��������	50~450us	���� 50		�ݶ� 
	uint8_t rasetime;				// ����ʱ��	0~18.0s		���� 0.1	�ݶ�
	uint8_t	stimtime;				// �̼�ʱ��	0~60s			���� 1		�ݶ�
	uint8_t falltime;				// �½�ʱ��	0~18.0s		���� 0.1	�ݶ�
	uint8_t	resttime;				// ��Ϣʱ��	0~120s		���� 1		�ݶ�	
}Stim_parameter_Typedef;


//extern Stim_status_Typedef stim_status;

extern Stim_control_Typedef stim_a_control;
extern Stim_control_Typedef stim_b_control;

extern Stim_parameter_Typedef stim_parameter;

void pulse_parameter_set(void);

void set_stim_intensity_general(uint8_t intensity, Stim_control_Typedef *pc);
uint8_t startup_stim_operation(Stim_control_Typedef *pc);

void intensity_up_or_down(uint8_t channel, uint8_t operation);

void stim_control_handler(void);

void stim_init(void);

void stim_50us_server(void);

void start_stim(uint8_t mode);
#endif
