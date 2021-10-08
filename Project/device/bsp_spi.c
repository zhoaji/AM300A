/**
	@Company		: Shenzhen Creative Industry Co., Ltd.
	@Department	: Embedded Software Group
	@Project		: AM300
	@File				: bsp_spi.c
	@Author			: cms
	@Version		: V0.0.0.1
	@History		: 20210526
		1. 20210526		First editon
		2.
*/

#include "bsp_spi.h"
#include "spi.h"
#include "bsp_gpio.h"

#define SPI_TX_FIFO_NUM 32
#define SPI_RX_FIFO_NUM 32

void spi_config(void)
{
	// SPI Init
  spi_open(HS_SPI0, SPI_MODE_MASTER, SPI_TRANS_MODE_0, 1000000);
	
}

//static spi_dma_t spi_dma = {0};
uint16_t spi_read(void )
{
//	volatile uint32_t spi_stat;
	uint8_t buff[2] = {0};
	
//	pinmux_config(PIN_ADC_MISO,  PINMUX_SPI0_MST_SDA_I_CFG); // MISO
//	pinmux_config(PIN_ADC_CLK,  PINMUX_SPI0_MST_SCK_CFG);    // SCLK
//	pinmux_config(PIN_ADC_CS,  PINMUX_SPI0_MST_CSN_CFG);   	 // CSN 
	
//	spi_open(HS_SPI0, SPI_MODE_MASTER, SPI_TRANS_MODE_0, 200000);
	
//	dma_init();
//	spi_dma_config(HS_SPI0, &spi_dma, NULL);
	
	spi_master_cs_low(HS_SPI0);
	co_delay_us(5);
	spi_master_cs_high(HS_SPI0);
	
	spi_master_exchange(HS_SPI0, 0, buff, 2);
	
	spi_master_cs_low(HS_SPI0);

	spi_master_exchange(HS_SPI0, 0, buff, 2);
	
	spi_master_cs_high(HS_SPI0);
	
	return ((buff[0] << 8) + buff[1]);
}


#include "ll.h"
uint16_t spi_read_data(void)
{
	uint16_t	data = 0;
	
	GLOBAL_INT_STOP();
	
	gpio_write(BITMASK(PIN_ADC_CS), GPIO_LOW);
	co_delay_us(5);
	gpio_write(BITMASK(PIN_ADC_CS), GPIO_HIGH);
	
	for(int i = 0; i < 16; i++)
	{
		gpio_write(BITMASK(PIN_ADC_CLK), GPIO_LOW);
		co_delay_us(2);
		gpio_write(BITMASK(PIN_ADC_CLK), GPIO_HIGH);
		co_delay_us(2);
	}
	gpio_write(BITMASK(PIN_ADC_CLK), GPIO_LOW);
	
	gpio_write(BITMASK(PIN_ADC_CS), GPIO_LOW);	
	
	for(int i = 0; i < 16; i++)
	{
		data <<= 1;
		gpio_write(BITMASK(PIN_ADC_CLK), GPIO_HIGH);
		co_delay_us(2);
		if(gpio_read(BITMASK(PIN_ADC_MISO))) data++;
		gpio_write(BITMASK(PIN_ADC_CLK), GPIO_LOW);
		co_delay_us(2);
	}
	gpio_write(BITMASK(PIN_ADC_CS), GPIO_HIGH);
	
	GLOBAL_INT_START();
	
	return data;
}

