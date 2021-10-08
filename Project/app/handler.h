
/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: handler.h
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#ifndef __HANDLER_H__
#define __HANDLER_H__

#include <stdint.h>

#define TYPE_BASE_ADDR			0x80

#define GERNARL_TOKEN				0xF0			// 通用指令

#define CMD_DEBUG_VERSION		0x80		// Debug版本
#define CMD_VERSION					0x81    // 查询版本/设备名称 
#define CMD_SN_INQ					0x82		// 查询序列号
#define CMD_BATVOL					0x83		// 查询电池电量

#define ACK_DEBUG_VERSION		0x00
#define ACK_VERSION					0x01		
#define ACK_SN_INQ					0x02
#define ACK_BATVOL					0x03


#define AM300_TOKEN					0x69		// 家用康复指令

#define CMD_EN_EMG					0x81		// 开始上传EMG数据
#define CMD_DIS_EMG					0x82		// 停止上传EMG数据

//#define CMD_INTENSITY_SET		0x87		// 刺激强度设置（新）
//#define CMD_INTENSITY_INQ		0x88		// 刺激强度查询（新）

#define CMD_PARA_SET				0x91		// 刺激参数设置
#define CMD_PARA_INQ				0x9D		// 刺激参数查询		
#define CMD_INTENSITY_SET		0x92		// 刺激强度设置 （旧协议）
#define CMD_INTENSITY_INQ		0x9E		// 刺激强度查询	（旧协议）
#define CMD_STIM_START			0x93		// 肌肉刺激治疗开始
#define CMD_STIM_PAUSE			0x94		// 肌肉刺激治疗暂停
#define CMD_STIM_STOP				0x95		// 肌肉刺激治疗停止
#define CMD_STIM_START1			0x97		// 肌电触发电刺激治疗开始
#define CMD_STIM_PAUSE1			0x98		// 肌电触发电刺激治疗暂停
#define CMD_STIM_STOP1			0x99		// 肌电触发点刺激治疗停止
#define CMD_STIM_SINGLE			0x9A		// 肌电触发电刺激进行单次刺激
#define CMD_MODE_STA				0x9C		// 查询下位机工作状态

#define ACK_EN_EMG					0x01		
#define ACK_DIS_EMG					0x02
#define PACK_EMG_WAVE				0x03		// EMG波形包（旧协议 单通道）
//#define PACK_EMG_WAVE				0x05		// EMG波形包（新 双通道）
#define	ACK_LEAD_STA				0x04		// 电极脱落/正常状态包（旧协议）
//#define	ACK_LEAD_STA				0x06		// 电极脱落/正常状态包（新 双通道，不含参考电极状态）

#define ACK_INTENSITY_SET		0x07		// 刺激强度设置回复（新）
#define ACK_INTENSITY_INQ		0x08		// 刺激强度查询回复（新）

#define ACK_PARA_SET				0x11		
#define ACK_PARA_INQ				0x1D
//#define ACK_INTENSITY_SET		0x12	// 刺激强度设置回复（旧）
//#define ACK_INTENSITY_INQ		0x1E	// 刺激强度设置回复（旧）
#define ACK_STIM_START			0x13		
#define ACK_STIM_PAUSE			0x14
#define ACK_STIM_STOP				0x15
#define PACK_STIM_STA				0x16		// 肌肉刺激治疗状态包
#define ACK_STIM_START1			0x17
#define ACK_STIM_PAUSE1			0x18
#define ACK_STIM_STOP1			0x19	
#define ACK_STIM_SINGLE			0x1A
#define ACK_BAT_LOW					0x1B		// 电池电量低，治疗终止
#define ACK_MODE_STA				0x1C		

// Debug cmd
#define CMD_DEBUG						0xA1		// EMG调试开始/停止
#define CMD_ORG_DATA				0xA4		// 使能/禁用EMG原始波形上传
#define CMD_OFF_DATA				0xA5		// 使能/禁止leadoff数据上传

#define ACK_DEBUG						0x21
#define PACK_ORG_DATA				0x22		// EMG原始波形数据包
#define PACK_OFF_DATA				0x23		// EMG原始leadoff数据包
#define ACK_ORG_DATA				0x24
#define ACK_OFF_DATA				0x25	

// Product cmd
#define CMD_SN_SET					0xA6		// 设置设备序列号
#define CMD_GAIN_SET				0xA7		// 设置设备硬件增益
#define CMD_GAIN_INQ				0xA8		// 查询设备硬件增益
#define CMD_CAL_EN					0xA9		// EMG定标开始/停止

#define ACK_SN_SET					0x26
#define ACK_GAIN_SET				0x27
#define ACK_GAIN_INQ				0x28
#define ACK_MODE_ERR				0x29		// 设备故障信息包
#define ACK_CAL_EN					0x2A
#define PACK_CAL_DATA				0x2B		// EMG定标波形包

#define ERROR_ACK						0xF1

typedef union{
	struct  _packet{
		uint8_t Head1;
		uint8_t Head2;
		uint8_t Token;
		uint8_t Length;
		uint8_t Type;
		uint8_t Data[59];
//		uint8_t	CRC;
	}para;
	uint8_t buf[64];
}PACKET_Typedef;

typedef void (* CMD_HANDLER_TYPE)(PACKET_Typedef *);

extern uint8_t old_protocol_en;

void handler_init(void);
void execute_handler(PACKET_Typedef *packet); 
void battery_voltage_packet_send(void);
void ble_send_buff(uint8_t *buff, uint32_t len);
void stim_status_packet_send(uint8_t status_a);
void emg_wave_packet_send(uint16_t emg_a, uint16_t emg_b);
void emg_org_wave_data_packet_send(uint8_t *buff);
void probe_status_packet_send(uint8_t emg_pro_status, uint8_t stim_pro_status);
/*
void inquire_debug_version_handler(PACKET_Typedef *packet);
void inquire_soft_version_handler(PACKET_Typedef *packet);
void inquire_serial_number_handler(PACKET_Typedef *packet);
void inquire_battery_voltage_handler(PACKET_Typedef *packet);

void start_emg_wave_data_handler(PACKET_Typedef *packet);
void stop_emg_wave_data_handler(PACKET_Typedef *packet);
void set_stim_parameter_handler(PACKET_Typedef *packet);
void inquire_stim_parameter_handler(PACKET_Typedef * packet);
void set_stim_intensity_handler(PACKET_Typedef *packet);
void inquire_stim_intensity_handler(PACKET_Typedef *packet);
void start_stim_output_handler(PACKET_Typedef *packet);
*/

#endif
