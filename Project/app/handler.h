
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

#define GERNARL_TOKEN				0xF0			// ͨ��ָ��

#define CMD_DEBUG_VERSION		0x80		// Debug�汾
#define CMD_VERSION					0x81    // ��ѯ�汾/�豸���� 
#define CMD_SN_INQ					0x82		// ��ѯ���к�
#define CMD_BATVOL					0x83		// ��ѯ��ص���

#define ACK_DEBUG_VERSION		0x00
#define ACK_VERSION					0x01		
#define ACK_SN_INQ					0x02
#define ACK_BATVOL					0x03


#define AM300_TOKEN					0x69		// ���ÿ���ָ��

#define CMD_EN_EMG					0x81		// ��ʼ�ϴ�EMG����
#define CMD_DIS_EMG					0x82		// ֹͣ�ϴ�EMG����

//#define CMD_INTENSITY_SET		0x87		// �̼�ǿ�����ã��£�
//#define CMD_INTENSITY_INQ		0x88		// �̼�ǿ�Ȳ�ѯ���£�

#define CMD_PARA_SET				0x91		// �̼���������
#define CMD_PARA_INQ				0x9D		// �̼�������ѯ		
#define CMD_INTENSITY_SET		0x92		// �̼�ǿ������ ����Э�飩
#define CMD_INTENSITY_INQ		0x9E		// �̼�ǿ�Ȳ�ѯ	����Э�飩
#define CMD_STIM_START			0x93		// ����̼����ƿ�ʼ
#define CMD_STIM_PAUSE			0x94		// ����̼�������ͣ
#define CMD_STIM_STOP				0x95		// ����̼�����ֹͣ
#define CMD_STIM_START1			0x97		// ���紥����̼����ƿ�ʼ
#define CMD_STIM_PAUSE1			0x98		// ���紥����̼�������ͣ
#define CMD_STIM_STOP1			0x99		// ���紥����̼�����ֹͣ
#define CMD_STIM_SINGLE			0x9A		// ���紥����̼����е��δ̼�
#define CMD_MODE_STA				0x9C		// ��ѯ��λ������״̬

#define ACK_EN_EMG					0x01		
#define ACK_DIS_EMG					0x02
#define PACK_EMG_WAVE				0x03		// EMG���ΰ�����Э�� ��ͨ����
//#define PACK_EMG_WAVE				0x05		// EMG���ΰ����� ˫ͨ����
#define	ACK_LEAD_STA				0x04		// �缫����/����״̬������Э�飩
//#define	ACK_LEAD_STA				0x06		// �缫����/����״̬������ ˫ͨ���������ο��缫״̬��

#define ACK_INTENSITY_SET		0x07		// �̼�ǿ�����ûظ����£�
#define ACK_INTENSITY_INQ		0x08		// �̼�ǿ�Ȳ�ѯ�ظ����£�

#define ACK_PARA_SET				0x11		
#define ACK_PARA_INQ				0x1D
//#define ACK_INTENSITY_SET		0x12	// �̼�ǿ�����ûظ����ɣ�
//#define ACK_INTENSITY_INQ		0x1E	// �̼�ǿ�����ûظ����ɣ�
#define ACK_STIM_START			0x13		
#define ACK_STIM_PAUSE			0x14
#define ACK_STIM_STOP				0x15
#define PACK_STIM_STA				0x16		// ����̼�����״̬��
#define ACK_STIM_START1			0x17
#define ACK_STIM_PAUSE1			0x18
#define ACK_STIM_STOP1			0x19	
#define ACK_STIM_SINGLE			0x1A
#define ACK_BAT_LOW					0x1B		// ��ص����ͣ�������ֹ
#define ACK_MODE_STA				0x1C		

// Debug cmd
#define CMD_DEBUG						0xA1		// EMG���Կ�ʼ/ֹͣ
#define CMD_ORG_DATA				0xA4		// ʹ��/����EMGԭʼ�����ϴ�
#define CMD_OFF_DATA				0xA5		// ʹ��/��ֹleadoff�����ϴ�

#define ACK_DEBUG						0x21
#define PACK_ORG_DATA				0x22		// EMGԭʼ�������ݰ�
#define PACK_OFF_DATA				0x23		// EMGԭʼleadoff���ݰ�
#define ACK_ORG_DATA				0x24
#define ACK_OFF_DATA				0x25	

// Product cmd
#define CMD_SN_SET					0xA6		// �����豸���к�
#define CMD_GAIN_SET				0xA7		// �����豸Ӳ������
#define CMD_GAIN_INQ				0xA8		// ��ѯ�豸Ӳ������
#define CMD_CAL_EN					0xA9		// EMG���꿪ʼ/ֹͣ

#define ACK_SN_SET					0x26
#define ACK_GAIN_SET				0x27
#define ACK_GAIN_INQ				0x28
#define ACK_MODE_ERR				0x29		// �豸������Ϣ��
#define ACK_CAL_EN					0x2A
#define PACK_CAL_DATA				0x2B		// EMG���겨�ΰ�

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
