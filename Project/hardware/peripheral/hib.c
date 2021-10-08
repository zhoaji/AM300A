/**
 * @file hib.c
 * @brief 
 * @date Thu, Jan  9, 2020  5:34:14 PM
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"


/*********************************************************************
 * MACROS
 */

#define HIBSPI_BEGIN()  HS_PSO->HIB_SPI_CFG = 0
#define HIBSPI_END()    HS_PSO->HIB_SPI_CFG = 1

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS (base REG read/write)
 */

/**
 * @brief  hib register spi read raw
 *
 * @param[in] addr  addr
 *
 * @return
 **/
uint32_t hib_register_spi_read_raw(volatile uint8_t *reg)
{
    int i;
    uint32_t res = 0;
    uint8_t addr = (uint8_t)(uint32_t)reg;

    HIBSPI_BEGIN();

    // CE
    HS_HIB_SPI->CTRL = 0xF;

    // write
    HS_HIB_SPI->WDATA = 0x00 | addr;
    while(!(HS_HIB_SPI->STATUS & 0x80));
    HS_HIB_SPI->STATUS = 0x80;

    for(i=0; i<4; i++)
    {
        // read
        HS_HIB_SPI->WDATA = 0xFF;
        while(!(HS_HIB_SPI->STATUS & 0x80));
        HS_HIB_SPI->STATUS = 0x80;
        res |= HS_HIB_SPI->RDATA << (i*8);
    }

    // CE
    HS_HIB_SPI->CTRL = 0x40F;

    HIBSPI_END();

    return res;
}

/**
 * @brief  hib register spi write raw
 *
 * @param[in] addr  addr
 * @param[in] wdata  wdata
 **/
void hib_register_spi_write_raw(volatile uint8_t *reg, uint32_t wdata)
{
    int i;
    uint8_t addr = (uint8_t)(uint32_t)reg;

    HIBSPI_BEGIN();

    // CE
    HS_HIB_SPI->CTRL = 0xF;

    // write
    HS_HIB_SPI->WDATA = 0x20 | addr;
    while(!(HS_HIB_SPI->STATUS & 0x80));
    HS_HIB_SPI->STATUS = 0x80;

    for(i=0; i<4; i++)
    {
        // write
        HS_HIB_SPI->WDATA = wdata >> (i*8);
        while(!(HS_HIB_SPI->STATUS & 0x80));
        HS_HIB_SPI->STATUS = 0x80;
    }

    // CE
    HS_HIB_SPI->CTRL = 0x40F;

    HIBSPI_END();
}

/**
 * @brief  hib register spi write
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 * @param[in] value  value
 **/
void hib_register_spi_write(volatile uint8_t *reg, uint32_t mask, uint32_t value)
{
    uint32_t reg_prev;

    reg_prev = hib_register_spi_read_raw(reg);
    reg_prev &= ~mask;
    reg_prev |= mask & value;
    hib_register_spi_write_raw(reg, reg_prev);
}

/**
 * @brief  hib register spi write0
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 **/
void hib_register_spi_write0(volatile uint8_t *reg, uint32_t mask)
{
    hib_register_spi_write_raw(reg, hib_register_spi_read_raw(reg) & ~mask);
}

/**
 * @brief  hib register spi write1
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 **/
void hib_register_spi_write1(volatile uint8_t *reg, uint32_t mask)
{
    hib_register_spi_write_raw(reg, hib_register_spi_read_raw(reg) | mask);
}

/**
 * @brief  hib register spi read
 *
 * @param[in] reg  reg
 * @param[in] mask  mask
 * @param[in] pos  pos
 *
 * @return
 **/
