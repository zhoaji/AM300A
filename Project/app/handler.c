
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: handler.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/
#include "peripheral.h"
#include <string.h>
#include "handler.h"
#include "main.h"
#include "crc8.h"
#include "bsp_battery.h"
#include "emg_wave.h"
#include "stim_control.h"
#include "bsp_gpio.h"

//#include "protocol.h"

#define HEAD1 0xAA
#define HEAD2 0x55

#define TOKEN_NUM			2
#define TYPE_NUM			50

CMD_HANDLER_TYPE cmd_handler_tab[TOKEN_NUM][TYPE_NUM] = {NULL};

uint8_t old_protocol_en = 0;

/************************************************
	@Function			: ble_send_buff
	@Description	:	通过BLE发送非协议数据包类型数据
	@parameter		: buff , 数据
									len , 数据长度
	@Return				: None
	@Remark				: None
*/
void ble_send_buff(uint8_t *buff, uint32_t len)
{
//	for(int i = 0; i < len; i++)  
//	{
//		QUEUE_WRITE(BLE_Tx, *buff);
//		buff++;
//	}
}

/************************************************
	@Function			: ble_send_packet
	@Description	:	通过BLE发送协议类型数据包
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: None
*/
extern void ble_send_data(uint8_t *buff, uint32_t length);
static void ble_send_packet(PACKET_Typedef * packet)
{

#ifdef CONFIG_LOG_OUTPUT
	
	uint8_t i;
	uint8_t crc = 0;
	static uint8_t buff[64] = {0};
	
	for(i = 0; i < (packet->para.Length + 3); i++)  // add head1 head2 token length  sub crc
	{
		crc = CRC8(crc, packet->buf[i]);
		buff[i] = packet->buf[i];
//		QUEUE_WRITE(BLE_Tx, packet->buf[i]);
	}
	buff[i] = crc;
//	QUEUE_WRITE(BLE_Tx, crc); // add crc byte
	uart_send_block(HS_UART0, buff, packet->para.Length + 4);
//	printf("\r\naaaa 0x%2x aaaa\r\n", crc);
#else
	
	ble_send_data(packet->buf, packet->para.Length + 3);
#endif

	
//	ble_send_data(packet->buf, packet->para.Length + 3);
}

/************************************************
	@Function			: battery_voltage_packet_send
	@Description	:	发送电池电电量
	@parameter		: None
	@Return				: None
	@Remark				: 主动上传速率 1Hz
*/
void battery_voltage_packet_send()
{
	PACKET_Typedef battery_packet;
	
	battery.vol_level = 3;
	battery.voltage_bat_mv = 4200;
	
	battery_packet.para.Head1 = HEAD1;
	battery_packet.para.Head2 = HEAD2;
	battery_packet.para.Token = GERNARL_TOKEN;   
	battery_packet.para.Length = 0x05;
	battery_packet.para.Type = ACK_BATVOL;  
	
	battery_packet.para.Data[0] = battery.vol_level;
	battery_packet.para.Data[1] = (uint8_t)(battery.voltage_bat_mv >> 8);
	battery_packet.para.Data[2] = (uint8_t)battery.voltage_bat_mv;
	
	ble_send_packet(&battery_packet);
	
}

/************************************************
	@Function			: emg_wave_packet_send
	@Description	:	发送EMG波形包
	@parameter		: emg_a , A通道肌电数据
									emg_b , B通道肌电数据
	@Return				: None
	@Remark				: 主动上传速率 10Hz
*/
void emg_wave_packet_send(uint16_t emg_a, uint16_t emg_b)
{	
	PACKET_Typedef emg_wave_packet;
	
	emg_wave_packet.para.Head1 = HEAD1;
	emg_wave_packet.para.Head2 = HEAD2;
	emg_wave_packet.para.Token = AM300_TOKEN;  

	if(!old_protocol_en) // new protocol
	{
		emg_wave_packet.para.Length = 0x06;
		emg_wave_packet.para.Type = PACK_EMG_WAVE + 0x02;  
		
		emg_wave_packet.para.Data[0] = (uint8_t)(emg_a >> 8);
		emg_wave_packet.para.Data[1] = (uint8_t)emg_a;
		
		emg_wave_packet.para.Data[2] = (uint8_t)(emg_b >> 8);
		emg_wave_packet.para.Data[3] = (uint8_t)emg_b;
	}
	else // old protocol
	{
		emg_wave_packet.para.Length = 0x04;
		emg_wave_packet.para.Type = PACK_EMG_WAVE;  // 此处旧协议（Type = 0x03）仅上传1个通道EMG数据，
		
		emg_wave_packet.para.Data[0] = (uint8_t)(emg_a >> 8);
		emg_wave_packet.para.Data[1] = (uint8_t)emg_a;
	}
	
	ble_send_packet(&emg_wave_packet);
}

