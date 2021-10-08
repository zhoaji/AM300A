/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: main.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/


#include "rwip_config.h" // RW SW configuration
#include "arch.h"      // architectural platform definitions
#include "rwip.h"      // RW SW initialization
#include "peripheral.h"
#include "sysdump.h"
#include "app.h"

#include "main.h"
#include "protocol.h"
#include "emg_wave.h"
#include "stim_control.h"
#include "bsp_gpio.h"
#include "bsp_key.h"
#include "bsp_adc.h"
#include "bsp_usart.h"
#include "bsp_systick.h"
#include "bsp_battery.h"
#include "bsp_iic.h"
#include "bsp_spi.h"
#include "bsp_timer.h"

uint8_t BLE_RX_Buf[BLE_BUF_LEN] = {0};
QUEUE_U8	BLE_Rx;

//uint8_t BLE_TX_Buf[BLE_BUF_LEN] = {0};
//QUEUE_U8	BLE_Tx;

static co_timer_t adc_sample_timer;

/************************************************
	@Function			: queue_init
	@Description	:	BLE Rx and Tx FIFO queue initialization
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
static void queue_init(void)
{
	// BlE FIFO Inint
	QUEUE_INIT(BLE_Rx, BLE_RX_Buf, BLE_BUF_LEN);
//	QUEUE_INIT(BLE_Tx, BLE_TX_Buf, BLE_BUF_LEN);
	
	emg_init();
	stim_init();
	
}



/************************************************
	@Function			: hardware_init
	@Description	:	PMU peripheral initialization
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
static void hardware_init(void)
{
	// Init systick clock
	SysTick_Config(64000); 
	
	// Init gpio
	gpio_config();
		
	// Init key 
	key_config();
	
	// Init UART
#ifdef CONFIG_LOG_OUTPUT
	usart_config();
#endif
	
	TIM1_config(250);   // 250us   2KHz * 2 = 4KHz
	TIM0_config(50);   
	
//	iic_config();
	spi_config();
	
	gpio_write(BITMASK(PIN_SW_OFF_OR_BAT_EN), GPIO_HIGH);  // enable bat voltage sample
	
//	emg_wave.emg_wave_en = 1;  
//	gpio_write(BITMASK(PIN_EMG_OR_STIM_SW), GPIO_HIGH);  // �̵����е�EMG
//	tim_start(HS_TIM1);
}


/************************************************
	@Function			: adc_sample_timer_handler
	@Description	:	��ص�ѹadc�ɼ���������
	@parameter		: None
	@Return				: None
	@Remark				: 5ms
*/
static void adc_sample_timer_handler(co_timer_t *timer, void *param)
{
	static uint16_t time_1s_cnt = 0;
	static uint16_t time_100ms_cnt = 0;
//	static uint16_t test_cnt = 0;
	
	if(emg_wave.emg_wave_en && (!emg_wave.emg_wave_org_en)
		&& (!stim_a_control.stim_section) && (!stim_a_control.stim_section)) 
		get_emg_lead_off_adc_value();   // EMG�缫����adc�ɼ�  200Hz * 5
	
	if(++time_100ms_cnt >= 20)  // 100ms
	{
		time_100ms_cnt = 0;
		if(emg_wave.emg_wave_en && (!emg_wave.emg_wave_org_en) 
			&& (!stim_a_control.stim_section) && (!stim_a_control.stim_section)) 
			probe_status_packet_send(0x01, 0x01); 
		
		if(emg_wave.emg_wave_en && (stim_a_control.stim_section || stim_a_control.stim_section))
			emg_wave_packet_send(0x9999, 0x9999);
	}
	
	if(++time_1s_cnt >= 200)  // 200 -> 1s
	{
		time_1s_cnt = 0;
		
		get_battery_adc_value();   // ��ص����ɼ�
		if(!emg_wave.emg_wave_org_en && (!stim_a_control.stim_section) && (!stim_a_control.stim_section)) 
			battery_voltage_packet_send();  // 1s ��1�ε�ص�����Ϣ
	}
}

/************************************************
	@Function			: ble_stack_config
	@Description	:	BLE���ֵ����á�PMUʱ��64MHz����
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
static void ble_stack_config(void)
{
	// Disable WDT
	wdt_enable(0);

	// Enable DCDC
	pmu_dcdc_enable(true);

	// Power down xtal32k in deep sleep mode
	pmu_32k_enable_in_deep_sleep(false);

	// Select 32k clock for stack
	//pmu_xtal32k_change_param(15, 1);
	pmu_select_32k(PMU_32K_SEL_RC);

	// xtal32m param
	pmu_xtal32m_change_param(15);

	// ultra sleep mode enable
	co_power_ultra_sleep_mode_enable(false);  // 20210520  true -> false

	// Enable sleep, SWD will be closed.
	co_power_sleep_enable(false); // 20210520  true -> false
	
	// init pmu clock 64MHz    add 20210525
	pmu_xtal32m_x2_startup();
	cpm_set_clock_div(CPM_CPU_CLK, 1);
	cpm_set_clock(CPM_CPU_CLK, 64000000);
}

/************************************************
	@Function			: main
	@Description	:	main function
	@parameter		: None
	@Return				: None
	@Remark				: None
*/
#include "ke_timer.h"
//#include "simple_server.h"
#include "simple_server_task.h"
//#include "prf_utils.h"


//static void test_timer_handler(co_timer_t *timer, void *param)
//{
//	if(gpio_read(BITMASK(PIN_STIM_PWM_H))) gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_LOW);
//	else gpio_write(BITMASK(PIN_STIM_PWM_H), GPIO_HIGH);
//	
//}

int main(void)
{
	queue_init();  
	handler_init();
	
  ble_stack_config();
	
  hardware_init();

  rwip_init(RESET_NO_ERROR);
	
// Remove 20210520    co_power_register_sleep_event(power_sleep_event_handler);

// Remove 20210520    log_debug("running %d\n", pmu_reboot_reason());
	pmu_reboot_reason(); // Add 20210520
	
// Remove 20210520    dbg_mmi_enable();
	
  appm_init();

  co_timer_set(&adc_sample_timer, 5, TIMER_REPEAT, adc_sample_timer_handler, NULL);
	
//	co_timer_set(&adc_sample_timer, 1, TIMER_REPEAT, test_timer_handler, NULL);
	
//	struct simple_server_env_tag* simple_server_env = PRF_ENV_GET(SIMPLE_SERVER, simple_server);
//	simple_server_env->ntf_cfg[0] = PRF_CLI_START_NTF;
//	ke_timer_set(SIMPLE_SERVER_TIMEOUT_TIMER, TASK_APP, 100);
	
//	enable_notification(ENABLE);

  while(1)
  {
//		if(gpio_read(BITMASK(PIN_LED0))) gpio_write(BITMASK(PIN_LED0), GPIO_LOW);
//		else gpio_write(BITMASK(PIN_LED0), GPIO_HIGH);
//		printf("123456\r\n");
    rwip_schedule();		
		
		protocol_handler();
		
		emg_calculate_handler();
		
		stim_control_handler();
  }
}

