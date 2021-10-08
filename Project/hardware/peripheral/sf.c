/**
 * @file sf_raw.c
 * @brief 
 * @date Thu, Jul 11, 2019  5:52:51 PM
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
#include "co.h"

/*********************************************************************
 * MACROS
 */
#define SPI_CMD_WREN        0x06u /* Write enable */
#define SPI_CMD_WRDI        0x04u /* Write disable */
#define SPI_CMD_RDSR        0x05u /* Read status register */
#define SPI_CMD_RDSR2       0x35u /* Read status register (high byte) */
#define SPI_CMD_WRSR        0x01u /* Write status register */
#define SPI_CMD_READ        0x03u /* Read data bytes (low frequency) */
#define SPI_CMD_FAST_READ   0x0Bu /* Read data bytes (high frequency) */
#define SPI_CMD_DUAL_READ   0x3Bu /* Dual output fast read: HS6620 doesn't support 0xBB */
#define SPI_CMD_QUAD_READ   0x6Bu /* Quad output fast read: PN25F04C doesn't support 0x6B, but HS6620 doesn't support 0xEB */
#define SPI_CMD_PP          0x02u /* Page program (up to page in 256 bytes) */
#define SPI_CMD_RDID        0x9fu /* Read JEDEC ID */
#define SPI_CMD_RDUID       0x4Bu /* Read UID */

#define SPI_CMD_SE          0x20u /* Sector erase (usually 4KiB) */
#define SPI_CMD_BE_32K      0x52u /* Erase 32KiB block */
#define SPI_CMD_BE_64K      0xD8u /* Erase 64KiB block */
#define SPI_CMD_CE          0x60u /* Erase whole flash chip, or 0xC7 */

#define SPI_CMD_ENDP        0xB9u /* Enter Deep Power-Down */
#define SPI_CMD_EXDP        0xABu /* Exit Deep Power-Down */
#define SPI_CMD_RDSEC       0x48u /* Read Security Registers */
#define SPI_CMD_ERSEC       0x44u /* Erase Security Registers */
#define SPI_CMD_PPSEC       0x42u /* Program Security Registers */

#define SPI_CMD_EQPI        0x38u /* Enable Quad Peripheral Interface: HS6620 doesn't support */

#define SR_WIP              0x01  /* Write in progress */
#define SR_WEL              0x02  /* Write enable latch */
#define SR_BP0              0x04  /* Block protect 0 */
#define SR_BP1              0x08  /* Block protect 1 */
#define SR_BP2              0x10  /* Block protect 2 */
#define SR_SRP              0x80  /* SR write protect */
#define SR_QE               0x0200 /* Quad enable */

#define SF_READ_FAST_DMA_CLOCK_FREQ_HZ_THRESHOLD    20000000 // 20MHz

#define SF_WORKAROUND_PUYA_FLASH_OVERWRITE_ISSUE
#define SF_NOT_MODIFY_SF0CS0_LAST_SECTOR

#define SF_RW_PARAM_CMD_DECLARE(name, cmd, data, len) \
    sfb_rw_params_t name = {{{((cmd)<<24), 0}}, 8, (data), (len)}
#define SF_RW_PARAM_CMD_ADDR_DECLARE(name, cmd, addr, data, len) \
    sfb_rw_params_t name = {{{((cmd)<<24)|(addr), 0}}, 32, (data), (len)}
#define SF_RW_PARAM_CMD_ADDR_40BITS_DECLARE(name, cmd, addr, data, len) \
    sfb_rw_params_t name = {{{((cmd)<<24)|(addr), 0}}, 40, (data), (len)}

#define IFLASH_INFO (&sf_env.info[0][0])

#define SF_WRITE_BEGIN(sf, cs)  do{uint32_t irq_save=0; if(sfb_critical_object_get()==sf) CO_DISABLE_IRQ_EX_EXCEPT_HIGHEST(irq_save); sf_write_enable(sf, cs);
#define SF_WRITE_END(sf, cs)    sf_wait_sr_no_busy(sf, cs); if(sfb_critical_object_get()==sf) CO_RESTORE_IRQ_EX_EXCEPT_HIGHEST(irq_save);} while(0)


