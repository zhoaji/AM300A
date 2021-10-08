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

#define STIMOVER	0x00   	// 刺激结束阶段
#define RASETIME	0x01	 	// 上升时间阶段
#define STIMTIME	0x02		// 刺激输出阶段
#define FALLTIME	0x03		// 下降时间阶段

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
	
	uint8_t intensity;	// 刺激强度
	uint16_t intensity_dac; // DAC刺激强度 单位：mA
	
	uint16_t intensity_ampl_change;  // 刺激强度变化的幅度（x100）
	uint16_t intensity_temp_dac;		 // 计算得出的当前送入DAC的值（x100）
	
	uint8_t intensity_changed_flag;	 // 刺激强度变更标志位（如重新设置了刺激强度）
	
	uint16_t stim_50ms_count;				 // 上升下降时间每50ms的计数（每50ms变化一次）
	
//	uint16_t pulse_low_time_cnt;		 // 脉冲低电平时间计数（用于辅助控制下个脉冲输出时间）
	
	uint8_t	start_in_half;					// 是否以刺激强度的1/2为第1个脉冲的刺激强度
	
	uint8_t updata_rase_and_fall_time;  // 刷新上升下降时间使能位
	
	uint16_t period_time;						// 刺激周期时间，单位：秒
	
	uint16_t pw_period_time;					// 脉冲周期时间，单位：50us
	
	uint16_t pw_period_cnt;				// 脉冲周期时间计数，单位：50us
	
	uint16_t pw_50us_cnt;						// 单脉冲脉宽时间计数，单位：50us
	
	uint8_t stim_section;						// 脉冲输出阶段
	
	uint8_t probe_status;			// 0x00:正常  0x01:脱落
	
}Stim_control_Typedef;


typedef struct{
	uint16_t frequency;  		// 脉冲频率 1~120Hz   步进 1		暂定
	uint16_t pulse_width;		// 脉冲脉宽	50~450us	步进 50		暂定 
	uint8_t rasetime;				// 上升时间	0~18.0s		步进 0.1	暂定
	uint8_t	stimtime;				// 刺激时间	0~60s			步进 1		暂定
	uint8_t falltime;				// 下降时间	0~18.0s		步进 0.1	暂定
	uint8_t	resttime;				// 休息时间	0~120s		步进 1		暂定	
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
