/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: algorithm.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "algorithm.h"
#include "fifter.h"
#include <math.h>

#define EMG_Coefficient_Default		1000

enum __Channel{
	EMG_A,
	EMG_B,
	EMG_CHn,
};

volatile uint16_t  EMG_coefficient_PP[EMG_CHn] = { EMG_Coefficient_Default, EMG_Coefficient_Default};
volatile uint16_t  EMG_coefficient_avg[EMG_CHn] = { EMG_Coefficient_Default, EMG_Coefficient_Default};
volatile uint16_t  EMG_coefficient_rms[EMG_CHn] = { EMG_Coefficient_Default, EMG_Coefficient_Default};


/*******************************************************
	@Function			: Calculated_dc_component
	@Parameter 		: ch , 通道编号
									dat , 肌电数据
	@Description	: 求直流分量（偏置电压）
	@Return				: None
	@Remark				: 100个值的递推平均
*/
static uint16_t Calculated_dc_component ( uint8_t ch, uint16_t dat ) 
{
	static uint32_t sum[EMG_CHn] = {0};       // 累加值
	static uint16_t init_done[EMG_CHn] = {0};	// 数值的计算
	static uint16_t old_buff[EMG_CHn][257];   // 数据缓存
	static uint16_t index[EMG_CHn] = {0};			// 100个数据缓存的指针
	uint16_t DC_component = 0;			// 直流分量
	
	sum[ch] += dat;  
	
	old_buff[ch][index[ch]] = dat;  // FIFO
	if(++index[ch] > 256) index[ch] = 0;  
	
	if(init_done[ch] < 256) 	init_done[ch]++; // 更新总和
	else 	sum[ch] -= old_buff[ch][index[ch]];
	
	if(init_done[ch] >= 256) 	DC_component = sum[ch] >> 8;	 // 计算直流分量
	else DC_component = sum[ch] / init_done[ch];
	
	return DC_component;
}

/*******************************************************
	@Function			: smooth_handler
	@Parameter 		: ch , 通道编号
									EMG_dat , 肌电数据
	@Description	: 平滑处理
	@Return				: None
	@Remark				: None
*/
static uint16_t smooth_handler ( uint8_t ch, uint16_t EMG_dat )
{
	static uint8_t EMG_value_pcount[EMG_CHn] = {0};	// 正极性计数
	static uint8_t EMG_value_ncount[EMG_CHn] = {0};	// 负极性计数	
	static uint16_t EMG_value[EMG_CHn] = {0};
	
	if (EMG_dat > 1)	EMG_dat--;
	if ((EMG_dat - EMG_value[ch]) == 1)
	{
		EMG_value_ncount[ch] = 0;	
		if (++EMG_value_pcount[ch] > 3)
		{
			EMG_value[ch] = EMG_dat;
			EMG_value_pcount[ch] = 0;
		}
	}	  
	else        
	{
		if ((EMG_value[ch] - EMG_dat) == 1)
		{
			EMG_value_pcount[ch] = 0;
			if (++EMG_value_ncount[ch] > 1)
			{
				EMG_value[ch] = EMG_dat;
				EMG_value_ncount[ch] = 0;
			}
		}
		else
		{
			EMG_value[ch] = EMG_dat;
			EMG_value_pcount[ch] = 0;
			EMG_value_ncount[ch] = 0;
		}
	}
	
	return EMG_value[ch];
}

