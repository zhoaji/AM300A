/**
 * @file sf_base.h
 * @brief SF and LCD base driver
 * @date Mon, Jul  8, 2019  4:16:06 PM
 * @author liqiang
 *
 * @defgroup SF_BASE SF and LCD base
 * @ingroup PERIPHERAL
 * @brief SF and LCD base driver
 * @details SF and LCD base driver
 *
 * @{
 */

#ifndef __SF_BASE_H__
#define __SF_BASE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */
#ifndef CONFIG_LAGACY_DESIGN
/// define sf xjf design
#define CONFIG_SF_XJF_DESIGN
#endif

#if defined(CONFIG_HS6621)
/// module number (HS_SF, HS_SF1)
#define SFB_MODULE_NUM         2
/// DMA data max length
#define SFB_DMA_DATA_LEN_MAX   0xFFF0 /* IP limitation & aligned to 16-byte */
#elif defined(CONFIG_HS6621P)
/// module number (HS_SF, HS_SF1, HS_SF2, HS_PSRAM)
#define SFB_MODULE_NUM         4
/// DMA data max length
#define SFB_DMA_DATA_LEN_MAX   0xFFFF0 /* IP limitation & aligned to 20-byte */
#else
/// module number (HS_SF, HS_SF1, HS_SF2)
#define SFB_MODULE_NUM         3
/// DMA data max length
#define SFB_DMA_DATA_LEN_MAX   0xFFFF0 /* IP limitation & aligned to 20-byte */
#endif

/// CS number
#define SFB_CS_NUM             2

/*********************************************************************
 * TYPEDEFS
 */

/**
 * @brief  event callback
 *
 * @param[in] sf  sf object
 **/
typedef void (*sfb_callback_t)(HS_SF_Type *sf);

#ifndef CONFIG_SF_XJF_DESIGN // move to lcd_serial.h
/// LCD RGB mode
typedef enum
{
    SFB_LCD_RGB565 = 0,
    SFB_LCD_RGB666 = 1,
    SFB_LCD_RGB888 = 2,
    SFB_LCD_RGBNON = 3,
}sfb_lcd_rgb_mode_t;

/// LCD RGb transmite mode
typedef enum
{
    /// 1 lane
    SFB_LCD_1LANE,
    /// 2lane: 1 pixle (RGB565-16bits, RGB666-18bits, RGB888-24bits), 1 time transmission
    SFB_LCD_2LANE_1PIXLE_1TRANS,
    /// 2lane: 2 pixle (RGB565-2*16bits, RGB666-2*18bits, RGB888-2*24bits), 3 times transmission
    SFB_LCD_2LANE_2PIXLE_3TRANS,
}sfb_lcd_rgb_transmode_t;

/// LCD wire mode
typedef enum
{
    /// 1byte(DCX) + 8bytes(CMD) in SDA
    SFB_LCD_9BITS_WITH_DCX,
    /// 8bytes(CMD) in SDA, DCX in DCX wire
    SFB_LCD_8BITS_DCX_ALONE,
}sfb_lcd_wire_mode_t;

/// LCD read and write mode
typedef enum
{
    /// write
    SFB_LCD_WRITE = 0,
    /// read
    SFB_LCD_READ = 1,
}sfb_lcd_rw_mode_t;
#endif

/// SPI transmite mode
typedef enum
{
    /// Mode 0: CPOL=0 CPHA=0
    SFB_SPI_MODE_0 = 0,
    /// Mode 1: CPOL=0 CPHA=1
    SFB_SPI_MODE_1 = 1,
    /// Mode 2: CPOL=1 CPHA=0
    SFB_SPI_MODE_2 = 2,
    /// Mode 3: CPOL=1 CPHA=1
    SFB_SPI_MODE_3 = 3,
}sfb_spi_transmode_t;

/// SPI CS acive pol
typedef enum
{
    /// Chip select is active low
    SFB_SPI_CS_LOW_ACTIVE = 0,
    /// Chip select is active high
    SFB_SPI_CS_HIGH_ACTIVE = 1,
}sfb_spi_cs_pol_t;

/// SPI keep CS type
typedef enum
{
    /// Not keep (normal mode)
    SFB_CS_NOKEEP,
    /// Keep CS begin
    SFB_CS_BEGIN,
    /// Keep CS continue
    SFB_CS_KEEP,
    /// Keep CS end
    SFB_CS_END,
}sfb_keep_cs_t;

/// cmd bytes
typedef struct
{
    /// byte3
    uint8_t d3;
    /// byte2
    uint8_t d2;
    /// byte1
    uint8_t d1;
    /// byte0
    uint8_t d0;
    /// byte7
    uint8_t d7;
    /// byte6
    uint8_t d6;
    /// byte5
    uint8_t d5;
    /// byte4
    uint8_t d4;
}sfb_cmd_bytes_t;