/************************************************
	@Function			: probe_status_packet_send
	@Description	:	发送电极状态包
	@parameter		: emg_pro_status , EMG电极状态
									stim_pro_status , 刺激电极状态
	@Return				: None
	@Remark				: 主动上传速率  10Hz 
*/
void probe_status_packet_send(uint8_t emg_pro_status, uint8_t stim_pro_status)
{	
	PACKET_Typedef probe_status_packet;
	
	probe_status_packet.para.Head1 = HEAD1;
	probe_status_packet.para.Head2 = HEAD2;
	probe_status_packet.para.Token = AM300_TOKEN;   
	probe_status_packet.para.Length = 0x04;
	probe_status_packet.para.Type = ACK_LEAD_STA;  
	
//	if(stim_a_control.probe_status )
	
	
	probe_status_packet.para.Data[0] = stim_a_control.probe_status + (stim_b_control.probe_status << 1);
	probe_status_packet.para.Data[1] = stim_pro_status;
	
	ble_send_packet(&probe_status_packet);
}

/************************************************
	@Function			: stim_status_packet_send
	@Description	:	肌肉刺激治疗状态包
	@parameter		: status , 治疗状态
	@Return				: None
	@Remark				: 该状态包仅在肌肉刺激模式启动后发送   待完善 20210527
*/
void stim_status_packet_send(uint8_t status)
{	
	PACKET_Typedef probe_status_packet;
	
	probe_status_packet.para.Head1 = HEAD1;
	probe_status_packet.para.Head2 = HEAD2;
	probe_status_packet.para.Token = AM300_TOKEN;   
	probe_status_packet.para.Length = 0x03;
	probe_status_packet.para.Type = PACK_STIM_STA;  
	
	probe_status_packet.para.Data[0] = status;
	
	ble_send_packet(&probe_status_packet);
}

/************************************************
	@Function			: battery_alarm_stop_cure_packet_send
	@Description	:	电池电量低，停止治疗
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void battery_alarm_stop_cure_packet_send(void)
{
	PACKET_Typedef probe_status_packet;
	
	probe_status_packet.para.Head1 = HEAD1;
	probe_status_packet.para.Head2 = HEAD2;
	probe_status_packet.para.Token = AM300_TOKEN;   
	probe_status_packet.para.Length = 0x02;
	probe_status_packet.para.Type = ACK_BAT_LOW;  
	
	ble_send_packet(&probe_status_packet);
}

/************************************************
	@Function			: emg_org_wave_data_packet_send
	@Description	:	EMG原始波形数据包
	@parameter		: channel , 通道
									fifo , 原始数据缓存
	@Return				: None
	@Remark				: None
*/
void emg_org_wave_data_packet_send(uint8_t *buff)
{
	static uint8_t index = 0;
	PACKET_Typedef emg_raw_wave_packet;
	
	emg_raw_wave_packet.para.Head1 = HEAD1;
	emg_raw_wave_packet.para.Head2 = HEAD2;
	emg_raw_wave_packet.para.Token = AM300_TOKEN;  

	emg_raw_wave_packet.para.Length = 0x17;
	emg_raw_wave_packet.para.Type = PACK_ORG_DATA;  
	
	emg_raw_wave_packet.para.Data[0] = index;
	if(++index >= 256) index = 0;
	
	for(uint8_t i = 0; i < 10; i++)
		emg_raw_wave_packet.para.Data[i + 1] = buff[i];

	ble_send_packet(&emg_raw_wave_packet);
}

