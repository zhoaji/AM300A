/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: stim_control.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/
#include <stdbool.h>
#include "stim_control.h"
#include "bsp_gpio.h"
#include "handler.h"
#include "bsp_iic.h"
#include "bsp_timer.h"
#include "bsp_systick.h"
#include "emg_wave.h"

//Stim_status_Typedef stim_status;

Stim_control_Typedef stim_a_control;
Stim_control_Typedef stim_b_control;

Stim_parameter_Typedef stim_parameter =
{
	.frequency = 100,  		
	.pulse_width = 100,		 
	.rasetime = 10,				
	.stimtime = 5,				
	.falltime = 10,				
	.resttime = 10,				
};

/************************************************
	@Function			: pulse_parameter_set
	@Description	:	脉冲参数设置
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void pulse_parameter_set(void)
{
	stim_a_control.pw_50us_cnt = stim_parameter.pulse_width / 50;
	stim_a_control.pw_period_cnt = (1000000 / stim_parameter.frequency ) / 50;
	stim_a_control.period_time = stim_parameter.stimtime + stim_parameter.rasetime/10;
	
	stim_b_control.pw_50us_cnt = stim_parameter.pulse_width / 50;
	stim_b_control.pw_period_cnt = (1000000 / stim_parameter.frequency) / 50;
	stim_b_control.period_time = stim_parameter.stimtime + stim_parameter.rasetime/10;
}

/************************************************
	@Function			: set_stim_intensity_general
	@Description	:	设置刺激强度通用函数
	@parameter		: *sp , 参数结构体指针
									*pc , 输出控制结构体指针
	@Return				: None
	@Remark				: None
*/
void set_stim_intensity_general(uint8_t intensity, Stim_control_Typedef *pc)
{	
	pc->intensity = intensity;
	
	if(pc->stim_section == STIMTIME) // 处于刺激时间，刺激强度变更
		pc->intensity_dac = pc->intensity;
	
	pc->updata_rase_and_fall_time = TRUE;	// 强制刷新上升下降时间
	
	pc->intensity_changed_flag = TRUE;
	
	pc->period_time = stim_parameter.stimtime + (stim_parameter.rasetime / 10); // 更新剩余刺激输出周期时间
}

/************************************************
	@Function			: startup_stim_operation
	@Description	:	启动刺激操作
	@parameter		: *pc , 输出控制结构体指针				
	@Return				: 0x00 , 治疗开始
									0xF1 , 开始失败，参数未设置或刺激强度为0
	@Remark				: None
*/
uint8_t startup_stim_operation(Stim_control_Typedef *pc)
{
	uint8_t res = 0;
	
	if(pc->probe_status == LEAD_OFF) { return ERROR_ACK; } // 电极脱落
	
	if(pc->intensity > 0)
	{
		if(pc->stim_section != STIMTIME) pc->stim_section = RASETIME;
		pc->period_time = stim_parameter.stimtime + stim_parameter.rasetime / 10;
//		pc->pulse_low_time_cnt = 0;
	}
	else { res = ERROR_ACK; } // 刺激强度为0
	
//	stim_a_control.pulse_low_time_cnt = stim_a_control.pw_period_time / 2; // 待定
	
	gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_LOW); // 关放电电路
	gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_LOW);  	 // 开继电器
	
	return res;
}

/**************************************************************
	@Function 		: stim_lead_off_check
	@Parameter		: None
	@Description	: 刺激状态电极脱落判断
	@Return				: None
	@Remark				: None
*/
#define LEAD_OFF_CNT		7  	// 连续的10次检测里面有8次及以上次数都是脱落，则认为电极脱落
//#define LEAD_OFF				0x01	// 脱落
void stim_lead_off_check(uint8_t channel, Stim_control_Typedef *pc)
{
	static uint8_t index[CH_NUM] = {0};
	static uint16_t status_cnt[CH_NUM] = {0}; 
	
	if(gpio_read(BITMASK(PIN_STIM_OFF))) status_cnt[channel]++;
	
	if(++index[channel] >= LEAD_OFF_CNT)
	{
		index[channel] = 0;
		pc->probe_status = (status_cnt[channel] >= 5) ? LEAD_OFF : 0;
		status_cnt[channel] = 0;
		
		if(stim_a_control.probe_status) gpio_write(BITMASK(PIN_LED0), GPIO_HIGH);
		else gpio_write(BIT_MASK(PIN_LED0), GPIO_LOW);
		
	}
}