/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
    SF_MID_NONE        = 0,

    SF_MID_PUYA        = 0x85,    /* Puya */
    SF_MID_BOYA_0      = 0xE0,    /* Boya */
    SF_MID_BOYA_1      = 0x68,    /* Boya */
    SF_MID_TONGXIN     = 0xEB,    /* Tongxin */
    SF_MID_XTX         = 0x0B,    /* XTX */
    SF_MID_ALLIANCE    = 0x52,    /* Alliance Semiconductor */
    SF_MID_AMD         = 0x01,    /* AMD */
    SF_MID_AMIC        = 0x37,    /* AMIC */
    SF_MID_ATMEL       = 0x1F,    /* Atmel (now used by Adesto) */
    SF_MID_BRIGHT      = 0xAD,    /* Bright Microelectronics */
    SF_MID_CATALYST    = 0x31,    /* Catalyst */
    SF_MID_ESMT        = 0x8C,    /* Elite Semiconductor Memory Technology (ESMT) / EFST Elite Flash Storage */
    SF_MID_EON         = 0x1C,    /* EON, missing 0x7F prefix */
    SF_MID_EXCEL       = 0x4A,    /* ESI, missing 0x7F prefix */
    SF_MID_FIDELIX     = 0xF8,    /* Fidelix */
    SF_MID_FUJITSU     = 0x04,    /* Fujitsu */
    SF_MID_GIGADEVICE  = 0xC8,    /* GigaDevice */
    SF_MID_GIGADEVICE_XD = 0x50,  /* GigaDevice */
    SF_MID_GIGADEVICE_MD = 0x51,  /* GigaDevice */
    SF_MID_HYUNDAI     = 0xAD,    /* Hyundai */
    SF_MID_IMT         = 0x1F,    /* Integrated Memory Technologies */
    SF_MID_INTEL       = 0x89,    /* Intel */
    SF_MID_ISSI        = 0xD5,    /* ISSI Integrated Silicon Solutions, see also PMC. */
    SF_MID_MACRONIX    = 0xC2,    /* Macronix (MX) */
    SF_MID_NANTRONICS  = 0xD5,    /* Nantronics, missing prefix */
    SF_MID_PMC         = 0x9D,    /* PMC, missing 0x7F prefix */
    SF_MID_SANYO       = 0x62,    /* Sanyo */
    SF_MID_SHARP       = 0xB0,    /* Sharp */
    SF_MID_SPANSION    = 0x01,    /* Spansion, same ID as AMD */
    SF_MID_SST         = 0xBF,    /* SST */
    SF_MID_ST          = 0x20,    /* ST / SGS/Thomson / Numonyx (later acquired by Micron) */
    SF_MID_SYNCMOS_MVC = 0x40,    /* SyncMOS (SM) and Mosel Vitelic Corporation (MVC) */
    SF_MID_TI          = 0x97,    /* Texas Instruments */
    SF_MID_WINBOND_NEX = 0xEF,    /* Winbond (ex Nexcom) serial flashes */
    SF_MID_WINBOND     = 0xDA,    /* Winbond */
}sf_mid_type_t;

typedef struct
{
    uint32_t freq_hz;
    sf_width_t width;
    sf_mid_type_t mid;  // Manufacturer ID
    uint8_t mtid;       // Memory Type ID
    uint8_t cid;        // Capacity ID (shift value)
    sf_status_t status;
}sf_info_t;

typedef struct
{
    // Auto power manage
#ifndef CONFIG_WITHOUT_CO_TIMER
    uint32_t iflash_auto_close_delay_ms;
    co_timer_t iflash_auto_close_timer;
#endif
    // Power on delay
    uint32_t iflash_extra_open_delay_10us;
    // info
    sf_info_t info[SFB_MODULE_NUM][SFB_CS_NUM];
}sf_env_t;