/************************************************
	@Function			: emg_org_probe_leadoff_data_packet_send
	@Description	:	EMG原始脱落数据包
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void emg_org_probe_leadoff_data_packet_send(void)
{

}

/*********************** 通用指令处理函数 ************************/
/************************************************
	@Function			: inquire_debug_version_handler
	@Description	:	查询Debug版本号指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 F0 02 80 47 
*/
static void inquire_debug_version_handler(PACKET_Typedef * packet)
{
	packet->para.Length = 3 + sizeof(__DATE__);
	packet->para.Type = ACK_DEBUG_VERSION;
	
	packet->para.Data[0] = (uint8_t)DEBUG_VERSION;
	memcpy(packet->para.Data + 1, __DATE__, sizeof(__DATE__));

	ble_send_packet(packet);
}

/************************************************
	@Function			: inquire_soft_version_handler
	@Description	:	查询固件版本号指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 F0 02 81 19 
*/
static void inquire_soft_version_handler(PACKET_Typedef *packet)
{
	uint8_t tmp = 0;
	
	packet->para.Length = 5 + sizeof(PRODUCT_NAME);
	packet->para.Type = ACK_VERSION;
	
	// sv_h
	tmp = APP_VERSION / 100;
	tmp = (tmp/10 * 16) + (tmp % 10);
	packet->para.Data[0] = tmp;
	
	// sv_l
	tmp = APP_VERSION % 100;
	tmp = (tmp/10 * 16) + (tmp % 10);
	packet->para.Data[1] = tmp;
	
	// hv
	tmp = (uint8_t)HW_VERSION;
	tmp = (tmp/10 * 16) + (tmp % 10);
	packet->para.Data[2] = tmp;
	
	memcpy(packet->para.Data + 3, PRODUCT_NAME, sizeof(PRODUCT_NAME));

	ble_send_packet(packet);
}

/************************************************
	@Function			: inquire_serial_number_handler
	@Description	:	查询序列号指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 F0 02 82 FB 
									待完善 20210527
*/
#define SERIAL_NUMBER		"00001"   // 序列号测试暂定
static void inquire_serial_number_handler(PACKET_Typedef *packet)
{
	packet->para.Length = 2 + sizeof(SERIAL_NUMBER);
	packet->para.Type = ACK_SN_INQ;
	
	memcpy(packet->para.Data, SERIAL_NUMBER, sizeof(SERIAL_NUMBER));
	
	ble_send_packet(packet);
}

/************************************************
	@Function			: inquire_battery_voltage_handler
	@Description	:	查询电池电量指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 F0 02 83 A5
*/
static void inquire_battery_voltage_handler(PACKET_Typedef *packet)
{
	// test 
//	battery.vol_level = 3;
//	battery.voltage_bat_mv = 4200;
	
	packet->para.Length = 5;
	packet->para.Type = ACK_BATVOL;
	
	packet->para.Data[0] = battery.vol_level;
	packet->para.Data[1] = (uint8_t)(battery.voltage_bat_mv >> 8);
	packet->para.Data[2] = (uint8_t)battery.voltage_bat_mv;
	
	ble_send_packet(packet);
}


/************************ AM300指令处理函数 ************************/
/************************************************
	@Function			: start_emg_wave_data_handler
	@Description	:	开始上传EMG波形指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 81 BF
*/
static void start_emg_wave_data_handler(PACKET_Typedef *packet)
{
	if(packet->para.Length == 0x03) // new protocol
	{
		emg_wave.detector_type = packet->para.Data[0];  // 0x00: 峰值平均  0x01: 峰值最大  0x02: 均方根
		old_protocol_en = 0;
	}
	else if(packet->para.Length == 0x02)
	{
		old_protocol_en = 1;
	}
	
	emg_wave.emg_wave_en = 1;
	emg_wave.emg_wave_org_en = 0;
	gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_HIGH);  // 继电器切到EMG
	tim_start(HS_TIM1);
	
	packet->para.Length = 3;
	packet->para.Type = ACK_EN_EMG;
	
	packet->para.Data[0] = 0x00;  // 0x00 : 响应正确 
																// 0xF1 命令错误（例如肌肉刺激治疗模式发送该命令，则命令错误）
	ble_send_packet(packet);
}

