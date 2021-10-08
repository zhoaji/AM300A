/**
 * @file lcd_serial.c
 * @brief 
 * @date Mon, Jul 22, 2019 11:32:03 AM
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details LCD serial interface driver, Used for HS6621CB/HS6621P
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

#define SW_CFG_ENABLE                    1
#define BUF_WIDTH_BYTES_RGB565           4
#define BUF_WIDTH_BYTES_RGB666_RGB888    3

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    struct {
        lcd_line_mode_t line_mode;
        lcd_rgb_mode_t rgb_mode;
        lcd_wire_mode_t wire_mode;
        lcd_width_swap_mode_t swap_mode;
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
/**
 * @brief lcd config
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  config
 **/
static void sfb_lcd_config(HS_SF_Type *sf, uint32_t cs, const lcd_config_t *config)
{
    co_assert(cs < SFB_CS_NUM);

    // Write REG
    REGW(&sf->CONFIGURATION[cs].CTRL, MASK_3REG(
        SF_CTRL_RGB_MODE,       config->rgb_mode,
        SF_CTRL_LCD_SPI_CTRL,   config->wire_mode,
        SF_CTRL_WIDTH,          config->swap_mode));

    // Save
    sfb_regs_set(sf, cs, sf->CONFIGURATION[cs].CTRL);
}

/**
 * @brief sfb_lcd_config_sw
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  config
 **/
static void sfb_lcd_config_sw(HS_SF_Type *sf, const lcd_config_t *config, uint32_t byteCnt)
{
    // Write sw cfg0 REG
    REGW(&sf->SW_SPI_CFG0_REG,MASK_4REG(SW_CFG0_CMD_P0_BIT_CNT,        8,
                                        SW_CFG0_CMD_P0_BUS_WIDTH,      0,
                                        SW_CFG0_CMD_P1_BUS_WIDTH,      0,
                                        SW_CFG0_CMD_P1_BIT_CNT,        24));

    // Write sw cfg1 REG
    REGW(&sf->SW_SPI_CFG1_REG,MASK_4REG(SW_CFG1_SDATA_BYTE_CNT,        byteCnt,
                                        SW_CFG1_SDATA_BUS_WIDTH,       config->line_mode,
                                        SW_CFG1_BUF_WIDTH_BYTES,       config->rgb_mode==SFB_LCD_RGB565 ? BUF_WIDTH_BYTES_RGB565 : BUF_WIDTH_BYTES_RGB666_RGB888,
                                        SW_CFG1_SW_CFG_EN,             SW_CFG_ENABLE));
}

/**
 * @brief sfb_lcd_sw_reset
 *
 * @param[in] sf  sf object
 **/
static void sfb_lcd_sw_enable(HS_SF_Type *sf, uint8_t isEnable)
{
    /* Enable or Disable software cfg */
    REGW(&sf->SW_SPI_CFG1_REG,MASK_1REG(SW_CFG1_SW_CFG_EN, isEnable));

//    register_set0(&sf->SW_SPI_CFG0_REG,0xFFFFFFFF,0);
//    register_set0(&sf->SW_SPI_CFG1_REG,0xFFFFFFFF,0);
}


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
void lcd_config(HS_SF_Type *lcd, uint32_t cs, const sfb_config_t *sfconfig, const lcd_config_t *lcdconfig)
{
    sfb_config(lcd, cs, sfconfig);
    sfb_lcd_config(lcd, cs, lcdconfig);

    /* save the lcd relative cfg parameters */
    lcds_env.config[cs].line_mode = lcdconfig->line_mode;
    lcds_env.config[cs].rgb_mode = lcdconfig->rgb_mode;
    lcds_env.config[cs].wire_mode = lcdconfig->wire_mode;
    lcds_env.config[cs].swap_mode = lcdconfig->swap_mode;
}

/****************************************************************************************************
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
    lcd_config_t config;
    uint32_t sfb_reg;
    sfb_callback_t sfb_cb;

    // save
    sfb_reg = sfb_regs_get(lcd, cs);
    sfb_cb = sfb_dma_done_event_get(lcd);
    if(sfb_cb) sfb_dma_done_event_register(lcd, NULL);

    config.rgb_mode = SFB_LCD_RGBNON;
    config.wire_mode = SFB_LCD_3WIRE_MODE1;
    config.swap_mode = SFB_LCD_SWAP_LEN_8BITS;

    sfb_lcd_config(lcd, cs, &config);

    // write reg
    param.cm.d0 = reg;
    param.cmd_bits = 8; 
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
    lcd_config_t config;
    uint32_t sfb_reg;
    sfb_callback_t sfb_cb;

    // save
    sfb_reg = sfb_regs_get(lcd, cs);
    sfb_cb = sfb_dma_done_event_get(lcd);
    if(sfb_cb) sfb_dma_done_event_register(lcd, NULL);

    config.rgb_mode = SFB_LCD_RGBNON;
    config.wire_mode = SFB_LCD_3WIRE_MODE1;
    config.swap_mode = SFB_LCD_SWAP_LEN_8BITS;

    sfb_lcd_config(lcd, cs, &config);

    // write reg
    param.cm.d0 = reg;
    param.cmd_bits = dummy_bits + 8;
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

    param.cmd[0] = reg << 24; 
    param.cmd_bits = 8; 
    param.data = data;
    param.data_bytes = data_bytes;

    if( SFB_LCD_2LANE == lcds_env.config[cs].line_mode )
    {
        sfb_lcd2lane_enable(lcd, cs, 1);
    }
    else if( SFB_LCD_1LANE == lcds_env.config[cs].line_mode )
    {
        sfb_lcd2lane_enable(lcd, cs, 0);
    }
    
    sfb_write_dma(lcd, cs, &param);

    sfb_lcd2lane_enable(lcd, cs, 0);
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

    param.cmd[0] = reg << 24; 
    param.cmd_bits = 8; 
    param.data = data;
    param.data_bytes = data_bytes;

    if( SFB_LCD_2LANE == lcds_env.config[cs].line_mode )
    {
        sfb_lcd2lane_enable(lcd, cs, 1);
    }
    else if( SFB_LCD_1LANE == lcds_env.config[cs].line_mode )
    {
        sfb_lcd2lane_enable(lcd, cs, 0);
    }
    
    sfb_write_dma_ex(lcd, cs, keep_cs, &param);

    sfb_lcd2lane_enable(lcd, cs, 0);
}

/**
 * @brief lcd_write_reg_4lines()
 *
 * @param[in] lcd  
 * @param[in] cs  
 * @param[in] reg  
 * @param[in] data  
 * @param[in] data_bytes  
 *
 * @return 
 **/