typedef void (* sf_rw_func_t)(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static sf_env_t sf_env = {0};

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief  sf iflash pin input enable
 *
 * @param[in] enable  enable
 **/
static void sf_iflash_pin_input_enable(bool enable)
{
    if(enable)
        REGW1(&HS_PMU->GPIO_IE_CTRL_1, PMU_GPIO_SF_CTRL_5_0_MASK);
    else
        REGW0(&HS_PMU->GPIO_IE_CTRL_1, PMU_GPIO_SF_CTRL_5_0_MASK);
}

/**
 * @brief sf_iflash_power_enable()
 *
 * @param[in] enable  
 *
 * @return 
 **/
static void sf_iflash_power_enable(bool enable)
{
    if(enable)
    {
        if (!(HS_PMU->SW_STATUS & PMU_SW_STATUS_FLASH_OPENED_MASK))
        {
            // flash io IE enable
            sf_iflash_pin_input_enable(true);
            // open flash power
            REGW0(&HS_PMU->ANA_PD, PMU_ANA_PD_FLASH_LDO_MASK);
            // flag
            REGW1(&HS_PMU->SW_STATUS, PMU_SW_STATUS_FLASH_OPENED_MASK);

#ifndef CONFIG_LAGACY_DESIGN
            // wait flash power ready
            while(!(HS_PMU->FLASH_LOW_VOL_CTRL_0 & PMU_FLASH_POWER_READY_SYNC_MASK)){}
#endif

#if defined(CONFIG_HS6621C) && (CONFIG_HARDWARE_VERSION>=HARDWARE_VERSION_B(1))
            // retention in HIB mode
            uint32_t preset_delay_50us = HREGR(&HS_HIB->SW_STATUS, MASK_POS(HIB_SW_STATUS_FLASH_PWRON_DELAY_50US));
            if (preset_delay_50us)
            {
                co_delay_10us(preset_delay_50us * 5);
            }
            else
#endif
            {
                bool delayed = false;

                if (IFLASH_INFO->status == SF_STATUS_PRESENT)
                {
                    switch(IFLASH_INFO->mid)
                    {
                        case SF_MID_PUYA:
                        case SF_MID_TONGXIN:
                            if (IFLASH_INFO->mtid == 0x60)
                            {
                                // tVSL = 70us
                                co_delay_10us(20);
                                delayed = true;
                            }
                            break;

                        default:
                            break;
                    }
                }

                // worst case (tVSL, tPUW)
                if (delayed)
                    co_delay_10us(sf_env.iflash_extra_open_delay_10us);
                else
                    co_delay_ms(10);
            }
        }
    }
    else
    {
        // flash io IE disable
        sf_iflash_pin_input_enable(false);
        // power off
        REGW1(&HS_PMU->ANA_PD, PMU_ANA_PD_FLASH_LDO_MASK);
        // flag
        REGW0(&HS_PMU->SW_STATUS, PMU_SW_STATUS_FLASH_OPENED_MASK);
    }
}

#ifndef CONFIG_WITHOUT_CO_TIMER
/**
 * @brief sf_iflash_auto_close_timeout_handler()
 *
 * @param[in] timer  
 * @param[in] param  
 *
 * @return 
 **/
static void sf_iflash_auto_close_timeout_handler(co_timer_t *timer, void *param)
{
    sf_disable(HS_SF, 0);
}

/**
 * @brief iflash is HS_SF
 *
 * @param[in] enable  
 *
 * @return 
 **/
static void sf_iflash_auto_close_timer_enable(bool enable)
{
    if(enable)
        co_timer_set(&sf_env.iflash_auto_close_timer, sf_env.iflash_auto_close_delay_ms,
                TIMER_ONE_SHOT, sf_iflash_auto_close_timeout_handler, NULL);
    else
        co_timer_del(&sf_env.iflash_auto_close_timer);
}
#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief read sr reg
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sr value
 **/
uint8_t sf_read_sr(HS_SF_Type *sf, uint32_t cs)
{
    uint8_t sr = 0;
    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_RDSR, &sr, 1);
    sfb_read_nodma(sf, cs, &param);
    return sr;
}

/**
 * @brief read sr2
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sr2 value
 **/
uint8_t sf_read_sr2(HS_SF_Type *sf, uint32_t cs)
{
    uint8_t sr2 = 0;
    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_RDSR2, &sr2, 1);
    sfb_read_nodma(sf, cs, &param);
    if (sr2 == 0xFF) sr2 = 0;
    return sr2;
}

/**
 * @brief read sr 16bits
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sr | (sr2<<8)
 **/
uint16_t sf_read_sr_16bits(HS_SF_Type *sf, uint32_t cs)
{
    return sf_read_sr(sf, cs) | (sf_read_sr2(sf, cs) << 8);
}

/**
 * @brief wait sr no busy
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_wait_sr_no_busy(HS_SF_Type *sf, uint32_t cs)
{
    while (1)
    {
        if ((sf_read_sr(sf, cs) & SR_WIP) == 0)
            break;
        co_delay_10us(10);
    }
}

/**
 * @brief write enable
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_write_enable(HS_SF_Type *sf, uint32_t cs)
{
    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_WREN, NULL, 0);
    sfb_write_nodma(sf, cs, &param);
}

/**
 * @brief write sr
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] sr  sr
 **/