/************************************************
	@Function			: stop_emg_wave_data_handler
	@Description	:	停止上传EMG波形指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 82 5D
									待完善  20210527
*/
static void stop_emg_wave_data_handler(PACKET_Typedef *packet)
{
	emg_wave.emg_wave_en = 0;
	gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_LOW);  // 继电器切到STIM
	tim_stop(HS_TIM1);
	
	packet->para.Length = 3;
	packet->para.Type = ACK_DIS_EMG;
	
	packet->para.Data[0] = 0x00;  // 0x00 : 响应正确 
																// 0xF1 命令错误（例如肌肉刺激治疗模式发送该命令，则命令错误）
																// ↑有疑问，下位机还分模式？20210527
	ble_send_packet(packet);
}

/************************************************
	@Function			: set_stim_parameter_handler
	@Description	:	刺激参数设置指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : 
*/
static void set_stim_parameter_handler(PACKET_Typedef *packet)
{
	Stim_parameter_Typedef temp_para;
	
	packet->para.Data[0] = 0x00;
	
	do{
		temp_para.frequency = (packet->para.Data[0] << 8) + packet->para.Data[1];
		if((temp_para.frequency < FREQUENCY_MIN) && (temp_para.frequency > FREQUENCY_MAX)) 
		{
			packet->para.Data[0] = 0xF1; 
			break;
		}
		
		temp_para.pulse_width = (packet->para.Data[2] << 8) + packet->para.Data[3];
		if((temp_para.pulse_width < PULSE_WIDTH_MIN) && (temp_para.pulse_width > PULSE_WIDTH_MAX)) 
		{
			packet->para.Data[0] = 0xF1; 
			break;
		}
		
		temp_para.rasetime = packet->para.Data[4];
		if(/*(temp_para.rasetime < RASETIME_MIN) && */(temp_para.rasetime > RASETIME_MAX)) 
		{
			packet->para.Data[0] = 0xF1; 
			break;
		}
		
		temp_para.stimtime = packet->para.Data[5];
		if(/*(temp_para.stimtime < STIMTIME_MIN) && */(temp_para.stimtime > STIMTIME_MAX) 
			&& temp_para.stimtime != STIMTIME_UNLIMIT) 
		{
			packet->para.Data[0] = 0xF1; 
			break;
		}
		
		temp_para.falltime = packet->para.Data[6];
		if(/*(temp_para.falltime < FALLTIME_MIN) && */(temp_para.falltime > FALLTIME_MAX)) 
		{
			packet->para.Data[0] = 0xF1; 
			break;
		}
		
		temp_para.resttime = packet->para.Data[7];
		if(/*(temp_para.resttime < RESTTIME_MIN) &&*/ (temp_para.resttime > RESTTIME_MAX)) 
		{
			packet->para.Data[0] = 0xF1; 
			break;
		}
		
		if( 0 == (temp_para.rasetime + temp_para.stimtime + temp_para.falltime)) 
		{
			packet->para.Data[0] = 0xF1; 
			break;
		}
		
		memcpy(&stim_parameter, &temp_para, sizeof(temp_para));

	}while(0);
	
	
	if(!packet->para.Data[0])
	{
		pulse_parameter_set();
	}
	
	packet->para.Length = 3;
	packet->para.Type = ACK_PARA_SET;
//	packet->para.Data[0] = 0x00;  // 0x00 响应成功    
//																// 0xF1 参数错误
	ble_send_packet(packet);
}

