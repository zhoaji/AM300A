/**
 * @file lcd_serial.h
 * @brief LCD serial interface driver
 * @date Mon, Jul 22, 2019 11:32:08 AM
 * @author liqiang
 *
 * @defgroup LCD_SERIAL LCD
 * @ingroup PERIPHERAL
 * @brief LCD serial interface driver
 * @details LCD serial interface driver
 *
 * @{
 *
 * @example example_lcd_serial.c
 * This is an example of how to use the lcd_serial
 *
 */

#ifndef __LCD_SERIAL_H__
#define __LCD_SERIAL_H__

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

/// Set dma done event callback, reference @ref sfb_dma_done_event_register
#define lcd_dma_done_event_register(lcd, cb)    sfb_dma_done_event_register(lcd, cb);
/// lcd open, reference @ref sfb_open
#define lcd_open(lcd)                           sfb_open(lcd);
/// lcd close, reference @ref sfb_close
#define lcd_close(lcd)                          sfb_close(lcd);
/// lcd enable, reference @ref sfb_enable
#define lcd_enable(lcd, cs)                     sfb_enable(lcd, cs);

#ifdef CONFIG_SF_XJF_DESIGN // This is just to be compatible with the old version LCD driver
#define SFB_LCD_1LANE                           LCD_1LANE
#define SFB_LCD_2LANE                           LCD_2LANE
#define SFB_LCD_4LANE                           LCD_4LANE
#define SFB_LCD_RGBNON                          LCD_RGBNON
#define SFB_LCD_RGB565                          LCD_RGB565
#define SFB_LCD_RGB666                          LCD_RGB666
#define SFB_LCD_RGB888                          LCD_RGB888
#define SFB_LCD_FLASH_LIKE                      LCD_FLASH_LIKE
#define SFB_LCD_3WIRE_MODE1                     LCD_3WIRE_MODE1
#define SFB_LCD_3WIRE_MODE2                     LCD_3WIRE_MODE2
#define SFB_LCD_4WIRE_MODE1                     LCD_4WIRE_MODE1
#define SFB_LCD_4WIRE_MODE2                     LCD_4WIRE_MODE2
#define SFB_LCD_WRITE                           LCD_WRITE
#define SFB_LCD_READ                            LCD_READ
#define SFB_LCD_SWAP_LEN_8BITS                  LCD_SWAP_LEN_8BITS
#define SFB_LCD_SWAP_LEN_16BITS                 LCD_SWAP_LEN_16BITS
#define SFB_LCD_SWAP_NONE                       LCD_SWAP_NONE
#endif

/*********************************************************************
 * TYPEDEFS
 */
#ifdef CONFIG_SF_XJF_DESIGN
/// LCD RGb transmite mode
typedef enum
{
    /// 1 lane
    LCD_1LANE,
    /// 2 lane
    LCD_2LANE,
    /// 4 lane
    LCD_4LANE,
}lcd_line_mode_t;

/// LCD RGB mode
typedef enum
{
    LCD_RGBNON = 0,
    LCD_RGB565 = 1,
    LCD_RGB666 = 2,
    LCD_RGB888 = 3,
}lcd_rgb_mode_t;

/// LCD wire mode
typedef enum
{
    LCD_FLASH_LIKE = 0,
    LCD_3WIRE_MODE1 = 1,
    LCD_3WIRE_MODE2 = 2,
    LCD_4WIRE_MODE1 = 3,
    LCD_4WIRE_MODE2 = 4,
}lcd_wire_mode_t;

/// LCD read and write mode
typedef enum
{
    /// write
    LCD_WRITE = 0,
    /// read
    LCD_READ = 1,
}lcd_rw_mode_t;

/// LCD read and write swap width mode
typedef enum
{
    //the len of swap is 8 bits
    LCD_SWAP_LEN_8BITS = 0,
    //the len of swap is 16 bits
    LCD_SWAP_LEN_16BITS = 1,
    //do not swap
    LCD_SWAP_NONE = 2,
}lcd_width_swap_mode_t;