void sf_write_sr(HS_SF_Type *sf, uint32_t cs, uint8_t sr)
{
    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_WRSR, &sr, 1);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief write sr 16bits
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] sr  sr
 **/
void sf_write_sr_16bits(HS_SF_Type *sf, uint32_t cs, uint16_t sr)
{
    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_WRSR, &sr, 2);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief write sr with mask
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] mask  sr mask
 * @param[in] value  sr value
 **/
void sf_write_sr_mask(HS_SF_Type *sf, uint32_t cs, uint8_t mask, uint8_t value)
{
    uint8_t sr = sf_read_sr(sf, cs);
    value &= mask;
    if((sr & mask) != value)
        sf_write_sr(sf, cs, (sr & ~mask) | value);
}

/**
 * @brief write 16bits sr with mask
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] mask  sr mask
 * @param[in] value  sr value
 **/
void sf_write_sr_mask_16bits(HS_SF_Type *sf, uint32_t cs, uint16_t mask, uint16_t value)
{
    uint16_t sr = sf_read_sr_16bits(sf, cs);
    value &= mask;
    if((sr & mask) != value)
        sf_write_sr_16bits(sf, cs, (sr & ~mask) | value);
}

/**
 * @brief quad enable
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] enable  true or false
 **/
void sf_quad_enable(HS_SF_Type *sf, uint32_t cs, bool enable)
{
    sf_write_sr_mask_16bits(sf, cs, SR_QE, enable ? SR_QE : 0);
}

/**
 * @brief otp set
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] lb_mask  lb mask
 **/
void sf_otp_set(HS_SF_Type *sf, uint32_t cs, uint8_t lb_mask)
{
    uint32_t sr;
    int sf_index = sfb_index(sf);
    sf_mid_type_t mid = sf_env.info[sf_index][cs].mid;
    uint8_t mtid = sf_env.info[sf_index][cs].mtid;

    sr = sf_read_sr_16bits(sf, cs);

    if (mid == SF_MID_GIGADEVICE && mtid == 0x40)
        sr |= (lb_mask & 0x1) << (2+8);
    else
        sr |= (lb_mask & 0x7) << (3+8);

    sf_write_sr_16bits(sf, cs, sr);
}

/**
 * @brief otp get
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return lb_mask
 **/
uint8_t sf_otp_get(HS_SF_Type *sf, uint32_t cs)
{
    uint32_t sr2 = sf_read_sr2(sf, cs);
    int sf_index = sfb_index(sf);
    sf_mid_type_t mid = sf_env.info[sf_index][cs].mid;
    uint8_t mtid = sf_env.info[sf_index][cs].mtid;

    if (mid == SF_MID_GIGADEVICE && mtid == 0x40)
        return (sr2 >> 2) & 0x1;
    else
        return (sr2 >> 3) & 0x7;
}

/**
 * @brief  sf lowpower enter
 *
 * @param[in] sf  sf
 * @param[in] cs  cs
 **/
void sf_lowpower_enter(HS_SF_Type *sf, uint32_t cs)
{
    int sf_index = sfb_index(sf);
    bool delayed = false;

    // enter
    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_ENDP, NULL, 0);
    sfb_write_nodma(sf, cs, &param);

    // tDP
    if (sf_env.info[sf_index][cs].status == SF_STATUS_PRESENT)
    {
        switch(sf_env.info[sf_index][cs].mid)
        {
            case SF_MID_PUYA:
            case SF_MID_TONGXIN:
                // tDP = 3us
                co_delay_us(3*2);
                delayed = true;
                break;

            default:
                break;
        }
    }

    if(!delayed)
        co_delay_us(20);

    // pin input disable: avoid pin leakage
    sf_iflash_pin_input_enable(false);
}

/**
 * @brief  sf lowpower leave
 *
 * @param[in] sf  sf
 * @param[in] cs  cs
 **/
void sf_lowpower_leave(HS_SF_Type *sf, uint32_t cs)
{
    uint8_t res;
    int sf_index = sfb_index(sf);
    bool delayed = false;

    // pin input enable
    sf_iflash_pin_input_enable(true);
    co_delay_us(1);

    // leave
    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_EXDP, 0, &res, 1);
    sfb_write_nodma(sf, cs, &param);

    // tRES2
    if (sf_env.info[sf_index][cs].status == SF_STATUS_PRESENT)
    {
        switch(sf_env.info[sf_index][cs].mid)
        {
            case SF_MID_PUYA:
            case SF_MID_TONGXIN:
                // tDP = 8us
                co_delay_us(8*2);
                delayed = true;
                break;

            default:
                break;
        }
    }

    if(!delayed)
        co_delay_us(30);
}

