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
	@Parameter 		: ch , ͨ�����
									dat , ��������
	@Description	: ��ֱ��������ƫ�õ�ѹ��
	@Return				: None
	@Remark				: 100��ֵ�ĵ���ƽ��
*/
static uint16_t Calculated_dc_component ( uint8_t ch, uint16_t dat ) 
{
	static uint32_t sum[EMG_CHn] = {0};       // �ۼ�ֵ
	static uint16_t init_done[EMG_CHn] = {0};	// ��ֵ�ļ���
	static uint16_t old_buff[EMG_CHn][257];   // ���ݻ���
	static uint16_t index[EMG_CHn] = {0};			// 100�����ݻ����ָ��
	uint16_t DC_component = 0;			// ֱ������
	
	sum[ch] += dat;  
	
	old_buff[ch][index[ch]] = dat;  // FIFO
	if(++index[ch] > 256) index[ch] = 0;  
	
	if(init_done[ch] < 256) 	init_done[ch]++; // �����ܺ�
	else 	sum[ch] -= old_buff[ch][index[ch]];
	
	if(init_done[ch] >= 256) 	DC_component = sum[ch] >> 8;	 // ����ֱ������
	else DC_component = sum[ch] / init_done[ch];
	
	return DC_component;
}

/*******************************************************
	@Function			: smooth_handler
	@Parameter 		: ch , ͨ�����
									EMG_dat , ��������
	@Description	: ƽ������
	@Return				: None
	@Remark				: None
*/
static uint16_t smooth_handler ( uint8_t ch, uint16_t EMG_dat )
{
	static uint8_t EMG_value_pcount[EMG_CHn] = {0};	// �����Լ���
	static uint8_t EMG_value_ncount[EMG_CHn] = {0};	// �����Լ���	
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
	@Parameter 		: EMG_data , ����ԭʼ����
	@Description	: ��ֵ���ֵ�첨
	@Return				: ���ռ���ֵ
	@Remark				: None	
*/
#define  clk_50Hz_value	  40   // 2KHz   
#define	 clk_10Hz_value   4		 
uint16_t emg_arithmetic_pp ( uint8_t ch, uint16_t EMG_org ) 
{	
  uint8_t  i,j;
	
  uint16_t EMG_present;				 // ��ǰEMGֵ
	uint16_t DC_offset;					 // ֱ������
	uint16_t EMG_after_filter;   // �˲��������
	uint16_t EMG_tmp;						 // �����õ��м����
	uint16_t result = 0xFFFF;				 // ���ս��
	
	static uint8_t  clk_50Hz_cnt[EMG_CHn] = {0};	 			// 50Hz����
	static uint8_t  clk_10Hz_cnt[EMG_CHn] = {0}; 	 			// 10Hz����
	static uint8_t  EMG_direction[EMG_CHn] = {0};	 			// ��ֵ��־λ
	static uint8_t  array_count[EMG_CHn] = {0};   			// ����Ԫ�ظ�������
	static uint16_t EMG_previous[EMG_CHn] = {0};				// ��һ��EMGֵ
	static uint16_t EMG_max[EMG_CHn] = {0};							// ���ֵ
	static uint16_t EMG_peak[EMG_CHn] = {0};						// ��ֵ
	static uint16_t	EMG_array[EMG_CHn][clk_10Hz_value]; // ���ֵ��������
	
  EMG_after_filter = Filter_Bandstop_50_100_150Hz_Sampling_2000Hz( EMG_org, ch ); // �����˲�

//	if(Flag.byte.EMG_filter_en == (ch + 1))
//	{
//			QUEUE_WRITE(EMG_FILTER_Dat, EMG_after_filter);
//			return;
//	}
	
	EMG_present = EMG_after_filter;  
	
	DC_offset = Calculated_dc_component( ch, EMG_after_filter );

	// ��ȥֱ������
	if( EMG_present >= DC_offset ) EMG_present -= DC_offset;  
	else  EMG_present = DC_offset - EMG_present;			

	if (EMG_present > EMG_max[ch])	{ EMG_max[ch] = EMG_present; }  // ��ǰֵ�������ֵ
	if (EMG_present > EMG_previous[ch])	{ EMG_previous[ch] = EMG_present; EMG_direction[ch] = 1; } // ��ǰֵ����ǰһ��ֵ
	else
	{		
/* ������ȡ���ֵ���㷨,���û�з�ֵ,ȡ���ֵ */
		if (EMG_present < EMG_previous[ch]) // ��ǰֵС��ǰһ��ֵ
		{				
			if (EMG_direction[ch] != 0)  // ���Ｋ�嶥
			{
				EMG_direction[ch] = 0;	// �л�����
				if (EMG_peak[ch] < EMG_previous[ch])  
				{
					EMG_peak[ch]= EMG_previous[ch]; // ���ֵС��ǰһ��ֵ(����ǰ��ֵ)
//					if(ch) printf("%d\r\n",EMG_peak[ch]);
				}
			}
			EMG_previous[ch] = EMG_present;	// ����ǰһ��ֵbuff			  
		}
	}
	
	if (++clk_50Hz_cnt[ch] >= clk_50Hz_value)  // ���ڿ�� S_clk_value  
	{	
		clk_50Hz_cnt[ch] = 0;
		if (EMG_peak[ch] != 0 )	{ EMG_tmp = EMG_peak[ch]; } // �з�ֵȡ��ֵ
		else  EMG_tmp = EMG_max[ch]; // û�з�ֵ�����ȡ���ֵ
		EMG_max[ch] = 0;	 // ���ֵ��λ
		EMG_peak[ch] = 0;  // ��ֵ��λ
/* ������ȡ����ֵ���㷨,���û�з�ֵ,ȡ���ֵ */
//		if(ch) printf("%d\r\n",EMG_tmp);
		if (array_count[ch])	 // ����С��������
		{
			for(j = 0; j < array_count[ch]; j++) // ����
			{
				if (EMG_tmp >= EMG_array[ch][j]); // ��ǰֵ��λ�Ƚ�
				else
				{
					 for(i = array_count[ch]; i > j; i--) // ���º��������
					 {
						 EMG_array[ch][i] = EMG_array[ch][i-1];
					 }
					 break;
				}
			}
			EMG_array[ch][j] = EMG_tmp;											
		}
		else	EMG_array[ch][0] = EMG_tmp;  // �׸�����
			
		array_count[ch]++; // ��������������
			
		if (++clk_10Hz_cnt[ch] >= clk_10Hz_value)  // 10Hz  M_clk_value
		{
			clk_10Hz_cnt[ch] = 0;

			EMG_tmp = (uint32_t)EMG_array[ch][ clk_10Hz_value - 1 ]*1000/EMG_coefficient_PP[ch];	  // 0.1uV  20200817

			if(EMG_tmp >= 20000) EMG_tmp = 20000; 
			
			EMG_tmp = smooth_handler( ch, EMG_tmp );  // ����EMG����

/*			
			if(EMG_tmp > 10) 
			{
				if(!ch) EMG_A_value = EMG_tmp - 10; // �������� - 5
				else EMG_B_value = EMG_tmp - 10;    
			}
*/			
			
/*	// ��������EMG���� 
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
	@Parameter 		: EMG_data , ����ԭʼ����
	@Description	: ��ֵƽ��ֵ�첨
	@Return				: ���ռ�����
	@Remark				: None	
*/
#define  DIV_10HZ_CNT    200  
uint16_t  EMG_arithmetic_average ( uint8_t ch, uint16_t EMG_org )  
{	
	uint16_t EMG_present;				 // ��ǰEMGֵ
	uint16_t DC_offset;					 // ֱ������
	uint16_t EMG_after_filter;   // �˲��������
	uint16_t EMG_tmp;						 // EMG��ʱ����
	uint16_t result = 0xFFFF;				 // ���ս��
	
	static uint8_t  clk_10Hz_cnt[EMG_CHn] = {0}; 	 			// 10Hz����
	static uint8_t  EMG_direction[EMG_CHn] = {0};	 			// ��ֵ��־λ
	static uint16_t EMG_previous[EMG_CHn] = {0};				// ��һ��EMGֵ
	static uint16_t EMG_max[EMG_CHn] = {0};							// ���ֵ
	static uint32_t EMG_peak_sum[EMG_CHn] = {0};				// ��ֵ�ۼӺ�
	static uint16_t EMG_peak_cnt[EMG_CHn] = {0}; 				// ��ֵ��������

  EMG_after_filter = Filter_Bandstop_50_100_150Hz_Sampling_2000Hz( EMG_org, ch ); // ��ͨ + �����˲�
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

	if (EMG_present > EMG_max[ch])	{ EMG_max[ch] = EMG_present; }  // ��ǰֵ�������ֵ
	if (EMG_present >= EMG_previous[ch])	{ EMG_previous[ch] = EMG_present; EMG_direction[ch] = 1; } // ��ǰֵ����ǰһ��ֵ
	else
	{		
		if (EMG_present < EMG_previous[ch]) // ��ǰֵС��ǰһ��ֵ
		{				
			if (EMG_direction[ch] != 0)  // ���Ｋ�嶥
			{
				EMG_direction[ch] = 0;	// �л�����
				EMG_peak_sum[ch] += EMG_previous[ch];   // ��ֵ�ۼ�
				EMG_peak_cnt[ch]++;		// ��ֵ��������
				
				EMG_max[ch] = 0;
			}
			EMG_previous[ch] = EMG_present;	// ����ǰһ��ֵbuff			  
		}
	}
	
	if (++clk_10Hz_cnt[ch] >= DIV_10HZ_CNT)   
	{	
		clk_10Hz_cnt[ch] = 0;
		
//			EMG_tmp = (uint32_t)(EMG_peak_sum[ch]/EMG_peak_cnt[ch])*500/EMG_coefficient_avg[ch];	// 0.2uV
		EMG_tmp = (uint32_t)(EMG_peak_sum[ch]/EMG_peak_cnt[ch])*1000/EMG_coefficient_avg[ch];   // 0.1uV
		
		EMG_tmp = smooth_handler( ch, EMG_tmp );  // ����EMG����			
		
/*			
		if(!ch) EMG_A_value = EMG_tmp;
		else EMG_B_value = EMG_tmp;

		
	// EMG�����ϴ�	
		if(Flag.byte.EMG_data_send_en == ENABLE) 
		{
			if( !ch ) send_EMG_value();  
		}				
*/
		
		result = EMG_tmp;	
		
		EMG_peak_sum[ch] = 0;				
		EMG_peak_cnt[ch] = 0;
		EMG_max[ch] = 0;	 // ���ֵ��λ
	}
	return result;
}

/*******************************************************
	@Function			: EMG_arithmetic_RMS
	@Parameter 		: EMG_data , ����ԭʼ����
	@Description	: �������첨
	@Return				: None
	@Remark				: None	
*/
#define RMS_10HZ_CNT		200
uint16_t EMG_arithmetic_RMS( uint8_t ch, uint16_t EMG_org )  
{	
	uint16_t EMG_present;				 // ��ǰEMGֵ
	uint16_t DC_offset;					 // ֱ������
	uint16_t EMG_after_filter;   // �˲��������
	float EMG_tmp;						 // EMG��ʱ����
	uint16_t result = 0xFFFF;				 // ���ռ�����
	
	uint32_t avg;
	static uint16_t  clk_10Hz_cnt[EMG_CHn] = {0}; 	 			// 10Hz����
	static float avg_sum[EMG_CHn] = {0};		// ��ֵ��
	
  EMG_after_filter = Filter_Bandstop_50_100_150Hz_Sampling_2000Hz( EMG_org, ch ); // ��ͨ + �����˲�
	
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
		
		// EMG�����ϴ�	
		if(Flag.byte.EMG_data_send_en == ENABLE) 
		{
			if( !ch ) send_EMG_value(); 
		}
*/		
		result = EMG_tmp;
	}
	
	return result;
}