void lcd_write_reg_4lines(HS_SF_Type *lcd, uint32_t cs, uint8_t reg0, uint32_t reg1,uint8_t cmdBits)
{
    sfb_rw_params_t param;
    lcd_config_t config;
    uint32_t sfb_reg;
    sfb_callback_t sfb_cb;

    // save
    sfb_reg = sfb_regs_get(lcd, cs);
    sfb_cb = sfb_dma_done_event_get(lcd);
    if(sfb_cb) sfb_dma_done_event_register(lcd, NULL);

    // use 1lane config
    config.rgb_mode = SFB_LCD_RGBNON;
    config.wire_mode  = SFB_LCD_FLASH_LIKE;
    config.swap_mode  = SFB_LCD_SWAP_LEN_8BITS;
    sfb_lcd_config(lcd, cs, &config);

    // write reg
    param.cmd[0] = (0x02<<24)|(reg0<<8);
    param.cmd[1] = reg1;
    param.cmd_bits = cmdBits;
    param.data = 0;
    param.data_bytes = 0;
    sfb_write_dma(lcd, cs, &param);

    // restore old config
    sfb_regs_set(lcd, cs, sfb_reg);
    if(sfb_cb) sfb_dma_done_event_register(lcd, sfb_cb);
}


/**
 * @brief lcd_write_reg_and_data_4lines()
 *
 * @param[in] lcd  
 * @param[in] cs  
 * @param[in] reg  
 * @param[in] data  
 * @param[in] data_bytes  
 *
 * @return 
 **/
void lcd_write_reg_and_data_4lines(HS_SF_Type *lcd, uint32_t cs, uint8_t reg, const void *data, uint32_t data_bytes)
{
    sfb_rw_params_t param;
    lcd_config_t config;
    uint32_t sfb_reg;
    sfb_callback_t sfb_cb;

    // save
    sfb_reg = sfb_regs_get(lcd, cs);
    sfb_cb = sfb_dma_done_event_get(lcd);
    if(sfb_cb) sfb_dma_done_event_register(lcd, NULL);

    // use 1lane config
    config.rgb_mode = SFB_LCD_RGBNON;
    config.wire_mode  = SFB_LCD_FLASH_LIKE;
    config.swap_mode  = SFB_LCD_SWAP_LEN_8BITS;
    sfb_lcd_config(lcd, cs, &config);

    // write reg
    param.cmd[0] = (0x02<<24)|(reg<<8);
    param.cmd_bits = 8;
    param.data = data;
    param.data_bytes = data_bytes;
    sfb_write_dma(lcd, cs, &param);

    // restore old config
    sfb_regs_set(lcd, cs, sfb_reg);
    if(sfb_cb) sfb_dma_done_event_register(lcd, sfb_cb);
}

/**
 * @brief lcd_write_mem_ex_4lines()
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
void lcd_write_mem_ex_4lines(HS_SF_Type *lcd, uint32_t cs, sfb_keep_cs_t keep_cs, uint8_t reg, const void *data, uint32_t data_bytes)
{
    sfb_rw_params_t param;
    lcd_config_t config;

    config.rgb_mode = lcds_env.config[cs].rgb_mode;
    config.wire_mode = SFB_LCD_FLASH_LIKE;

    if( SFB_LCD_RGB565 == config.rgb_mode )
    {
        config.swap_mode = SFB_LCD_SWAP_LEN_16BITS;
    }
    else
    {
        config.swap_mode = SFB_LCD_SWAP_NONE;
    }
    // Write configuration REG
    sfb_lcd_config(lcd, cs, &config);

    config.line_mode = SFB_LCD_4LANE;

    /* Set the regs of SW_SPI_CFG0 reg and SW_SPI_CFG1 reg */
    sfb_lcd_config_sw(lcd, &config, data_bytes);

    param.cmd[0] = 0x32000000;
    param.cmd[1] = reg<<16;
    param.cmd_bits = 8;
    param.data = data;
    param.data_bytes = data_bytes;

    sfb_write_dma_ex(lcd, cs, keep_cs, &param);

    //disable sw reg
    sfb_lcd_sw_enable(lcd,0);
}


/** @} */