/**
 * @brief unlock all
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_unlock_all(HS_SF_Type *sf, uint32_t cs)
{
    int sf_index = sfb_index(sf);
    sf_mid_type_t mid = sf_env.info[sf_index][cs].mid;
    uint16_t bpx, cmp=0;

    /* cancel Complement Protect if set */
    switch(mid)
    {
        case SF_MID_GIGADEVICE:
        case SF_MID_BOYA_0:
        case SF_MID_WINBOND_NEX:
        case SF_MID_PUYA:
            cmp = 1 << 14;
            break;
        default:
            break;
    }

    /* cancel Block Protect if set */
    switch(mid)
    {
        case SF_MID_PUYA:
        case SF_MID_GIGADEVICE: bpx = 0x1F << 2; break;
        case SF_MID_EON:        bpx = 0x0F << 2; break;
        default:                 bpx =  0x7 << 2; break;
    }

    if(cmp)
        sf_write_sr_mask_16bits(sf, cs, cmp|bpx, 0);
    else
        sf_write_sr_mask(sf, cs, bpx, 0);
}

/**
 * @brief read id
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sflash id (24bits)
 **/
uint32_t sf_read_id(HS_SF_Type *sf, uint32_t cs)
{
    uint32_t id = 0;
    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_RDID, &id, 3);
    sfb_read_nodma(sf, cs, &param);
    id = ((id & 0xFF0000) >> 16) | (id & 0x00FF00) | ((id & 0x0000FF) << 16);
    return id;
}

/**
 * @brief read uid
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] data  read uid buffer
 * @param[in] length  length
 **/
void sf_read_uid_ex(HS_SF_Type *sf, uint32_t cs, void *data, uint32_t length)
{
    SF_RW_PARAM_CMD_ADDR_40BITS_DECLARE(param, SPI_CMD_RDUID, 0, data, length);
    sfb_read_nodma(sf, cs, &param);
}

/**
 * @brief read uid
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sflash UID
 **/
uint32_t sf_read_uid(HS_SF_Type *sf, uint32_t cs)
{
    uint32_t uid, uidbuf[4];
    int sf_index = sfb_index(sf);

    switch (sf_env.info[sf_index][cs].mid)
    {
        // Puya: 128bit UID
        case SF_MID_PUYA:
        case SF_MID_TONGXIN:
            sf_read_uid_ex(sf, cs, uidbuf, 16);
            uid = uidbuf[0] ^ uidbuf[1] ^ uidbuf[2] ^ uidbuf[3];
            break;

        // Boya: 64bit UID
        case SF_MID_BOYA_0:
        case SF_MID_BOYA_1:
            sf_read_uid_ex(sf, cs, uidbuf, 8);
            uid = uidbuf[0] ^ uidbuf[1];
            break;

        default:
            uid = sf_read_id(sf, cs);
            break;
    }

    return uid;
}

/**
 * @brief erase chip
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_erase_chip(HS_SF_Type *sf, uint32_t cs)
{
    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_DECLARE(param, SPI_CMD_CE, NULL, 0);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief erase sector
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 **/
void sf_erase_sector(HS_SF_Type *sf, uint32_t cs, uint32_t addr)
{
    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_SE, addr, NULL, 0);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief erase block
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 **/
void sf_erase_block(HS_SF_Type *sf, uint32_t cs, uint32_t addr)
{
    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_BE_64K, addr, NULL, 0);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief erase sec
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 **/
void sf_erase_sec(HS_SF_Type *sf, uint32_t cs, uint32_t addr)
{
    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_ERSEC, addr, NULL, 0);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief sf erase
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] length  length
 **/