/************************************************
	@Function			: inquire_stim_parameter_handler
	@Description	:	刺激参数查询指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 9D 81
*/
static void inquire_stim_parameter_handler(PACKET_Typedef *packet)
{
	
	packet->para.Length = 10;
	packet->para.Type = ACK_PARA_INQ;

	packet->para.Data[0] = (uint8_t)(stim_parameter.frequency >> 8);
	packet->para.Data[1] = (uint8_t)stim_parameter.frequency;
	packet->para.Data[2] = (uint8_t)(stim_parameter.pulse_width >>8);
	packet->para.Data[3] = (uint8_t)stim_parameter.pulse_width;
	packet->para.Data[4] = stim_parameter.rasetime;
	packet->para.Data[5] = stim_parameter.stimtime;
	packet->para.Data[6] = stim_parameter.falltime;
	packet->para.Data[7] = stim_parameter.resttime;
	
	ble_send_packet(packet);
}

/************************************************
	@Function			: set_stim_intensity_handler
	@Description	:	刺激强度设置指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 04 87 01 0A 78   A通道 刺激强度 10mA
*/
static void set_stim_intensity_handler(PACKET_Typedef *packet)
{
/*
	if(packet->para.Data[1] <= INTENSITY_MAX)
	{
		if(0x01 == (packet->para.Data[0] & 0x01))
		{
			set_stim_intensity_general(packet->para.Data[1], &stim_a_control);
		}
		if(0x02 == (packet->para.Data[0] & 0x02))
		{
			set_stim_intensity_general(packet->para.Data[1], &stim_b_control);
		}
		
		packet->para.Data[0] = 0x00;
	}
	else packet->para.Data[0] = 0xF1;
	
	packet->para.Length = 4;
	packet->para.Type = ACK_INTENSITY_SET;
	
//	packet->para.Data[0] = 0x00;  // 0x00 响应成功    
//																// 0xF1 参数错误

*/
//	printf("packet->para.Data[0] = %d\r\n",packet->para.Data[0]);
	if(packet->para.Data[0] <= INTENSITY_MAX)
	{
		set_stim_intensity_general(packet->para.Data[0], &stim_a_control);
		set_stim_intensity_general(packet->para.Data[0], &stim_b_control);
		
//		printf("A = %d\r\n",stim_a_control.intensity);
//		printf("B = %d\r\n",stim_b_control.intensity);
	}
	
	ble_send_packet(packet);
}

/************************************************
	@Function			: inquire_stim_intensity_handler
	@Description	:	刺激强度查询指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 07 EE  
*/
static void inquire_stim_intensity_handler(PACKET_Typedef *packet)
{
	packet->para.Length = 4;
	packet->para.Type = ACK_INTENSITY_INQ;
	
	packet->para.Data[0] = stim_a_control.intensity;  
	packet->para.Data[1] = stim_b_control.intensity;  															
	
	ble_send_packet(packet);
}

/************************************************
	@Function			: start_stim_output_handler
	@Description	:	肌肉刺激治疗开始指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 03 93 03 xx  
*/
static void start_stim_output_handler(PACKET_Typedef *packet)
{
//	uint8_t res[CH_NUM] = {0};
	
	start_stim(1);
	
//	if(packet->para.Data[0] & 0x01) { res[CH_A] = startup_stim_operation(&stim_a_control); }
//	if(packet->para.Data[0] & 0x02) { res[CH_B] = startup_stim_operation(&stim_b_control); }
	
//	gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_LOW); // 继电器切换到Stim
	
	packet->para.Length = 0x03;
	packet->para.Type = ACK_STIM_START;
	packet->para.Data[0] = 0;//res[CH_A];  
//	packet->para.Data[1] = 0;//res[CH_B]; 
	ble_send_packet(packet);
}	

/************************************************
	@Function			: pause_stim_output_handler
	@Description	:	肌肉刺激治疗暂停指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 94 1D  
*/
static void pause_stim_output_handler(PACKET_Typedef *packet)
{
	stim_a_control.period_time = 0;
	stim_b_control.period_time = 0;
	
	packet->para.Length = 0x03;
	packet->para.Type = ACK_STIM_PAUSE;
	packet->para.Data[0] = 0;//res[CH_A];  
//	packet->para.Data[1] = 0;//res[CH_B]; 
	ble_send_packet(packet);
}	