uint32_t hib_register_spi_read(volatile uint8_t *reg, uint32_t mask, uint32_t pos)
{
    return (hib_register_spi_read_raw(reg) & mask) >> pos;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief  hib wakeup pin set
 *
 * @param[in] pin_mask  pin mask (only for PIN0~PIN12)
 * @param[in] trigger_type  trigger type
 **/
void hib_wakeup_pin_set(uint32_t pin_mask, pmu_pin_wakeup_type_t trigger_type)
{
    // clear interrupt and disable func
    HREGW0(&HS_HIB->CONFIG, HIB_CONFIG_GPIO_WAKE_INT_EN_MASK);

    // wait clear ok
    HIBSPI_BEGIN();
    while(HS_HIB_SPI->STATUS & HIBSPI_STATUS_GPIO_INT_EN_FLAG_MASK);
    HIBSPI_END();

    if(trigger_type == PMU_PIN_WAKEUP_DISABLE)
    {
        HREGW0(&HS_HIB->GPIO_WAKEUP, pin_mask);
        HREGW0(&HS_HIB->GPIO_POL, pin_mask);
    }
    else
    {
        uint32_t gpio_wakeup_mask = (pin_mask | HREGRA(&HS_HIB->GPIO_WAKEUP)) & HIB_WAKEUP_PIN_MASK;
        HREGW1(&HS_HIB->CONFIG, HIB_CONFIG_GPIO_WAKE_INT_EN_MASK);
        HREGWA(&HS_HIB->GPIO_WAKEUP, 0);
        HREGWA(&HS_HIB->GPIO_WAKEUP, gpio_wakeup_mask);

        if (trigger_type == PMU_PIN_WAKEUP_LOW_LEVEL)
            HREGW1(&HS_HIB->GPIO_POL, pin_mask);
        else
            HREGW0(&HS_HIB->GPIO_POL, pin_mask);
    }
}

/**
 * @brief  hib wakeup pin get
 *
 * @return wakeup pin mask
 **/
uint32_t hib_wakeup_pin_get(void)
{
    return HREGRA(&HS_HIB->GPIO_WAKEUP) & HIB_WAKEUP_PIN_MASK;
}

/**
 * @brief  hib timer start
 *
 * @param[in] delay_32k  delay 32k
 **/
void hib_timer_start(uint32_t delay_32k)
{
    // stop
    hib_timer_stop();

    // wirte new count
    HREGWA(&HS_HIB->TIMER_CFG, delay_32k);

    // enable
    HREGW1(&HS_HIB->CONFIG, HIB_CONFIG_TIMER_EN_MASK|HIB_CONFIG_TIMER_INT_EN_MASK);
}

/**
 * @brief  hib timer stop
 **/
void hib_timer_stop(void)
{
    // clear count and clear interrupt
    HREGW0(&HS_HIB->CONFIG, HIB_CONFIG_TIMER_EN_MASK|HIB_CONFIG_TIMER_INT_EN_MASK);

    // wait clear ok
    HIBSPI_BEGIN();
    while(HS_HIB_SPI->STATUS & (HIBSPI_STATUS_TIMER_EN_FLAG_MASK|HIBSPI_STATUS_TIMER_INT_EN_FLAG_MASK));
    HIBSPI_END();
}

/**
 * @brief  hib wakeup pin get
 **/
uint32_t hib_is_pin_wakeup(void)
{
    uint32_t pin_mask = 0;

    HIBSPI_BEGIN();

    if (HS_PMU->BASIC & PMU_BASIC_HIB_EN_MASK)
    {
        pin_mask = HS_PMU->GPIO_LATCH;
    }
    else
    {
        if (HS_HIB_SPI->STATUS & HIBSPI_STATUS_GPIO_INT_MASK)
        {
            REGW1(HS_SYS->PINMUX, SYS_PINMUX_GPIO_DIN_HOLD_SEL_MASK);
            pin_mask = (HS_SYS->GPIO_POWER_UP_STATUS ^ HREGRA(&HS_HIB->GPIO_POL)) & HREGRA(&HS_HIB->GPIO_WAKEUP) & HIB_WAKEUP_PIN_MASK;
            REGW0(HS_SYS->PINMUX, SYS_PINMUX_GPIO_DIN_HOLD_SEL_MASK);
        }
    }
    HIBSPI_END();

    return pin_mask;
}

/**
 * @brief  hib is timer wakeup
 *
 * @return is timer
 **/
bool hib_is_timer_wakeup(void)
{
    bool is_timer = false;

    HIBSPI_BEGIN();

    if (HS_HIB_SPI->STATUS & HIBSPI_STATUS_TIMER_INT_MASK)
        is_timer = true;

    HIBSPI_END();

    return is_timer;
}

/** @} */