void sf_erase(HS_SF_Type *sf, uint32_t cs, uint32_t addr, uint32_t length)
{
    uint32_t sf_index = sfb_index(sf);
    uint32_t capacity = 1u << sf_env.info[sf_index][cs].cid;
    uint32_t sector, sector_end;

    if(length == 0)
    {
        // Erase ALl Flash

        sector_end = capacity;

#ifdef SF_NOT_MODIFY_SF0CS0_LAST_SECTOR
        if(sf==HS_SF && cs==0)
            sector_end = capacity - SF_SECTOR_SIZE;
#endif

        for (sector=SF_SECTOR_SIZE; sector<sector_end; sector+=SF_SECTOR_SIZE)
            sf_erase_sector(sf, cs, sector);

        sf_erase_sector(sf, cs, 0);
    }
    else
    {
        sector_end = addr + length;

        if (sector_end > capacity)
            sector_end = capacity;

#ifdef SF_NOT_MODIFY_SF0CS0_LAST_SECTOR
        if(sf==HS_SF && cs==0)
            if (sector_end > capacity-SF_SECTOR_SIZE)
                sector_end = capacity-SF_SECTOR_SIZE;
#endif

        for (sector = addr & ~(SF_SECTOR_SIZE-1); sector<sector_end; sector+=SF_SECTOR_SIZE)
            sf_erase_sector(sf, cs, sector);
    }
}

