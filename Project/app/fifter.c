/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: fifter.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "fifter.h"

/************************************************************************
* Function Name : Filter_Bandstop_50_100_150Hz_Sampling_2000Hz
* Description   : 69阶 50Hz、100Hz、150Hz、梳状带阻滤波器
* Parameter			: EMG_original , EMG原始数据
*									Channel, 通道号
* Return				: 滤波后的数据
* Remark				: None
************************************************************************/
int16_t  giBSF_Buff[2][256];
uint16_t Filter_Bandstop_50_100_150Hz_Sampling_2000Hz(uint16_t EMG_original, uint8_t Channel)   
{  
  int64_t sum = 0;	  
	int32_t tmp;
  uint16_t i;	 
	 
	uint16_t k;
	static uint16_t index[2] = {0,0};
	
	 
	static int16_t Sampling_Factor[243] =
	{
		 -1, -1, -2, -3, -2, -1, 0, 2, 4, 7, 7, 5, 5, 4, 1, -3, 0, 5, 7, 9, 17, 24, 21, 13, 13, 11, 0, -9, 3, 20, 27, 37, 64, 81, 61, 28, 3, -41, -124, -199, -228, -244, 
		 -269, -251, -160, -64, -10, 55, 155, 204, 159, 114, 118, 85, -13, -49, 25, 80, 61, 103, 233, 274, 174, 119, 160, 109, -53, -80, 76, 160, 128, 238, 480, 512, 292, 
		 151, 102, -209, -734, -1007, -997, -1124, -1326, -1106, -550, -251, -183, 197, 736, 750, 383, 375, 590, 288, -262, -162, 326, 242, -113, 269, 946, 721, 63, 281, 
		 821, 266, -637, -187, 785, 348, -498, 468, 1910, 989, -749, 267, 2066, -356, -4516, -1675, 8603, 14615, 8603, -1675, -4516, -356, 2066, 267, -749, 989, 1910, 468, 
		 -498, 348, 785, -187, -637, 266, 821, 281, 63, 721, 946, 269, -113, 242, 326, -162, -262, 288, 590, 375, 383, 750, 736, 197, -183, -251, -550, -1106, -1326, -1124, 
		 -997, -1007, -734, -209, 102, 151, 292, 512, 480, 238, 128, 160, 76, -80, -53, 109, 160, 119, 174, 274, 233, 103, 61, 80, 25, -49, -13, 85, 118, 114, 159, 204, 155, 
		 55, -10, -64, -160, -251, -269, -244, -228, -199, -124, -41, 3, 28, 61, 81, 64, 37, 27, 20, 3, -9, 0, 11, 13, 13, 21, 24, 17, 9, 7, 5, 0, -3, 1, 4, 5, 5, 7, 7, 4, 
		 2, 0, -1, -2, -3, -2, -1, -1 
	};

	#define  Factor_len  (sizeof(Sampling_Factor)/sizeof(int16_t))

	giBSF_Buff[Channel][index[Channel]] = EMG_original - UINT16_middle_value;
	
	if(++index[Channel] >= Factor_len) index[Channel] = 0;
	
	for(i=0;i<Factor_len;i++)
	{
		k = i + index[Channel];
		if(k >= Factor_len) k -= Factor_len;
		tmp = Sampling_Factor[i] * giBSF_Buff[Channel][k];
		
		sum += tmp;
	}
	
	sum = sum / 32768;
	
	sum += UINT16_middle_value;
	
	if(sum > 65535) sum = 65535;
	if(sum < 0) sum = 0;
	
	return((uint16_t)sum);
}