/*******************************************************
	@Function			: emg_arithmetic_pp
	@Parameter 		: EMG_data , 肌电原始数据
	@Description	: 峰值最大值检波
	@Return				: 最终肌电值
	@Remark				: None	
*/
#define  clk_50Hz_value	  40   // 2KHz   
#define	 clk_10Hz_value   4		 
uint16_t emg_arithmetic_pp ( uint8_t ch, uint16_t EMG_org ) 
{	
  uint8_t  i,j;
	
  uint16_t EMG_present;				 // 当前EMG值
	uint16_t DC_offset;					 // 直流分量
	uint16_t EMG_after_filter;   // 滤波后的数据
	uint16_t EMG_tmp;						 // 计算用的中间变量
	uint16_t result = 0xFFFF;				 // 最终结果
	
	static uint8_t  clk_50Hz_cnt[EMG_CHn] = {0};	 			// 50Hz计数
	static uint8_t  clk_10Hz_cnt[EMG_CHn] = {0}; 	 			// 10Hz计数
	static uint8_t  EMG_direction[EMG_CHn] = {0};	 			// 峰值标志位
	static uint8_t  array_count[EMG_CHn] = {0};   			// 数组元素个数计数
	static uint16_t EMG_previous[EMG_CHn] = {0};				// 上一个EMG值
	static uint16_t EMG_max[EMG_CHn] = {0};							// 最大值
	static uint16_t EMG_peak[EMG_CHn] = {0};						// 峰值
	static uint16_t	EMG_array[EMG_CHn][clk_10Hz_value]; // 最大值排序数组
	
  EMG_after_filter = Filter_Bandstop_50_100_150Hz_Sampling_2000Hz( EMG_org, ch ); // 带阻滤波

//	if(Flag.byte.EMG_filter_en == (ch + 1))
//	{
//			QUEUE_WRITE(EMG_FILTER_Dat, EMG_after_filter);
//			return;
//	}
	
	EMG_present = EMG_after_filter;  
	
	DC_offset = Calculated_dc_component( ch, EMG_after_filter );

	// 减去直流分量
	if( EMG_present >= DC_offset ) EMG_present -= DC_offset;  
	else  EMG_present = DC_offset - EMG_present;			

	if (EMG_present > EMG_max[ch])	{ EMG_max[ch] = EMG_present; }  // 当前值大于最大值
	if (EMG_present > EMG_previous[ch])	{ EMG_previous[ch] = EMG_present; EMG_direction[ch] = 1; } // 当前值大于前一个值
	else
	{		
/* 以下是取最大值的算法,如果没有峰值,取最大值 */
		if (EMG_present < EMG_previous[ch]) // 当前值小于前一个值
		{				
			if (EMG_direction[ch] != 0)  // 到达极峰顶
			{
				EMG_direction[ch] = 0;	// 切换极性
				if (EMG_peak[ch] < EMG_previous[ch])  
				{
					EMG_peak[ch]= EMG_previous[ch]; // 最大值小于前一个值(即当前峰值)
//					if(ch) printf("%d\r\n",EMG_peak[ch]);
				}
			}
			EMG_previous[ch] = EMG_present;	// 更新前一个值buff			  
		}
	}
	
	if (++clk_50Hz_cnt[ch] >= clk_50Hz_value)  // 窗口宽度 S_clk_value  
	{	
		clk_50Hz_cnt[ch] = 0;
		if (EMG_peak[ch] != 0 )	{ EMG_tmp = EMG_peak[ch]; } // 有峰值取峰值
		else  EMG_tmp = EMG_max[ch]; // 没有峰值的情况取最大值
		EMG_max[ch] = 0;	 // 最大值复位
		EMG_peak[ch] = 0;  // 峰值复位
/* 以上是取最大峰值的算法,如果没有峰值,取最大值 */
//		if(ch) printf("%d\r\n",EMG_tmp);
		if (array_count[ch])	 // 按从小到大排序
		{
			for(j = 0; j < array_count[ch]; j++) // 排序
			{
				if (EMG_tmp >= EMG_array[ch][j]); // 当前值逐位比较
				else
				{
					 for(i = array_count[ch]; i > j; i--) // 更新后面的数据
					 {
						 EMG_array[ch][i] = EMG_array[ch][i-1];
					 }
					 break;
				}
			}
			EMG_array[ch][j] = EMG_tmp;											
		}
		else	EMG_array[ch][0] = EMG_tmp;  // 首个数据
			
		array_count[ch]++; // 数据量计数递增
			
		if (++clk_10Hz_cnt[ch] >= clk_10Hz_value)  // 10Hz  M_clk_value
		{
			clk_10Hz_cnt[ch] = 0;

			EMG_tmp = (uint32_t)EMG_array[ch][ clk_10Hz_value - 1 ]*1000/EMG_coefficient_PP[ch];	  // 0.1uV  20200817

			if(EMG_tmp >= 20000) EMG_tmp = 20000; 
			
			EMG_tmp = smooth_handler( ch, EMG_tmp );  // 最终EMG数据

/*			
			if(EMG_tmp > 10) 
			{
				if(!ch) EMG_A_value = EMG_tmp - 10; // 噪声修正 - 5
				else EMG_B_value = EMG_tmp - 10;    
			}
*/			
			
/*	// 发送最终EMG数据 
			if(EMG_data_send_en == ENABLE) 
			{
				if( !ch ) send_EMG_value(); 
			}	
*/			
			result = EMG_tmp;  
	
			array_count[ch] = 0;				
			EMG_array[ch][0] = 0;					
		}
	}
	
	return result;
}


