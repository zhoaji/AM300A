/**
 * @file sf.h
 * @brief SFlash driver
 * @date Fri, Jul 12, 2019  3:16:15 PM
 * @author liqiang
 *
 * @defgroup SF SFlash
 * @ingroup PERIPHERAL
 * @brief SFlash driver
 * @details SFlash driver
 *
 * @{
 *
 * @example example_sf.c
 * This is an example of how to use the sf
 *
 */

#ifndef __SF_RAW_H__
#define __SF_RAW_H__

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
/// SF page shift
#define SF_PAGE_SHIFT       8
/// SF sector shift
#define SF_SECTOR_SHIFT     12
/// SF page size
#define SF_PAGE_SIZE        256
/// SF sector size
#define SF_SECTOR_SIZE      4096

/*********************************************************************
 * TYPEDEFS
 */
/// SF wire width
typedef enum
{
    SF_WIDTH_1LINE = 1,
    SF_WIDTH_2LINE = 2,
    SF_WIDTH_4LINE = 4,
}sf_width_t;

/// SF status
typedef enum
{
    /// Not detected
    SF_STATUS_NONE,
    /// Detected and sflash absent
    SF_STATUS_ABSENT,
    /// Detected and sflash present
    SF_STATUS_PRESENT,
}sf_status_t;

/// SF config
typedef struct
{
    /// frequency in Hz
    uint32_t freq_hz;
    /// width @ref sf_width_t
    sf_width_t width;
    /// Delayed Sampling, default is 0
    uint8_t delay;
}sf_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief read sr reg
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sr value
 **/
uint8_t sf_read_sr(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief read sr2
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sr2 value
 **/
uint8_t sf_read_sr2(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief read sr 16bits
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sr | (sr2<<8)
 **/
uint16_t sf_read_sr_16bits(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief wait sr no busy
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_wait_sr_no_busy(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief write enable
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_write_enable(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief write sr
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] sr  sr
 **/
void sf_write_sr(HS_SF_Type *sf, uint32_t cs, uint8_t sr);

/**
 * @brief write sr 16bits
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] sr  sr
 **/
void sf_write_sr_16bits(HS_SF_Type *sf, uint32_t cs, uint16_t sr);

/**
 * @brief write sr with mask
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] mask  sr mask
 * @param[in] value  sr value
 **/
void sf_write_sr_mask(HS_SF_Type *sf, uint32_t cs, uint8_t mask, uint8_t value);

/**
 * @brief write 16bits sr with mask
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] mask  sr mask
 * @param[in] value  sr value
 **/
void sf_write_sr_mask_16bits(HS_SF_Type *sf, uint32_t cs, uint16_t mask, uint16_t value);

/**
 * @brief quad enable
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] enable  true or false
 **/
void sf_quad_enable(HS_SF_Type *sf, uint32_t cs, bool enable);

/**
 * @brief otp set
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] lb_mask  lb mask
 **/
void sf_otp_set(HS_SF_Type *sf, uint32_t cs, uint8_t lb_mask);

/**
 * @brief otp get
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return lb_mask
 **/
uint8_t sf_otp_get(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief  sf lowpower enter
 *
 * @param[in] sf  sf
 * @param[in] cs  cs
 **/
void sf_lowpower_enter(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief  sf lowpower leave
 *
 * @param[in] sf  sf
 * @param[in] cs  cs
 **/
void sf_lowpower_leave(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief unlock all
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_unlock_all(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief read id
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sflash id (24bits)
 **/
uint32_t sf_read_id(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief read uid
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] data  read uid buffer
 * @param[in] length  length
 **/
void sf_read_uid_ex(HS_SF_Type *sf, uint32_t cs, void *data, uint32_t length);

/**
 * @brief read uid
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sflash UID
 **/
uint32_t sf_read_uid(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief erase chip
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_erase_chip(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief erase sector
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 **/
void sf_erase_sector(HS_SF_Type *sf, uint32_t cs, uint32_t addr);

/**
 * @brief erase block
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 **/
void sf_erase_block(HS_SF_Type *sf, uint32_t cs, uint32_t addr);

/**
 * @brief erase sec
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 **/
void sf_erase_sec(HS_SF_Type *sf, uint32_t cs, uint32_t addr);

/**
 * @brief sf erase
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] length  length
 **/
void sf_erase(HS_SF_Type *sf, uint32_t cs, uint32_t addr, uint32_t length);

/**
 * @brief write page without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_page_nodma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length);

/**
 * @brief write page with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_page_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length);

/**
 * @brief write page
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_page(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length);

/**
 * @brief write sec
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write_sec(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length);

/**
 * @brief sf write
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[in] data  write data
 * @param[in] length  length
 **/
void sf_write(HS_SF_Type *sf, uint32_t cs, uint32_t addr, const void *data, uint32_t length);

/**
 * @brief read normal without dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_normal_nodma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief read normal with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_normal_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief read fast with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief read fast 2line with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_dual_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief read fast 4line with dma (Not auto enable quad mode, please call sf_quad_enable outside)
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_quad_naked_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief read fast 4line with dma
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_fast_quad_dma(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief read sec
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read_sec(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief sf read
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] addr  sflash address
 * @param[out] data  read data buffer
 * @param[in] length  length
 **/
void sf_read(HS_SF_Type *sf, uint32_t cs, uint32_t addr, void *data, uint32_t length);

/**
 * @brief sf config
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 * @param[in] config  sf config
 **/
void sf_config(HS_SF_Type *sf, uint32_t cs, const sf_config_t *config);

/**
 * @brief sf enable
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_enable(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief  sf disable
 *
 * @param[in] sf  sf
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 **/
void sf_disable(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief sf iflash auto close
 *
 * @param[in] delay_ms  
 **/
void sf_iflash_auto_close(uint32_t delay_ms);

/**
 * @brief  sf iflash open extra delay set
 *
 * @param[in] delay_10us  delay 10us
 **/
void sf_iflash_extra_open_delay_set(uint32_t delay_10us);

/**
 * @brief sf detect
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return is present ?
 **/
bool sf_detect(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief sf status
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return status
 **/
sf_status_t sf_status(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief sf capacity
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return capacity
 **/
uint32_t sf_capacity(HS_SF_Type *sf, uint32_t cs);

/**
 * @brief sf get id
 *
 * @param[in] sf  sf object
 * @param[in] cs  cs (0 ~ SFB_CS_NUM-1)
 *
 * @return sflash id (saved by @ref sf_detect)
 **/
uint32_t sf_id(HS_SF_Type *sf, uint32_t cs);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

