/**
 * @file sf_sys.c
 * @brief 
 * @date Tue, Jul 16, 2019  2:29:06 PM
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
// xflash as flash memory
#define SFS_XFLASH_HD_PIN   20
#define SFS_XFLASH_CSN_PIN  21
#define SFS_XFLASH_MISO_PIN 22
#define SFS_XFLASH_WP_PIN   23
#define SFS_XFLASH_CLK_PIN  24
#define SFS_XFLASH_MOSI_PIN 25
#define SFS_XFLASH_PWR_PIN  15

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * CONSTANTS
 */
HS_SF_Type * const sfs_regobj_tbl[SFS_LOCATE_NUM] = {HS_SF, HS_SF1};

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
sfs_env_t sfs_env = {
    /* locate     */    SFS_IFLASH,
    /* auto_disen */    false,
#ifdef CONFIG_XIP_FLASH_ALL
    /* xip        */    true,
#else
    /* xip        */    false,
#endif
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief sfs_xflash_quad_pinmux()
 *
 * @return 
 **/
static void sfs_xflash_quad_pinmux(void)
{
    pmu_pin_mode_set(BITMASK(SFS_XFLASH_WP_PIN), PMU_PIN_MODE_PU);
    pmu_pin_mode_set(BITMASK(SFS_XFLASH_HD_PIN), PMU_PIN_MODE_PU);

    pinmux_config(SFS_XFLASH_WP_PIN, PINMUX_SFLASH1_WP_CFG);
    pinmux_config(SFS_XFLASH_HD_PIN, PINMUX_SFLASH1_HD_CFG);
}

/**
 * @brief sfs_xflash_pinmux()
 *
 * @return 
 **/
static void sfs_xflash_pinmux(void)
{
    pmu_pin_mode_set(BITMASK(SFS_XFLASH_CSN_PIN),  PMU_PIN_MODE_PU);
    pmu_pin_mode_set(BITMASK(SFS_XFLASH_MISO_PIN), PMU_PIN_MODE_PD);

    pinmux_config(SFS_XFLASH_CSN_PIN,  PINMUX_SFLASH1_CSN_CFG);
    pinmux_config(SFS_XFLASH_MISO_PIN, PINMUX_SFLASH1_SO_CFG);
    pinmux_config(SFS_XFLASH_CLK_PIN,  PINMUX_SFLASH1_CK_CFG);
    pinmux_config(SFS_XFLASH_MOSI_PIN, PINMUX_SFLASH1_SI_CFG);
}

/**
 * @brief  sfs xflash power on
 **/
static void sfs_xflash_power_on(void)
{
    // mode,driven,pinmux
    pmu_pin_mode_set(BITMASK(SFS_XFLASH_PWR_PIN), PMU_PIN_MODE_PP);
    pmu_pin_driven_current_set(BITMASK(SFS_XFLASH_PWR_PIN), PMU_PIN_DRIVEN_CURRENT_MAX);
    pinmux_config(SFS_XFLASH_PWR_PIN, PINMUX_GPIO_MODE_CFG);

    // init gpio
    gpio_open_clock();
    gpio_write(BITMASK(SFS_XFLASH_PWR_PIN), GPIO_HIGH);
    gpio_set_direction(BITMASK(SFS_XFLASH_PWR_PIN), GPIO_OUTPUT);

    // wait ready
    co_delay_ms(10);
}

/**
 * @brief  sfs xflash power on
 **/
static void sfs_xflash_power_off(void)
{
    // init gpio
    gpio_write(BITMASK(SFS_XFLASH_PWR_PIN), GPIO_LOW);
}

/**
 * @brief  sfs xflash open
 **/
static void sfs_xflash_open(void)
{
    sfs_xflash_power_on();

    sfs_xflash_pinmux();
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief sf raw opration
 *
 * JUST FOR TEST
 *
 * @param[in] locate  0:inside 1:outside 0xFF:auto
 * @param[in] ctrl  1:read 2:write
 * @param[in] cmd  sf command
 * @param[in] cmd_bits  sf command bits
 * @param[in] data  sf data
 * @param[in] data_bytes  sf data bytes
 *
 * @return errno
 **/
void sfs_raw_op(uint8_t locate, uint8_t ctrl,
                uint32_t cmd[2], uint8_t cmd_bits,
                void *data, uint16_t data_bytes)
{
    sfb_rw_params_t param;
    HS_SF_Type *sf = locate < SFS_LOCATE_NUM ? sfs_regobj_tbl[locate] : sfs_regobj_tbl[sfs_env.locate];

    param.cmd[0] = cmd[0];
    param.cmd[1] = cmd[1];
    param.cmd_bits = cmd_bits;
    param.data = data;
    param.data_bytes = data_bytes;

    if(ctrl == 1) // read
        sfb_read_nodma(sf, 0, &param);
    else // write
        sfb_write_nodma(sf, 0, &param);
}

/**
 * @brief config
 *
 * @param[in] freq_hz  frequency in Hz
 * @param[in] width  width
 * @param[in] delay  delay
 **/
void sfs_config(uint32_t freq_hz, sf_width_t width, uint8_t delay)
{
    sf_config_t sfrconfig;
    HS_SF_Type *sf = sfs_regobj_tbl[sfs_env.locate];

    if (width==SF_WIDTH_4LINE && sfs_env.locate==SFS_XFLASH)
        sfs_xflash_quad_pinmux();

    sfrconfig.freq_hz = freq_hz;
    sfrconfig.width = width;
    sfrconfig.delay = delay;
    sf_config(sf, 0, &sfrconfig);
}

/**
 * @brief probe
 *
 * @param[in] locate  locate
 * @param[in] freq_hz  frequency in Hz
 *
 * @return errno
 **/
int sfs_probe(sfs_locate_t locate, uint32_t freq_hz)
{
    sf_config_t sfrconfig;
    HS_SF_Type *sf = sfs_regobj_tbl[locate];
    sf_status_t status = sf_status(sf, 0);
    bool detected;

    if (status == SF_STATUS_ABSENT)
    {
        return -ENODEV;
    }
    else if (status == SF_STATUS_PRESENT)
    {
        sfs_env.locate = locate;
        return 0;
    }

    // pinmux for xflash
    if(locate == SFS_XFLASH)
        sfs_xflash_open();

    // open
    sf_enable(sf, 0);

    // config
    sfrconfig.freq_hz = freq_hz;
    sfrconfig.delay = 5;
    sfrconfig.width = SF_WIDTH_1LINE;
    sf_config(sf, 0, &sfrconfig);

    // detect
    detected = sf_detect(sf, 0);

    if (!detected)
    {
        // try leave lowpower mode
        sf_lowpower_leave(sf, 0);

        // re-detect
        detected = sf_detect(sf, 0);
    }

    if (!detected)
    {
        // disable SF
        sf_disable(sf, 0);
        if(locate == SFS_XFLASH)
            sfs_xflash_power_off();

        // delay 50ms
        co_delay_ms(50);

        // open
        sf_enable(sf, 0);
        if(locate == SFS_XFLASH)
            sfs_xflash_power_on();

        // delay 10ms
        co_delay_ms(10);

        // re-detect
        detected = sf_detect(sf, 0);
    }

    if (detected)
    {
        sfs_env.locate = locate;
        return 0;
    }
    else
    {
        return -ENODEV;
    }
}

/**
 * @brief select
 *
 * @param[in] locate  locate
 *
 * @return errno
 **/
int sfs_select(sfs_locate_t locate)
{
    if(sf_status(sfs_regobj_tbl[locate], 0) != SF_STATUS_PRESENT)
        return -ENODEV;

    sfs_env.locate = locate;
    return 0;
}

/**
 * @brief  sfs is auto disen
 *
 * @return is
 **/
bool sfs_is_auto_disen(void)
{
    return sfs_env.auto_disen;
}

/**
 * @brief  sfs auto disen set
 *
 * @param[in] auto_disen  auto disen
 **/
void sfs_auto_disen_set(bool auto_disen)
{
    sfs_env.auto_disen = auto_disen;
}

/**
 * @brief  sfs is xip
 *
 * @return is
 **/
bool sfs_is_xip(void)
{
    return sfs_env.xip;
}

/**
 * @brief  sfs xip set
 *
 * @param[in] xip  xip
 **/
void sfs_xip_set(bool xip)
{
    sfs_env.xip = xip;
}

/**
 * @brief cache enable
 *
 * @param[in] enable  enable
 **/
void sfs_cache_enable(bool enable)
{
    if(enable)
    {
        HS_HCACHE->CONFIG = 0;
        HS_HCACHE->CTRL = HCACHE_CTRL_CEN_MASK;
        while((HS_HCACHE->STATUS & HCACHE_STATUS_CSTS_MASK) == 0);
    }
    else
    {
        HS_HCACHE->CONFIG = HCACHE_CONFIG_GCLKDIS_MASK;
        HS_HCACHE->CTRL = 0;
        while((HS_HCACHE->STATUS & HCACHE_STATUS_CSTS_MASK) != 0);
    }

#ifndef CONFIG_HS6621C
    // cache auto disable
//    REGW0(&HS_PMU->RAM_PM_1, PMU_PM_ICACHE_POWER_ON_MASK);
#endif
}

/**
 * @brief  sfs cache invalidate all
 **/
void sfs_cache_invalidate_all(void)
{
    HS_HCACHE->MAINT0 = HCACHE_MAINT0_INVALL_MASK;
}

/** @} */