/*******************************************************
	@Function			: EMG_arithmetic_average
	@Parameter 		: EMG_data , 肌电原始数据
	@Description	: 峰值平均值检波
	@Return				: 最终计算结果
	@Remark				: None	
*/
#define  DIV_10HZ_CNT    200  
uint16_t  EMG_arithmetic_average ( uint8_t ch, uint16_t EMG_org )  
{	
	uint16_t EMG_present;				 // 当前EMG值
	uint16_t DC_offset;					 // 直流分量
	uint16_t EMG_after_filter;   // 滤波后的数据
	uint16_t EMG_tmp;						 // EMG临时数据
	uint16_t result = 0xFFFF;				 // 最终结果
	
	static uint8_t  clk_10Hz_cnt[EMG_CHn] = {0}; 	 			// 10Hz计数
	static uint8_t  EMG_direction[EMG_CHn] = {0};	 			// 峰值标志位
	static uint16_t EMG_previous[EMG_CHn] = {0};				// 上一个EMG值
	static uint16_t EMG_max[EMG_CHn] = {0};							// 最大值
	static uint32_t EMG_peak_sum[EMG_CHn] = {0};				// 峰值累加和
	static uint16_t EMG_peak_cnt[EMG_CHn] = {0}; 				// 峰值个数计数

  EMG_after_filter = Filter_Bandstop_50_100_150Hz_Sampling_2000Hz( EMG_org, ch ); // 低通 + 带阻滤波
//EMG_after_filter = EMG_org;
//	if(Flag.byte.EMG_filter_en == (ch + 1))
//	{
//			QUEUE_WRITE(EMG_FILTER_Dat, EMG_after_filter);
//			return;
//	}
	
	EMG_present = EMG_after_filter; 
	
	DC_offset = Calculated_dc_component( ch, EMG_after_filter );
	
	if(EMG_present >= DC_offset) EMG_present -= DC_offset;  
	else EMG_present = DC_offset - EMG_present;	

	if (EMG_present > EMG_max[ch])	{ EMG_max[ch] = EMG_present; }  // 当前值大于最大值
	if (EMG_present >= EMG_previous[ch])	{ EMG_previous[ch] = EMG_present; EMG_direction[ch] = 1; } // 当前值大于前一个值
	else
	{		
		if (EMG_present < EMG_previous[ch]) // 当前值小于前一个值
		{				
			if (EMG_direction[ch] != 0)  // 到达极峰顶
			{
				EMG_direction[ch] = 0;	// 切换极性
				EMG_peak_sum[ch] += EMG_previous[ch];   // 峰值累加
				EMG_peak_cnt[ch]++;		// 峰值个数计数
				
				EMG_max[ch] = 0;
			}
			EMG_previous[ch] = EMG_present;	// 更新前一个值buff			  
		}
	}
	
	if (++clk_10Hz_cnt[ch] >= DIV_10HZ_CNT)   
	{	
		clk_10Hz_cnt[ch] = 0;
		
//			EMG_tmp = (uint32_t)(EMG_peak_sum[ch]/EMG_peak_cnt[ch])*500/EMG_coefficient_avg[ch];	// 0.2uV
		EMG_tmp = (uint32_t)(EMG_peak_sum[ch]/EMG_peak_cnt[ch])*1000/EMG_coefficient_avg[ch];   // 0.1uV
		
		EMG_tmp = smooth_handler( ch, EMG_tmp );  // 最终EMG数据			
		
/*			
		if(!ch) EMG_A_value = EMG_tmp;
		else EMG_B_value = EMG_tmp;

		
	// EMG数据上传	
		if(Flag.byte.EMG_data_send_en == ENABLE) 
		{
			if( !ch ) send_EMG_value();  
		}				
*/
		
		result = EMG_tmp;	
		
		EMG_peak_sum[ch] = 0;				
		EMG_peak_cnt[ch] = 0;
		EMG_max[ch] = 0;	 // 最大值复位
	}
	return result;
}