/**
 * @brief write page without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_page_nodma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length)
{
    co_assert(((addr & (SF_PAGE_SIZE-1)) + length) <= SF_PAGE_SIZE);

    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_PP, addr, data, length);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief write page with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_page_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length)
{
    co_assert((length & 3) == 0);
    co_assert(((uint32_t)data & 3) == 0);

    co_assert(((addr & (SF_PAGE_SIZE-1)) + length) <= SF_PAGE_SIZE);

    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_PP, addr, data, length);
    sfb_write_dma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief write page
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_page(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length)
{
    uint32_t tlen;
    const uint8_t *src = data;
    uint32_t dst = addr;

    if(addr & 0x08000000)
    {
        sf_write_page_nodma(sf, cs, addr, data, length);
        return;
    }

    tlen = (uint32_t)data & 3;
    if (tlen)
    {
        tlen = 4 - tlen;
        if (tlen > length)
            tlen = length;
        sf_write_page_nodma(sf, cs, dst, src, tlen);
        src += tlen;
        dst += tlen;
        length -= tlen;
    }

    if(length)
    {
        tlen = length & 3;
        length -= tlen;

        if(length)
        {
            sf_write_page_dma(sf, cs, dst, src, length);
            src += length;
            dst += length;
        }

        if (tlen)
            sf_write_page_nodma(sf, cs, dst, src, tlen);
    }
}

/**
 * @brief write sec
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_sec(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length)
{
    SF_WRITE_BEGIN(sf, cs);

    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_PPSEC, addr, data, length);
    sfb_write_nodma(sf, cs, &param);

    SF_WRITE_END(sf, cs);
}

/**
 * @brief sf write
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length)
{
    uint32_t page_offset;
    uint32_t tlen;
    uint32_t i;

    co_assert(data != NULL);
    co_assert(length != 0);

#ifdef SF_WORKAROUND_PUYA_FLASH_OVERWRITE_ISSUE
    // Workaround PUYA flash(0x856013,0x856014) BUG:
    // Same place write more than 1 times, some bit many from 0 to 1.
    // Only cover 1,2 bytes over-write
    uint8_t write_data[4];
    if(length < 3)
    {
        uint8_t flash_data[4];

        sf_read_normal_nodma(sf, cs, addr, flash_data, length);

        for(i=0; i<length; ++i)
            write_data[i] = ((uint8_t *)data)[i] & flash_data[i];

        data = write_data;
    }
#endif

#ifdef SF_NOT_MODIFY_SF0CS0_LAST_SECTOR
    if(sf==HS_SF && cs==0)
    {
        uint32_t sf_index = sfb_index(sf);
        uint32_t capacity = 1u << sf_env.info[sf_index][cs].cid;
        uint32_t last_sector = capacity - SF_SECTOR_SIZE;
        if(addr+length > last_sector)
        {
            if (addr >= last_sector)
                return;
            length = last_sector - addr;
        }
    }
#endif

    page_offset = addr & (SF_PAGE_SIZE - 1);
    tlen = SF_PAGE_SIZE - page_offset;

    if (tlen > length)
        tlen = length;

    sf_write_page(sf, cs, addr, data, tlen);

    for (i=tlen; i<length; i+=SF_PAGE_SIZE)
    {
        tlen = length - i;
        if(tlen > SF_PAGE_SIZE)
            tlen = SF_PAGE_SIZE;
        sf_write_page(sf, cs, addr+i, (uint8_t *)data+i, tlen);
    }
}

/**
 * @brief read normal without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_normal_nodma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_READ, addr, data, length);
    sfb_read_nodma(sf, cs, &param);
}

/**
 * @brief read normal with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_normal_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    co_assert((length & 3) == 0);
    co_assert(((uint32_t)data & 3) == 0);

    SF_RW_PARAM_CMD_ADDR_DECLARE(param, SPI_CMD_READ, addr, data, length);
    sfb_read_dma(sf, cs, &param);
}

/**
 * @brief read fast with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    co_assert((length & 3) == 0);
    co_assert(((uint32_t)data & 3) == 0);

    SF_RW_PARAM_CMD_ADDR_40BITS_DECLARE(param, SPI_CMD_FAST_READ, addr, data, length);
    sfb_read_dma(sf, cs, &param);
}

/**
 * @brief read fast 2line with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_dual_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    co_assert((length & 7) == 0);
    co_assert(((uint32_t)data & 7) == 0);

    SF_RW_PARAM_CMD_ADDR_40BITS_DECLARE(param, SPI_CMD_DUAL_READ, addr, data, length);
    sfb_read_dma(sf, cs, &param);
}

/**
 * @brief read fast 4line with dma (Not auto enable quad mode, please call sf_quad_enable outside)
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_quad_naked_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    co_assert((length & 15) == 0);
    co_assert(((uint32_t)data & 15) == 0);

    SF_RW_PARAM_CMD_ADDR_40BITS_DECLARE(param, SPI_CMD_QUAD_READ, addr, data, length);
    sfb_read_dma(sf, cs, &param);
}

/**
 * @brief read fast 4line with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_quad_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    sf_quad_enable(sf, cs, true);

    sf_read_fast_quad_naked_dma(sf, cs, addr, data, length);

    sf_quad_enable(sf, cs, false);
}

/**
 * @brief read sec
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_sec(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    SF_RW_PARAM_CMD_ADDR_40BITS_DECLARE(param, SPI_CMD_RDSEC, addr, data, length);
    sfb_read_nodma(sf, cs, &param);
}

/**
 * @brief sf read
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length)
{
    uint32_t tlen, once_dma_length;
    sf_rw_func_t rw_dma_func;
    uint32_t src = addr;
    uint8_t * dst = data;
    int sf_index = sfb_index(sf);
    sf_width_t width = sf_env.info[sf_index][cs].width;
    uint32_t align_size = width==SF_WIDTH_4LINE ? 16 :
                          width==SF_WIDTH_2LINE ? 8 : 4;

    co_assert(data != NULL);
    co_assert(length != 0);

    tlen = (uint32_t)data & (align_size - 1);
    if (tlen)
    {
        tlen = align_size - tlen;
        if(tlen > length)
            tlen = length;
        sf_read_normal_nodma(sf, cs, src, dst, tlen);
        src += tlen;
        dst += tlen;

        length -= tlen;
    }

    if (length)
    {
        tlen = length & (align_size - 1);
        length -= tlen;

        if (length)
        {
            switch(width)
            {
                case SF_WIDTH_2LINE: rw_dma_func = sf_read_fast_dual_dma; break;
                case SF_WIDTH_4LINE: rw_dma_func = sf_read_fast_quad_dma; break;
                case SF_WIDTH_1LINE:
                default:
                    if(sf_env.info[sf_index][cs].freq_hz >= SF_READ_FAST_DMA_CLOCK_FREQ_HZ_THRESHOLD)
                        rw_dma_func = sf_read_fast_dma;
                    else
                        rw_dma_func = sf_read_normal_dma;
                    break;
            }

            while(length)
            {
                once_dma_length = length>SFB_DMA_DATA_LEN_MAX ? SFB_DMA_DATA_LEN_MAX : length;

                rw_dma_func(sf, cs, src, dst, once_dma_length);
                src += once_dma_length;
                dst += once_dma_length;

                length -= once_dma_length;
            }
        }

        if (tlen)
            sf_read_normal_nodma(sf, cs, src, dst, tlen);
    }
}

/**
 * @brief sf config
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  sf config
 **/
void sf_config(HS_SF_Type *sf, uint32_t cs, const sf_config_t *config)
{
    sfb_config_t sfbconfig;
    int sf_index = sfb_index(sf);

    // config
    sfbconfig.freq_hz = config->freq_hz;
    sfbconfig.delay = config->delay;
    sfbconfig.transmode = SFB_SPI_MODE_0;
    sfbconfig.cs_pol = SFB_SPI_CS_LOW_ACTIVE;
    sfb_config(sf, cs, &sfbconfig);

    sf_env.info[sf_index][cs].width = config->width;
    sf_env.info[sf_index][cs].freq_hz = config->freq_hz;
}