/**************************************************************
	@Function 		: stim_period_output_control
	@Parameter		: None							 
	@Description	: 刺激周期输出控制
	@Return				: None
	@Remark				: 放主循环
*/
void stim_period_output_control(void)
{
//	uint8_t status = 0;
	
	// 刺激时间结束，进入下降时间
	if(!stim_a_control.period_time && stim_a_control.stim_section) 
		stim_a_control.stim_section =  FALLTIME;
	if(!stim_b_control.period_time && stim_b_control.stim_section) 
		stim_b_control.stim_section = FALLTIME;

/*	
	// 非刺激状态下取消 刺激电极脱落报警
	if(!stim_a_control.stim_section ) { stim_a_control.probe_status &= ~LEAD_OFF; } 
	if(!stim_b_control.stim_section ) { stim_b_control.probe_status &= ~LEAD_OFF; }
	
	// 脱落情况，复位刺激周期时间，复位刺激阶段状态位
	if(stim_a_control.probe_status ) 
	{
		stim_a_control.period_time = 0; 
		stim_a_control.stim_section = STIMOVER; 
	}
	
	if(stim_b_control.probe_status)  
	{
		stim_b_control.period_time = 0; 
		stim_b_control.stim_section = STIMOVER; 
	}
	
	// AB通道均脱落，关继电器，开放电电路
	if(stim_a_control.probe_status && stim_b_control.probe_status)
	{
		gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_HIGH);  // 关继电器
		gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_HIGH); // 打开放电电路
	}
*/

/* 此处应按周期发送		
	if(stim_a_control.probe_status || stim_b_control.probe_status) //发送刺激状态
	{
		if(stim_a_control.stim_section) { status |= 0x01; }  // A通道刺激中
		if(stim_b_control.stim_section) { status |= 0x02; } // B通道刺激中
		stim_status_packet_send(status);  
	}
*/	
}


/**************************************************************
	@Function 		: stim_intensity_value_control
	@Parameter		: channel , 控制的通道
									*sp , 参数指针
									*pc , 脉冲控制指针
	@Description	: 脉冲上升下降时间的脉冲强度变化值计算
	@Return				: None
	@Remark				: 放主循环
*/
void stim_intensity_value_control( uint8_t channel, Stim_parameter_Typedef *sp, Stim_control_Typedef *pc )
{
	static uint8_t intensity_last_value[CH_NUM] = {0};
	static uint8_t stim_start_last_value[CH_NUM] = {0};
	uint8_t intensity_tmp;
	
	if( pc->start_in_half ) { intensity_tmp = 1;}
	else { intensity_tmp = (pc->intensity + 1)/ 2;}
	
	if(( stim_start_last_value[channel] != pc->stim_section ) || pc->updata_rase_and_fall_time)  // rise time  or  fall time
	{
		if( pc->stim_section == RASETIME) // rise time
		{
			if( sp->rasetime == 0 ) { pc->intensity_dac = pc->intensity;}
			else  // 上升时间不为 0
			{
				// 脉冲强度没有改变
				if((intensity_last_value[channel] == pc->intensity) || !intensity_last_value[channel])
				{
					pc->intensity_dac = intensity_tmp;
					pc->intensity_temp_dac = intensity_tmp * 100;
					
					pc->intensity_ampl_change = ( (pc->intensity - intensity_tmp)* 100 )/(sp->rasetime*2);  
//					printf("UP = %d\r\n",pc->intensity_ampl_change);
				}
				else // 脉冲强度发生了改变
				{
					// 上升时间还没结束的情况下 脉冲幅度值改变
					if( (sp->rasetime *2) > pc->stim_50ms_count ) 
					{
						pc->intensity_ampl_change = (pc->intensity - pc->intensity_dac )* 100 / (sp->rasetime*2 - pc->stim_50ms_count); 
					}
				}
			}
		}
		else if( pc->stim_section == FALLTIME )// fall time
		{
			if( sp->falltime == 0 ) { pc->intensity_dac = 0;}
			else  // 下降时间非0
 			{
				if( (sp->rasetime*2) == pc->stim_50ms_count )	
				{
					pc->intensity_temp_dac = pc->intensity * 100;
				}
				pc->intensity_ampl_change = ( (pc->intensity - intensity_tmp) * 100 )/(sp->falltime*2);
//				printf("DOWN = %d\r\n",pc->intensity_ampl_change);
			}
		}
		
		pc->intensity_changed_flag = TRUE;
	}
	
	stim_start_last_value[channel] = pc->stim_section;
	intensity_last_value[channel] = pc->intensity;
	
 	pc->updata_rase_and_fall_time = FALSE;  // 取消刷新上升下降时间标志位
}



