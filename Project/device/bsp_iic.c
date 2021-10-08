/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_iic.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_iic.h"

void iic_config(void)
{
	uint8_t txbuf[3] = {0x08, 0x20, 0x48};
	
	pinmux_config(6, PINMUX_I2C_MST_SCK_CFG);
  pinmux_config(5, PINMUX_I2C_MST_SDA_CFG);
	
	pmu_pin_mode_set((1<<6)|(1<<5), PMU_PIN_MODE_OD);

  i2c_open(I2C_MODE_MASTER, 100000);

	i2c_master_write_mem(0xFF, 0x90, 2, txbuf, 3);
	
//	i2c_master_write(0x90, "123456789", 10);
}

/**************************************************************
	@Function 		: iic_start
	@Parameter		: None
	@Description	: 产生I2C起始信号
	@Return				: None
	@Remark				: None
*/
static void iic_start(void)
{
	gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_OUTPUT);   	
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_HIGH); 
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH); 
	co_delay_us(4);
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_LOW);
	co_delay_us(4);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW); 
}

/**************************************************************
	@Function 		: iic_stop
	@Parameter		: None
	@Description	: 产生I2C停止信号
	@Return				: None
	@Remark				: None
*/
static void iic_stop(void)
{
	gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_OUTPUT);
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_LOW);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
	co_delay_us(4);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH);
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_HIGH);
	co_delay_us(4);
}

/**************************************************************
	@Function 		: iic_wait_ack
	@Parameter		: None
	@Description	: 等待I2C应答信号
	@Return				: 0 , 接收应答成功
									1 , 接收应答失败
	@Remark				: None
*/
static uint8_t iic_wait_ack(void)
{
	uint8_t over_time = 0;
	
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_HIGH);
	gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_INPUT);
//	pmu_pin_mode_set(BITMASK(PIN_IIC_SDA),PMU_PIN_MODE_FLOAT);
	co_delay_us(1);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH);
	co_delay_us(1);
	while(gpio_read(BITMASK(PIN_IIC_SDA)))
	{
		if(++over_time >= 20)
		{
			iic_stop();
			return 1;
		}
		co_delay_us(1);
	}
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
	return 0;
}

/**************************************************************
	@Function 		: iic_ack
	@Parameter		: None
	@Description	: 产生I2C应答信号
	@Return				: None
	@Remark				: None
*/
static void iic_ack(void)
{
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
	gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_OUTPUT);
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_LOW);
	co_delay_us(2);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH);
	co_delay_us(2);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
}

/**************************************************************
	@Function 		: iic_not_ack
	@Parameter		: None
	@Description	: 不产生I2C应答信号
	@Return				: None
	@Remark				: None
*/
static void iic_not_ack(void)
{
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
	gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_OUTPUT);
	gpio_write(BITMASK(PIN_IIC_SDA), GPIO_HIGH);
	co_delay_us(2);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH);
	co_delay_us(2);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
}

/**************************************************************
	@Function 		: iic_send_byte
	@Parameter		: 被发送的字节
	@Description	: I2C发送一个字节
	@Return				: None
	@Remark				: None
*/
static void iic_send_byte(uint8_t dat)
{
	uint8_t i = 0;
	
	gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_OUTPUT);
	gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
	for(i = 0; i < 8; i++)
	{
		if((dat & 0x80) >> 7) gpio_write(BITMASK(PIN_IIC_SDA), GPIO_HIGH);
		else gpio_write(BITMASK(PIN_IIC_SDA), GPIO_LOW);
		
		dat <<= 1;
		co_delay_us(2);
		gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH);
		co_delay_us(2);
		gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
		co_delay_us(1);
	}
}

/**************************************************************
	@Function 		: iic_read_byte
	@Parameter		: ack , 是否需要应答
	@Description	: I2C发送一字节数据
	@Return				: 读取到的一字节数据
	@Remark				: None
*/
static uint8_t iic_read_byte(uint8_t ack)
{
	uint8_t dat, i;
	
	gpio_set_direction(BITMASK(PIN_IIC_SDA), GPIO_INPUT);
	for(i = 0; i < 8; i++)
	{
		gpio_write(BITMASK(PIN_IIC_SCK), GPIO_LOW);
		co_delay_us(2);
		gpio_write(BITMASK(PIN_IIC_SCK), GPIO_HIGH);
		dat <<= 1;
		if(gpio_read(BITMASK(PIN_IIC_SDA))) dat++;
		co_delay_us(1);
	}
	
	if(ack) iic_ack();
	else iic_not_ack();
	
	return dat;
}

/**************************************************************
	@Function 		: dac60501_write_register
	@Parameter		: offset , 寄存器地址
									data , 设置的数据
	@Description	: 对DAC60501寄存器进行写操作
	@Return				: None
	@Remark				: None
*/
#include  "ll.h"
uint8_t dac60501_write_register(uint8_t offset, uint16_t data)
{
	GLOBAL_INT_STOP();
	
	iic_start();
	iic_send_byte(DEV_ADDR);
	if(iic_wait_ack()) return 1;
	iic_send_byte(offset);
	if(iic_wait_ack()) return 1;
	iic_send_byte(data >> 8);
	if(iic_wait_ack()) return 1;
	iic_send_byte(data & 0xFF);
	if(iic_wait_ack()) return 1;
	iic_stop();
	co_delay_us(2);
	
	GLOBAL_INT_START();
	
	return 0;
}

/**************************************************************
	@Function 		: dac60501_read_register
	@Parameter		: offset , 寄存器地址
	@Description	: 对DAC60501寄存器进行读操作
	@Return				: 读取到的16位数据
	@Remark				: None
*/
uint16_t dac60501_read_register(uint8_t offset)
{
	uint16_t data;
	
	GLOBAL_INT_STOP();
	iic_start();
	iic_send_byte(DEV_ADDR);
	iic_wait_ack();
	iic_send_byte(offset);
	iic_wait_ack();
	
	iic_start();
	iic_send_byte(DEV_ADDR + 1);
	iic_wait_ack();
	
	data = iic_read_byte(1); // need ack
	data <<= 8;
	data += iic_read_byte(0); // not ack
	
	iic_stop();
	GLOBAL_INT_START();
	return data;
}


/**************************************************************
	@Function 		: dac_out_value_set
	@Parameter		: value , 输出的电流强度，单位：mA
	@Description	: 设置DAC输出值
	@Return				: None
	@Remark				: None
*/
void dac_out_value_set(uint16_t value)
{
	uint16_t cal_value = 0;
//	static uint16_t dac_data = 0;
//	static uint8_t res = 0;
//	static uint16_t dac_data[3] = {0};
	
//	dac_data = dac60501_read_register(DEVID);
	
//	res = 
	
	cal_value = value * 25;//30*4096/5000 = 24.576
	
	dac60501_write_register(DAC_DATA, cal_value << 4);  // 2048 << 4
	
//	dac_data[0] = dac60501_read_register(DAC_DATA);
//	dac_data[1] = dac60501_read_register(DEVID);
//	dac_data[2] = dac60501_read_register(GAIN);
	
	
}