/************************************************
	@Function			: stop_stim_output_handler
	@Description	:	肌肉刺激治疗暂停指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 95 43 
*/
static void stop_stim_output_handler(PACKET_Typedef *packet)
{
	
	stim_a_control.period_time = 0;
	stim_b_control.period_time = 0;
	
	packet->para.Length = 0x03;
	packet->para.Type = ACK_STIM_STOP;
	packet->para.Data[0] = 0;//res[CH_A];  
//	packet->para.Data[1] = 0;//res[CH_B]; 
	ble_send_packet(packet);
}	

/************************************************
	@Function			: start_trigger_stim_output_handler
	@Description	:	肌电触发电刺激治疗开始指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 93 9E  
*/
static void start_trigger_stim_output_handler(PACKET_Typedef *packet)
{
	
}	

/************************************************
	@Function			: pause_trigger_stim_output_handler
	@Description	:	肌电触发电刺激治疗暂停指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 94 1D  
*/
static void pause_trigger_stim_output_handler(PACKET_Typedef *packet)
{
	
}	

/************************************************
	@Function			: stop_trigger_stim_output_handler
	@Description	:	肌电触发电刺激疗暂停指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 95 43 
*/
static void stop_trigger_stim_output_handler(PACKET_Typedef *packet)
{
	
}	

/************************************************
	@Function			: trigger_stim_single_handler
	@Description	:	肌电触发电刺激疗暂停指令处理函数
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 9A 02 
*/
static void trigger_stim_single_handler(PACKET_Typedef *packet)
{
	
}

/************************************************
	@Function			: inquire_mode_working_status_handler
	@Description	:	查询下位机工作状态
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 9C DF 
*/
static void inquire_mode_working_status_handler(PACKET_Typedef *packet)
{

}

/************************************************
	@Function			: emg_debug_mode_en_handler
	@Description	:	EMG调试开始/停止
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 A1 9C 
*/
static void emg_debug_mode_en_handler(PACKET_Typedef *packet)
{

}

/************************************************
	@Function			: emg_org_wave_data_en_handler
	@Description	:	EMG原始波形数据使能
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 03 A4 01 B8  // 0:disable  1:enable
*/
static void emg_org_wave_data_en_handler(PACKET_Typedef *packet)
{
	emg_wave.emg_wave_org_en = packet->para.Data[0];
	if(emg_wave.emg_wave_org_en) emg_wave.emg_wave_en = 0;
	gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_HIGH);  // 继电器切到EMG
	tim_start(HS_TIM1);
	
	packet->para.Length = 4;
	packet->para.Type = ACK_ORG_DATA;
	
	packet->para.Data[0] = emg_wave.emg_wave_org_en;
	packet->para.Data[1] = 0x00;  // 0x00 : 响应正确 
																// 0xF1 命令错误（例如肌肉刺激治疗模式发送该命令，则命令错误）
	ble_send_packet(packet);
}

/************************************************
	@Function			: emg_org_probe_leadoff_data_en_handler
	@Description	:	EMG原始波形数据使能
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as : AA 55 69 02 A5 00 89  // 0:disable  1:enable
*/
static void emg_org_probe_leadoff_data_en_handler(PACKET_Typedef *packet)
{
	
}

/************************************************
	@Function			: set_serial_number_handler
	@Description	:	设置设备序列号
	@parameter		: packet , 协议指令类型
	@Return				: None
	@Remark				: CMD Such as :
*/
static void set_serial_number_handler(PACKET_Typedef *packet)
{
	
}

/************************************************
	@Function			: add_protocol_handler_fun
	@Description	:	往协议指令处理函数指针数组添加函数指针
	@parameter		: Token , 指令令牌
									Type , 指令类型
									handler , 指令处理函数指针
	@Return				: None
	@Remark				: None
*/
static void add_protocol_handler_fun(uint8_t Token, uint8_t Type, CMD_HANDLER_TYPE handler)
{
	if(handler != NULL)
	{
		switch(Token)
		{
			case AM300_TOKEN: cmd_handler_tab[0][Type - TYPE_BASE_ADDR] = (CMD_HANDLER_TYPE)handler; break;
			case GERNARL_TOKEN: cmd_handler_tab[1][Type - TYPE_BASE_ADDR] = (CMD_HANDLER_TYPE)handler; break;
			default: break;
		}
	}
}