/********************************************************************************
														 
							|<- stimtime ->|<-------------- a stim cycle ------------>|
							|______________|  											 _______________	|
						 /| 			 			 |\											  /							  \ |
						/	|							 | \            				 /								 \|
	_________/	|							 |	\___________________/									  \_______
					|   |							 |	 |
				rasetime					  falltime	
*/
/**************************************************************
	@Function 		: stim_intensity_output_control
	@Parameter		: None							 
	@Description	: 脉冲上升下降时间的脉冲强度输出值控制
	@Return				: None
	@Remark				: 50ms执行一次 ，放定时器中断
*/
void stim_intensity_output_control( Stim_parameter_Typedef *sp, Stim_control_Typedef *pc )
{	
	uint8_t intensity_tmp = 0;
	
	if( pc->intensity_changed_flag ) // 脉冲强度发生变化（含上升下降时间段）
	{
		if( pc->stim_section == RASETIME )  // start 
		{
			pc->stim_50ms_count++;
			if( pc->stim_50ms_count > sp->rasetime*2 ) { pc->stim_50ms_count = sp->rasetime*2;}
			
			if( pc->intensity_dac >= pc->intensity ) // 上升到顶
			{
				pc->intensity_dac = pc->intensity;
				pc->intensity_changed_flag = FALSE;
				pc->stim_50ms_count = sp->rasetime*2;
				pc->stim_section = STIMTIME;
			}
			else // 上升时间阶段
			{
				pc->intensity_temp_dac += pc->intensity_ampl_change;
				pc->intensity_dac = pc->intensity_temp_dac / 100;
				pc->intensity_changed_flag = TRUE;
			}
		}
		else if( pc->stim_section == FALLTIME ) // end
		{
			pc->stim_50ms_count = 0;
			
			if( pc->start_in_half ) { intensity_tmp = 1; }   
			else { intensity_tmp = (pc->intensity + 1) / 2; }
			
			if(pc->intensity_dac <= intensity_tmp )  // 下降到底
			{
				pc->intensity_dac = 0;
				pc->intensity_changed_flag = FALSE;
				pc->intensity_temp_dac = 0;
				pc->stim_section = STIMOVER;
//				pc->pulse_low_time_cnt = 0;  
				
//				if((stim_a_control.stim_section == STIMOVER) && (stim_b_control.stim_section == STIMOVER))
//				{ gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_HIGH); } // 关继电器
			}
			else  // 下降时间阶段
			{
				if( pc->intensity_temp_dac >= pc->intensity_ampl_change )
				{
					pc->intensity_temp_dac -= pc->intensity_ampl_change;
				}
				else 
				{
					pc->intensity_temp_dac = 0;
				}
				
				pc->intensity_dac = pc->intensity_temp_dac / 100;
			}
		}
	}
}