/// lcd config
typedef struct
{
    /// RGB trans mode
    lcd_line_mode_t line_mode;
    /// RGB mode
    lcd_rgb_mode_t rgb_mode;
    /// RGB write mode
    lcd_wire_mode_t wire_mode;
    /// the mode of len's swap
    lcd_width_swap_mode_t swap_mode;
    /// read and write mode
    lcd_rw_mode_t rw_mode;
}lcd_config_t;
#endif

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

#ifdef CONFIG_SF_XJF_DESIGN
/**
 * @brief LCD config
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] sfconfig  sf config
 * @param[in] lcdconfig  lcd config
 **/
void lcd_config(HS_SF_Type *lcd, uint32_t cs, const sfb_config_t *sfconfig, const lcd_config_t *lcdconfig);
#else
/**
 * @brief LCD config
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] sfconfig  sf config
 * @param[in] lcdconfig  lcd config
 **/
void lcd_config(HS_SF_Type *lcd, uint32_t cs, const sfb_config_t *sfconfig, const sfb_lcd_config_t *lcdconfig);
#endif

/**
 * @brief lcd write reg
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] reg  reg value
 * @param[in] data  write data (Must be 4 bytes align, DMA request)
 * @param[in] data_bytes  write data length
 **/
void lcd_write_reg(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, const void *data, uint32_t data_bytes);

/**
 * @brief lcd read reg
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] reg  reg value
 * @param[in] dummy_bits  dummy bits between command and data
 * @param[in] data  write data (Must be 4 bytes align, DMA request)
 * @param[in] data_bytes  write data length
 **/
void lcd_read_reg(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, uint8_t dummy_bits, const void *data, uint32_t data_bytes);

/**
 * @brief lcd write memery (SRAM)
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] reg  reg value
 * @param[in] data  write data (Must be 4 bytes align, DMA request)
 * @param[in] data_bytes  write data length
 **/
void lcd_write_mem(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, const void *data, uint32_t data_bytes);

/**
 * @brief lcd write memery (SRAM)
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] keep_cs  keep cs, @ref sfb_keep_cs_t
 * @param[in] reg  reg value
 * @param[in] data  write data (Must be 4 bytes align, DMA request)
 * @param[in] data_bytes  write data length
 **/
void lcd_write_mem_ex(HS_SF_Type *lcd, uint32_t cs, sfb_keep_cs_t keep_cs, uint8_t reg, const void *data, uint32_t data_bytes);

#ifdef CONFIG_SF_XJF_DESIGN
/**
 * @brief lcd_write_reg_4lines (SRAM)
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] reg0  reg value
 * @param[in] reg1 reg value
 * @param[in] cmdBits  command bit length
 *
 * @return 
 **/
void lcd_write_reg_4lines(HS_SF_Type *lcd, uint32_t cs, uint8_t reg0, uint32_t reg1,uint8_t cmdBits);

/**
 * @brief lcd_write_reg_and_data_4lines (SRAM)
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] reg  reg value
 * @param[in] data  write data (Must be 4 bytes align, DMA request)
 * @param[in] data_bytes  write data length
 *
 * @return 
 **/
void lcd_write_reg_and_data_4lines(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, const void *data, uint32_t data_bytes);

/**
 * @brief lcd_write_mem_ex_4lines (SRAM)
 *
 * @param[in] lcd  LCD(SF) object
 * @param[in] cs  cs select
 * @param[in] keep_cs   keep cs, @ref sfb_keep_cs_t
 * @param[in] reg  reg value
 * @param[in] data  write data (Must be 4 bytes align, DMA request)
 * @param[in] data_bytes   write data length
 *
 * @return 
 **/
void lcd_write_mem_ex_4lines(HS_SF_Type *lcd, uint32_t cs, sfb_keep_cs_t keep_cs, uint8_t reg, const void *data, uint32_t data_bytes);
#endif

#ifdef __cplusplus
}
#endif

#endif

/** @} */