/**
 * @brief sf enable
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_enable(HS_SF_Type *sf, uint32_t cs)
{
    // enable
    sfb_enable(sf, cs);
    // power on
    if (sf==HS_SF && cs==0)
    {
        sf_iflash_power_enable(true);
#ifndef CONFIG_WITHOUT_CO_TIMER
        if(sf_env.iflash_auto_close_delay_ms)
            sf_iflash_auto_close_timer_enable(true);
#endif
    }
}

/**
 * @brief  sf disable
 *
 * @param[in] sf  sf
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_disable(HS_SF_Type *sf, uint32_t cs)
{
    // TODO: Better way to call sfb_close()

    // power on
    if (sf==HS_SF && cs==0)
    {
        sf_iflash_power_enable(false);
#ifndef CONFIG_WITHOUT_CO_TIMER
        if(sf_env.iflash_auto_close_delay_ms)
            sf_iflash_auto_close_timer_enable(false);
#endif
    }
}

#ifndef CONFIG_WITHOUT_CO_TIMER
/**
 * @brief sf iflash auto close
 *
 * @param[in] delay_ms  
 **/
void sf_iflash_auto_close(uint32_t delay_ms)
{
    sf_env.iflash_auto_close_delay_ms = delay_ms;
    sf_iflash_auto_close_timer_enable(delay_ms==0 ? false : true);
}
#endif

/**
 * @brief  sf iflash open extra delay set
 *
 * @param[in] delay_10us  delay 10us
 **/
void sf_iflash_extra_open_delay_set(uint32_t delay_10us)
{
    sf_env.iflash_extra_open_delay_10us = delay_10us;
}

/**
 * @brief sf detect
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return is present ?
 **/
bool sf_detect(HS_SF_Type *sf, uint32_t cs)
{
    uint32_t id, id2;
    int sf_index = sfb_index(sf);

    id = sf_read_id(sf, cs);
    id2 = sf_read_id(sf, cs);

    // Two consecutive readings are the same
    while(id != id2)
    {
        co_delay_10us(10);
        id = sf_read_id(sf, cs);
        id2 = sf_read_id(sf, cs);
    }

    if(id == 0x00FFFFFF || id == 0)
    {
        sf_env.info[sf_index][cs].mid  = SF_MID_NONE;
        sf_env.info[sf_index][cs].mtid = 0;
        sf_env.info[sf_index][cs].cid  = 0;
        sf_env.info[sf_index][cs].status = SF_STATUS_ABSENT;
        return false;
    }
    else
    {
        sf_env.info[sf_index][cs].mid  = (sf_mid_type_t)((id >> 16) & 0xFF);
        sf_env.info[sf_index][cs].mtid = (id >> 8)  & 0xFF;
        sf_env.info[sf_index][cs].cid  = (id >> 0)  & 0xFF;
        sf_env.info[sf_index][cs].status = SF_STATUS_PRESENT;

        // default disable known quad mode
        switch(sf_env.info[sf_index][cs].mid)
        {
            case SF_MID_GIGADEVICE:
            case SF_MID_WINBOND_NEX:
            case SF_MID_BOYA_0:
            case SF_MID_PUYA:
                sf_quad_enable(sf, cs, false);
                break;

            default:
                break;
        }
        return true;
    }
}

/**
 * @brief sf status
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return status
 **/
sf_status_t sf_status(HS_SF_Type *sf, uint32_t cs)
{
    return sf_env.info[sfb_index(sf)][cs].status;
}

/**
 * @brief sf capacity
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return capacity
 **/
uint32_t sf_capacity(HS_SF_Type *sf, uint32_t cs)
{
    int sf_index = sfb_index(sf);

    return 1u << sf_env.info[sf_index][cs].cid;
}

/**
 * @brief sf get id
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sflash id (saved by @ref sf_detect)
 **/
uint32_t sf_id(HS_SF_Type *sf, uint32_t cs)
{
    int sf_index = sfb_index(sf);
    return (sf_env.info[sf_index][cs].mid  << 16) |
           (sf_env.info[sf_index][cs].mtid <<  8) |
           (sf_env.info[sf_index][cs].cid  <<  0);
}

/** @} */