/**************************************************************
	@Function 		: single_pulse_control
	@Parameter		: None						 
	@Description	: 单个脉冲输出控制
	@Return				: None
	@Remark				: None
*/
uint8_t single_pulse_control(void)
{
	static pulse_step step = step_a_prepare;  // 脉冲产生控制步骤
	static uint16_t count_50us[CH_NUM]; 			// 
//	uint8_t freq_comp = 0; // 频率补偿

//	// 只有B通道输出
	if(( !stim_a_control.stim_section ) && (step == step_a_prepare)) step = step_b_prepare;
//	
//	if(stim_a_control.pulse_low_time_cnt) stim_a_control.pulse_low_time_cnt++;
//	if(stim_b_control.pulse_low_time_cnt) stim_b_control.pulse_low_time_cnt++;
	
	switch( step )
	{
		case step_a_prepare:	 // 脉冲输出前的准备	
			if(stim_a_control.intensity < 40) { gpio_write(BITMASK(PIN_STIM_PWM_L), GPIO_HIGH); }
			else { gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_HIGH); }
			gpio_write(BITMASK(PIN_STIM_OUT_A), GPIO_HIGH);  // 开启A通道输出使能光耦			
//			if( ++count_50us[CH_A] >= 4 )  // 200us delay time
//			{
				count_50us[CH_A] = 0;
				step = step_a_up; 
//			}
		
		break;
		
		case step_a_up:  // 脉冲时间
			if(!count_50us[CH_A])
			{
				dac_out_value_set(stim_a_control.intensity_dac);    // test intensity set.			
//				if(stim_a_control.intensity < 40) { gpio_write(BITMASK(PIN_STIM_PWM_L), GPIO_HIGH); }
//				else { gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_HIGH); }
			}
			
			if( ++count_50us[CH_A] >= stim_a_control.pw_50us_cnt )  // 计数脉宽
			{
				count_50us[CH_A] = 0;
				tim_arr_set(HS_TIM0, 50); 

				step = step_a_delay; 
			}
			stim_lead_off_check(CH_A, &stim_a_control); // 脱落检测
		break;

		case step_a_delay:  // 低电平时间1
			if(!count_50us[CH_A])
			{				
				tim_arr_set(HS_TIM0, 50);	
				gpio_write(BITMASK(PIN_STIM_OUT_A), GPIO_LOW); // 关闭光耦  
				gpio_write(BITMASK(PIN_STIM_PWM_L), GPIO_LOW);
				gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_LOW);
		
				dac_out_value_set(0);
			}
//			else if(count_50us[CH_A]==1)
//			{
//				tim_arr_set(HS_TIM0, 50);
//				stim_lead_off_check(CH_A, &stim_a_control); // 脱落检测
//			}
			else
			{
				if(stim_b_control.stim_section)
				{
					if( count_50us[CH_A] >= stim_a_control.pw_period_cnt / 2 - 19)
					{
						count_50us[CH_A] = 0;
						step = step_b_prepare;
					}
				}
				else 
				{
					if( count_50us[CH_A] >= stim_a_control.pw_period_cnt - 21)
					{
						count_50us[CH_A] = 0;
						step = step_a_prepare;
						
						return 1;
					}
				}			
			}
			count_50us[CH_A]++;
		break;
			
		case step_b_prepare:		
			if(stim_b_control.intensity < 40) { gpio_write(BITMASK(PIN_STIM_PWM_L), GPIO_HIGH); }
			else { gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_HIGH); }
			gpio_write(BITMASK(PIN_STIM_OUT_B), GPIO_HIGH);  // 开启A通道输出使能光耦			
//			if( ++count_50us[CH_A] >= 4 )  // 200us delay time
//			{
				count_50us[CH_A] = 0;
				step = step_b_up; 
//			}
		break;
		
		case step_b_up:	
			if(!count_50us[CH_A])
			{
				dac_out_value_set(stim_b_control.intensity_dac);    // test intensity set.			
//				if(stim_b_control.intensity < 40) { gpio_write(BITMASK(PIN_STIM_PWM_L), GPIO_HIGH); }
//				else { gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_HIGH); }
			}
			
			if( ++count_50us[CH_A] >= stim_b_control.pw_50us_cnt )  // 计数脉宽
			{
				count_50us[CH_A] = 0;
				tim_arr_set(HS_TIM0, 50); 

				step = step_b_delay; 
			}
			stim_lead_off_check(CH_B, &stim_b_control); // 脱落检测
		break;

		case step_b_delay:
			if(!count_50us[CH_A])
			{				
				tim_arr_set(HS_TIM0, 50);
				gpio_write(BITMASK(PIN_STIM_OUT_B), GPIO_LOW); // 关闭光耦  
				gpio_write(BITMASK(PIN_STIM_PWM_L), GPIO_LOW);
				gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_LOW);
				dac_out_value_set(0);
			}
//			else if(count_50us[CH_A]==1)
//			{
//				tim_arr_set(HS_TIM0, 50);
//				stim_lead_off_check(CH_B, &stim_a_control); // 脱落检测
//			}
			else
			{
					if( count_50us[CH_A] >= stim_b_control.pw_period_cnt / 2 - 19)
					{
						count_50us[CH_A] = 0;
						step = step_a_prepare;
//						if(!stim_a_control.stim_section && !stim_b_control.stim_section) 	
//							tim_stop(HS_TIM0); 		

//						gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_HIGH); // 放电
//				
//						if(emg_wave.emg_wave_en) 
//							gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_HIGH); // 切换到EMG				
						return 1;
					}		
			}
			count_50us[CH_A]++;
			break;

		default: break;
	}
	return 0;
}