/*******************************************************
	@Function			: EMG_arithmetic_RMS
	@Parameter 		: EMG_data , 肌电原始数据
	@Description	: 均方根检波
	@Return				: None
	@Remark				: None	
*/
#define RMS_10HZ_CNT		200
uint16_t EMG_arithmetic_RMS( uint8_t ch, uint16_t EMG_org )  
{	
	uint16_t EMG_present;				 // 当前EMG值
	uint16_t DC_offset;					 // 直流分量
	uint16_t EMG_after_filter;   // 滤波后的数据
	float EMG_tmp;						 // EMG临时数据
	uint16_t result = 0xFFFF;				 // 最终计算结果
	
	uint32_t avg;
	static uint16_t  clk_10Hz_cnt[EMG_CHn] = {0}; 	 			// 10Hz计数
	static float avg_sum[EMG_CHn] = {0};		// 均值和
	
  EMG_after_filter = Filter_Bandstop_50_100_150Hz_Sampling_2000Hz( EMG_org, ch ); // 低通 + 带阻滤波
	
//	if(Flag.byte.EMG_filter_en == (ch + 1))
//	{
//			QUEUE_WRITE(EMG_FILTER_Dat, EMG_after_filter);
//			return;
//	}
	
	EMG_present = EMG_after_filter; 
	
	DC_offset = Calculated_dc_component( ch, EMG_after_filter );

	if(EMG_present >= DC_offset) EMG_present -= DC_offset;  
	else EMG_present = DC_offset - EMG_present;	
	
	avg = EMG_present * EMG_present;
	avg = avg / RMS_10HZ_CNT;
	
	avg_sum[ch] += avg;
	
	if(++clk_10Hz_cnt[ch] >= RMS_10HZ_CNT)
	{
		clk_10Hz_cnt[ch] = 0;
		EMG_tmp = sqrt(avg_sum[ch]);
		avg_sum[ch] = 0;
		
//		EMG_tmp = EMG_tmp*500/EMG_coefficient_avg[ch];	// 0.2uV
		EMG_tmp = EMG_tmp*1000/EMG_coefficient_rms[ch];	 	// 0.1uV

/*		
		if(!ch) EMG_A_value = (uint16_t)EMG_tmp;
		else EMG_B_value = (uint16_t)EMG_tmp;
		
		// EMG数据上传	
		if(Flag.byte.EMG_data_send_en == ENABLE) 
		{
			if( !ch ) send_EMG_value(); 
		}
*/		
		result = EMG_tmp;
	}
	
	return result;
}