/************************************************
	@Function			: handler_init
	@Description	:	指令处理函数集的初始化
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
void handler_init(void)
{
		// protocol cmd handler init
	// Gernarl CMD
	add_protocol_handler_fun(GERNARL_TOKEN, CMD_DEBUG_VERSION,	(CMD_HANDLER_TYPE)inquire_debug_version_handler);
	add_protocol_handler_fun(GERNARL_TOKEN, CMD_VERSION, 				(CMD_HANDLER_TYPE)inquire_soft_version_handler);
	add_protocol_handler_fun(GERNARL_TOKEN, CMD_SN_INQ, 				(CMD_HANDLER_TYPE)inquire_serial_number_handler);
	add_protocol_handler_fun(GERNARL_TOKEN, CMD_BATVOL, 				(CMD_HANDLER_TYPE)inquire_battery_voltage_handler);
	
	// AM300 CMD
	add_protocol_handler_fun(AM300_TOKEN, CMD_EN_EMG, 					(CMD_HANDLER_TYPE)start_emg_wave_data_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_DIS_EMG, 					(CMD_HANDLER_TYPE)stop_emg_wave_data_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_PARA_SET,					(CMD_HANDLER_TYPE)set_stim_parameter_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_PARA_INQ, 				(CMD_HANDLER_TYPE)inquire_stim_parameter_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_INTENSITY_SET, 		(CMD_HANDLER_TYPE)set_stim_intensity_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_INTENSITY_INQ, 		(CMD_HANDLER_TYPE)inquire_stim_intensity_handler);
	
	add_protocol_handler_fun(AM300_TOKEN, CMD_STIM_START, 			(CMD_HANDLER_TYPE)start_stim_output_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_STIM_PAUSE, 			(CMD_HANDLER_TYPE)pause_stim_output_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_STIM_STOP, 				(CMD_HANDLER_TYPE)stop_stim_output_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_STIM_START1, 			(CMD_HANDLER_TYPE)start_trigger_stim_output_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_STIM_PAUSE1, 			(CMD_HANDLER_TYPE)pause_trigger_stim_output_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_STIM_STOP1, 			(CMD_HANDLER_TYPE)stop_trigger_stim_output_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_STIM_SINGLE, 			(CMD_HANDLER_TYPE)trigger_stim_single_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_MODE_STA, 				(CMD_HANDLER_TYPE)inquire_mode_working_status_handler);
	
	add_protocol_handler_fun(AM300_TOKEN, CMD_DEBUG, 						(CMD_HANDLER_TYPE)emg_debug_mode_en_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_ORG_DATA, 				(CMD_HANDLER_TYPE)emg_org_wave_data_en_handler);
	add_protocol_handler_fun(AM300_TOKEN, CMD_OFF_DATA, 				(CMD_HANDLER_TYPE)emg_org_probe_leadoff_data_en_handler);

	add_protocol_handler_fun(AM300_TOKEN, CMD_SN_SET, 					(CMD_HANDLER_TYPE)set_serial_number_handler);
//	add_protocol_handler_fun(AM300_TOKEN, CMD_GAIN_SET, 				(CMD_HANDLER_TYPE)inquire_stim_intensity_handler);
//	add_protocol_handler_fun(AM300_TOKEN, CMD_GAIN_INQ, 				(CMD_HANDLER_TYPE)inquire_stim_intensity_handler);
//	add_protocol_handler_fun(AM300_TOKEN, CMD_CAL_EN, 					(CMD_HANDLER_TYPE)inquire_stim_intensity_handler);
}


void execute_handler(PACKET_Typedef *packet) 
{
	switch(packet->para.Token)
	{
		case AM300_TOKEN: cmd_handler_tab[0][packet->para.Type - TYPE_BASE_ADDR](packet); break;
		case GERNARL_TOKEN: cmd_handler_tab[1][packet->para.Type - TYPE_BASE_ADDR](packet); break;
		default: break;
	}
}