/**************************************************************
	@Function 		: intensity_up_or_down
	@Parameter		: operation , 0:down  1:up 
	@Description	: 增加/减少刺激强度
	@Return				: None
	@Remark				: 此处应该要加上传刺激强度变更指令
*/
void intensity_up_or_down(uint8_t channel, uint8_t operation)
{
	if(operation)  // up
	{
		if(STIM_CH_A == (channel & STIM_CH_A)) 
		{
			if(++stim_a_control.intensity >= INTENSITY_MAX) stim_a_control.intensity = 0;
			stim_a_control.stim_section = RASETIME;
		}
		if(STIM_CH_B == (channel & STIM_CH_B))
		{
			if(++stim_b_control.intensity >= INTENSITY_MAX) stim_b_control.intensity = 0;
			stim_b_control.stim_section = RASETIME;
		}
	}
	else  // down
	{
		if(STIM_CH_A == (channel & STIM_CH_A)) 
		{
			if(stim_a_control.intensity > INTENSITY_MIN) stim_a_control.intensity--;
			if(!stim_a_control.intensity) stim_a_control.stim_section = FALLTIME;
			else stim_a_control.stim_section = RASETIME;
		}
		if(STIM_CH_B == (channel & STIM_CH_B))
		{
			if(stim_b_control.intensity > INTENSITY_MIN) stim_b_control.intensity--;
			if(!stim_b_control.intensity)stim_b_control.stim_section = FALLTIME;
			else stim_b_control.stim_section = RASETIME;
		}
	}
	
//	printf("A = %d\r\n", stim_a_control.intensity);
}

/**************************************************************
	@Function 		: stim_led_control
	@Parameter		: sw , 0:off  1:on
	@Description	: 刺激指示灯控制
	@Return				: None
	@Remark				: None
*/
void stim_led_control(uint8_t sw)
{
	if(sw)
	{
		if((stim_a_control.intensity < 16)||(stim_b_control.intensity < 16))
		{
			gpio_write(BITMASK(PIN_LED3), GPIO_LOW);
		}
		else if((stim_a_control.intensity < 31)||(stim_b_control.intensity < 31)) 
		{
			gpio_write(BITMASK(PIN_LED3), GPIO_LOW);
			gpio_write(BITMASK(PIN_LED1), GPIO_LOW);
		}
		else
		{
			gpio_write(BITMASK(PIN_LED3), GPIO_LOW);
			gpio_write(BITMASK(PIN_LED1), GPIO_LOW);
			gpio_write(BITMASK(PIN_LED2), GPIO_LOW);
		}
	}
	else
	{
		gpio_write(BITMASK(PIN_LED1), GPIO_HIGH);
		gpio_write(BITMASK(PIN_LED2), GPIO_HIGH);
		gpio_write(BITMASK(PIN_LED3), GPIO_HIGH);
	}
}

/**************************************************************
	@Function 		: stim_control_handler
	@Parameter		: None
	@Description	: 刺激输出控制相关处理
	@Return				: None
	@Remark				: None
*/
void stim_control_handler(void)
{
	
	stim_period_output_control();
	
	stim_intensity_value_control( CH_A, &stim_parameter, &stim_a_control );  // stim risetime & falltime control of A
	stim_intensity_value_control( CH_B, &stim_parameter, &stim_b_control );	
	
	// 此处应有周期发送电极状态
	/*----------?????------------*/
	
	
/* test	
	static uint32_t demo_stim_tick = 0;
	
	if((stim_a_control.stim_section == RASETIME)||(stim_a_control.stim_section == RASETIME))
	{
		demo_stim_tick = TICK_NOW;
		stim_a_control.stim_section = STIMTIME;
		stim_b_control.stim_section = STIMTIME;
		
		stim_led_control(1);
	}
	else if((stim_a_control.stim_section == STIMTIME)||(stim_a_control.stim_section == STIMTIME))
	{
		if(TICK_PASSED(TICK_NOW, demo_stim_tick) >= TICK_nS(6)) 
		{
			stim_a_control.stim_section = FALLTIME;
			stim_b_control.stim_section = FALLTIME;
		}
	}
	else if((stim_a_control.stim_section == FALLTIME)||(stim_a_control.stim_section == FALLTIME))
	{
		stim_a_control.stim_section = STIMOVER;
		stim_b_control.stim_section = STIMOVER;
		stim_led_control(0);
	}
*/	
}

