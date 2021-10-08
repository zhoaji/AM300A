/**
 * @file lcd_serial.c
 * @brief 
 * @date Mon, Jul 22, 2019 11:32:03 AM
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details LCD serial interface driver, Used for HS6621/HS6621C
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


/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    struct {
        sfb_lcd_wire_mode_t wire_mode;
        sfb_lcd_rgb_transmode_t rgb_transmode;
    }config[SFB_CS_NUM];
}lcd_serial_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static lcd_serial_env_t lcds_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief lcd_config()
 *
 * @param[in] lcd  
 * @param[in] cs  
 * @param[in] sfconfig  
 * @param[in] lcdconfig  
 *
 * @return 
 **/
void lcd_config(HS_SF_Type *lcd, uint32_t cs, const sfb_config_t *sfconfig, const sfb_lcd_config_t *lcdconfig)
{
    sfb_config(lcd, cs, sfconfig);
    sfb_lcd_config(lcd, cs, lcdconfig);

    lcds_env.config[cs].wire_mode = lcdconfig->wire_mode;
    lcds_env.config[cs].rgb_transmode = lcdconfig->rgb_transmode;
}

/**
 * @brief lcd_write_reg()
 *
 * @param[in] lcd  
 * @param[in] cs  
 * @param[in] reg  
 * @param[in] data  
 * @param[in] data_bytes  
 *
 * @return 
 **/
void lcd_write_reg(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, const void *data, uint32_t data_bytes)
{
    sfb_rw_params_t param;
    sfb_lcd_config_t config;
    uint32_t sfb_reg;
    sfb_callback_t sfb_cb;

    // save
    sfb_reg = sfb_regs_get(lcd, cs);
    sfb_cb = sfb_dma_done_event_get(lcd);
    if(sfb_cb) sfb_dma_done_event_register(lcd, NULL);

    // use 1lane config
    config.rw_mode = SFB_LCD_WRITE;
    config.rgb_mode = SFB_LCD_RGBNON;
    config.rgb_transmode = SFB_LCD_1LANE;
    config.wire_mode = lcds_env.config[cs].wire_mode;
    sfb_lcd_config(lcd, cs, &config);

    // write reg
    param.cm.d0 = reg;
    param.cmd_bits = (lcds_env.config[cs].wire_mode == SFB_LCD_9BITS_WITH_DCX) ? 9 : 8; // 1byte(DCX) + 8bytes(CMD)
    param.data = data;
    param.data_bytes = data_bytes;
    sfb_write_dma(lcd, cs, &param);

    // restore old config
    sfb_regs_set(lcd, cs, sfb_reg);
    if(sfb_cb) sfb_dma_done_event_register(lcd, sfb_cb);
}

/**
 * @brief lcd_read_reg()
 *
 * @param[in] lcd  
 * @param[in] cs  
 * @param[in] reg  
 * @param[in] dummy_bits  
 * @param[in] data  
 * @param[in] data_bytes  
 *
 * @return 
 **/
void lcd_read_reg(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, uint8_t dummy_bits, const void *data, uint32_t data_bytes)
{
    sfb_rw_params_t param;
    sfb_lcd_config_t config;
    uint32_t sfb_reg;
    sfb_callback_t sfb_cb;

    // save
    sfb_reg = sfb_regs_get(lcd, cs);
    sfb_cb = sfb_dma_done_event_get(lcd);
    if(sfb_cb) sfb_dma_done_event_register(lcd, NULL);

    // use 1lane config
    config.rw_mode = SFB_LCD_READ;
    config.rgb_mode = SFB_LCD_RGBNON;
    config.rgb_transmode = SFB_LCD_1LANE;
    config.wire_mode = lcds_env.config[cs].wire_mode;
    sfb_lcd_config(lcd, cs, &config);

    // write reg
    param.cm.d0 = reg;
    param.cmd_bits = dummy_bits + ((lcds_env.config[cs].wire_mode == SFB_LCD_9BITS_WITH_DCX) ? 9 : 8); // 1byte(DCX) + 8bytes(CMD)
    param.data = data;
    param.data_bytes = data_bytes;
    sfb_read_dma(lcd, cs, &param);

    // restore old config
    sfb_regs_set(lcd, cs, sfb_reg);
    if(sfb_cb) sfb_dma_done_event_register(lcd, sfb_cb);
}

/**
 * @brief lcd_write_mem()
 *
 * @param[in] lcd  
 * @param[in] cs  
 * @param[in] reg  
 * @param[in] data  
 * @param[in] data_bytes  
 *
 * @return 
 **/
void lcd_write_mem(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, const void *data, uint32_t data_bytes)
{
    sfb_rw_params_t param;

    param.cmd[0] = reg << (lcds_env.config[cs].rgb_transmode==SFB_LCD_1LANE ? 24 : 23); // digital bug: not consider DCX in 2LANE mode
    param.cmd_bits = (lcds_env.config[cs].wire_mode==SFB_LCD_9BITS_WITH_DCX) ? 9 : 8; // 1byte(DCX) + 8bytes(CMD)
    param.data = data;
    param.data_bytes = data_bytes;
    sfb_write_dma(lcd, cs, &param);
}

/**
 * @brief lcd_write_mem_ex()
 *
 * @param[in] lcd  
 * @param[in] cs  
 * @param[in] keep_cs  
 * @param[in] reg  
 * @param[in] data  
 * @param[in] data_bytes  
 *
 * @return 
 **/
void lcd_write_mem_ex(HS_SF_Type *lcd, uint32_t cs, sfb_keep_cs_t keep_cs, uint8_t reg, const void *data, uint32_t data_bytes)
{
    sfb_rw_params_t param;

    param.cmd[0] = reg << (lcds_env.config[cs].rgb_transmode==SFB_LCD_1LANE ? 24 : 23); // digital bug: not consider DCX in 2LANE mode
    param.cmd_bits = (lcds_env.config[cs].wire_mode==SFB_LCD_9BITS_WITH_DCX) ? 9 : 8;
    param.data = data;
    param.data_bytes = data_bytes;
    sfb_write_dma_ex(lcd, cs, keep_cs, &param);
}

/** @} */