/// SFB config
typedef struct
{
    /// frequence in Hz. if less than 256, its division
    uint32_t freq_hz;
    /// Delayed Sampling
    uint8_t delay;
    /// spi transmode
    sfb_spi_transmode_t transmode;
    /// spi cs active pol
    sfb_spi_cs_pol_t cs_pol;
}sfb_config_t;

#ifndef CONFIG_SF_XJF_DESIGN // move to lcd_serial.h
/// lcd config
typedef struct
{
    /// read and write mode
    sfb_lcd_rw_mode_t rw_mode;
    /// RGB mode
    sfb_lcd_rgb_mode_t rgb_mode;
    /// RGB trans mode
    sfb_lcd_rgb_transmode_t rgb_transmode;
    /// RGB write mode
    sfb_lcd_wire_mode_t wire_mode;
}sfb_lcd_config_t;
#endif

/// SFB read and write param
typedef struct
{
    /// command
    union {
        uint32_t cmd[2];
        sfb_cmd_bytes_t cm;
    };
    /// command bits
    uint8_t cmd_bits;
    /// In DMA mode:
    ///   1-line: 4bytes align
    ///   2-line: 8bytes align
    ///   4-line: 16bytes align
    /// ROM address (0x08000000) can't use DMA write
    /// NOTE: data const type will be casted to non-cast for read operation
    const void *data;
    /// data length
    uint32_t data_bytes;
}sfb_rw_params_t;

/*********************************************************************
 * EXTERN VARIABLES
 */

/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief read with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_read_dma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param);

/**
 * @brief read with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] keep_cs  keep cs
 * @param[in] param  rw param
 **/
void sfb_read_dma_ex(HS_SF_Type *sf, uint32_t cs, sfb_keep_cs_t keep_cs, sfb_rw_params_t *param);

/**
 * @brief write with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_write_dma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param);

/**
 * @brief write with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] keep_cs  keep cs
 * @param[in] param  rw param
 **/
void sfb_write_dma_ex(HS_SF_Type *sf, uint32_t cs, sfb_keep_cs_t keep_cs, sfb_rw_params_t *param);

/**
 * @brief read without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_read_nodma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param);

/**
 * @brief write without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] param  rw param
 **/
void sfb_write_nodma(HS_SF_Type *sf, uint32_t cs, sfb_rw_params_t *param);

/**
 * @brief open
 *
 * @param[in] sf  sf object
 **/
void sfb_open(HS_SF_Type *sf);

/**
 * @brief close
 *
 * @param[in] sf  sf object
 **/
void sfb_close(HS_SF_Type *sf);

/**
 * @brief set dma done event
 *
 * @param[in] sf  sf object
 * @param[in] callback  event callback
 **/
void sfb_dma_done_event_register(HS_SF_Type *sf, sfb_callback_t callback);

/**
 * @brief  sfb dma done event get
 *
 * @param[in] sf  sf
 *
 * @return callback
 **/
sfb_callback_t sfb_dma_done_event_get(HS_SF_Type *sf);

/**
 * @brief config
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  config
 **/
void sfb_config(HS_SF_Type *sf, uint32_t cs, const sfb_config_t *config);

#ifdef CONFIG_SF_XJF_DESIGN
/**
 * @brief  sfb lcd2lane enable
 *
 * @param[in] sf  sf
 * @param[in] cs  cs
 * @param[in] enable  enable
 **/
void sfb_lcd2lane_enable(HS_SF_Type *sf, uint32_t cs, bool enable);
#else
/**
 * @brief lcd config
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  config
 **/
void sfb_lcd_config(HS_SF_Type *sf, uint32_t cs, const sfb_lcd_config_t *config);
#endif

/**
 * @brief restore
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sfb_restore(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief regs get
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return reg
 **/
uint32_t sfb_regs_get(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief regs get
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] regs  reg
 **/
void sfb_regs_set(HS_SF_Type *sf, uint32_t cs, uint32_t regs);

/**
 * @brief  sfb critical object set
 *
 * @param[in] sf  sf
 **/
void sfb_critical_object_set(HS_SF_Type *sf);

/**
 * @brief  sfb critical object get
 *
 * @return obj
 **/
HS_SF_Type *sfb_critical_object_get(void);

/**
 * @brief sfb_enable()
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sfb_enable(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief sfb_index()
 *
 * @param[in] sf  
 *
 * @return 
 **/
__STATIC_FORCEINLINE int sfb_index(HS_SF_Type *sf)
{
    return sf==HS_SF  ? 0 :
#if SFB_MODULE_NUM > 2
           sf==HS_SF2 ? 2 :
#endif
#if SFB_MODULE_NUM > 3
           sf==HS_SF3 ? 3 :
#endif
#if SFB_MODULE_NUM > 4
#error SFB_MODULE_NUM too larger!
#endif
                        1;


}

#ifdef __cplusplus
}
#endif

#endif

/** @} */