/**************************************************************
	@Function 		: stim_control_handler
	@Parameter		: None
	@Description	: 刺激输出控制相关处理
	@Return				: None
	@Remark				: None
*/
void stim_init(void)
{
	// control data init
	memset(&stim_a_control, 0, sizeof(stim_a_control));
	memset(&stim_b_control, 0, sizeof(stim_b_control));

	stim_a_control.pw_50us_cnt = 4;
	stim_a_control.pw_period_cnt = 200;
	stim_a_control.period_time = 5;
	
	stim_b_control.pw_50us_cnt = 4;
	stim_b_control.pw_period_cnt = 200;
	stim_b_control.period_time = 5;
	
	stim_a_control.intensity = 10;
	stim_b_control.intensity = 10;
//		stim_a_control.intensity_dac = 10;
}


/**************************************************************
	@Function 		: stim_50us_server
	@Parameter		: None
	@Description	: 刺激输出控制
	@Return				: None
	@Remark				: None
*/
void stim_50us_server(void)
{
	static uint16_t tim_50ms_cnt = 0;
	static uint16_t tim_1s_cnt = 0;
	static uint16_t tim_500us_cnt = 0;
	
	if( ++tim_1s_cnt >= 20000 )   // 50us * 20000 = 1s
	{
		tim_1s_cnt = 0;
		if(stim_a_control.period_time && stim_parameter.stimtime != 99) stim_a_control.period_time--;
		if(stim_b_control.period_time && stim_parameter.stimtime != 99) stim_b_control.period_time--;
	}
	
	if(single_pulse_control()) // 波形
	{
		if(!stim_a_control.stim_section && !stim_b_control.stim_section) 	// 刺激周期输出结束
		{
			tim_stop(HS_TIM0); 		
		
			gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_HIGH); // 放电

			if(emg_wave.emg_wave_en) 
			{
				tim_start(HS_TIM1); 
				gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_HIGH); // 切换到EMG
			}
		}
	}
	
	if(++tim_50ms_cnt >= 975)  // 50ms  误差修正 1000 -> 975
	{
		tim_50ms_cnt = 0;
		
		stim_intensity_output_control( &stim_parameter, &stim_a_control );
		stim_intensity_output_control( &stim_parameter, &stim_b_control );
	}
	
	if(++tim_500us_cnt >= 10)
	{
		tim_500us_cnt = 0;
		if(emg_wave.emg_wave_en) 
		{
			QUEUE_WRITE(emg_a_raw_fifo, 0);	
			QUEUE_WRITE(emg_b_raw_fifo, 0);	
		}
	}
}

/**************************************************************
	@Function 		: start_stim
	@Parameter		: mode , 0: stop 1:start  pause: 2
	@Description	: 开始/停止/暂停刺激
	@Return				: None
	@Remark				: None
*/
void start_stim(uint8_t mode)
{
	switch(mode)
	{
		case 0x01: //START_OUTPUT:  
			if(stim_a_control.intensity)
			{
				if(stim_a_control.stim_section != STIMTIME) 
					stim_a_control.stim_section = RASETIME; 
				stim_a_control.period_time = stim_parameter.stimtime + stim_parameter.rasetime/10;   // reset stim period time
			}

/* 暂时不开B通道			
			if(stim_b_control.intensity)
			{
				if(stim_b_control.stim_section != STIMTIME) 
					stim_b_control.stim_section = RASETIME; 
				stim_b_control.period_time = stim_parameter.stimtime + stim_parameter.rasetime/10;   // reset stim period time
			}
*/		
			gpio_write(BITMASK(PIN_OFF_EN_OR_RELEASE), GPIO_LOW);
			gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_LOW); // 切换到Stim
//			emg_wave.emg_wave_en = 0; // 停止EMG的活动
			tim_stop(HS_TIM1);
			tim_start(HS_TIM0);
		break;
		
		case 0x00://STOP_OUTPUT:
				stim_a_control.period_time = 0; // stim_A_control.stim_status = FALLTIME;
				stim_b_control.period_time = 0; // stim_B_control.stim_status = FALLTIME;
		break;
	}
}


