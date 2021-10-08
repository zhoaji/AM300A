/**
 * @file sf_sys.h
 * @brief System SFlash
 * @date Tue, Jul 16, 2019  2:29:46 PM
 * @author liqiang
 *
 * @defgroup SF_SYS System SFlash
 * @ingroup PERIPHERAL
 * @brief System Sflash module
 * @details System Sflash module
 *
 * @{
 */

#ifndef __SF_SYS_H__
#define __SF_SYS_H__

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
/// Default system SFlash clock frequency in Hz
#ifdef CONFIG_XIP_FLASH_ALL
#define SFS_CLK_FREQ_HZ_DEF     16000000
#else
#define SFS_CLK_FREQ_HZ_DEF     1000000
#endif


/// @cond

#define SFS_CUR_OBJ             sfs_regobj_tbl[sfs_env.locate], 0

// only compatible for old bootloader
#define SFS_IS_IN_SFLASH(addr)  ((addr) < 0x08000000)

// base address
#ifdef CONFIG_HS6621
#define SFS_SPI_BASE           0x00000000
#define SFS_CACHABLE_BASE      0x12000000
#define SFS_NONCACHABLE_BASE   0x50000000
#else
#define SFS_SPI_BASE           0x00000000
#define SFS_CACHABLE_BASE      0x00400000
#define SFS_NONCACHABLE_BASE   0x50000000
#define SFS_CACHABLE_BASE_1    0x00800000
#define SFS_NONCACHABLE_BASE_1 0x52000000
#define SFS_CACHABLE_BASE_2    0x00C00000
#define SFS_NONCACHABLE_BASE_2 0x54000000
#endif
#define SFS_BASE_MASK          0xFFC00000
#define SFS_OFFSET_MASK        0x003FFFFF

/// @endcond

/*********************************************************************
 * TYPEDEFS
 */
/// system sflash location
typedef enum
{
    SFS_IFLASH,
    SFS_XFLASH,
    SFS_LOCATE_NUM,
}sfs_locate_t;

/// @cond
typedef struct
{
    sfs_locate_t locate;
    bool auto_disen;
    bool xip;
}sfs_env_t;
/// @endcond

/*********************************************************************
 * EXTERN VARIABLES
 */
/// @cond
extern HS_SF_Type * const sfs_regobj_tbl[SFS_LOCATE_NUM];
extern sfs_env_t sfs_env;
/// @endcond

/*********************************************************************
 * EXTERN FUNCTIONS
 */

/// read sr
#define sfs_read_sr()                                       sf_read_sr(SFS_CUR_OBJ)
/// read sr2
#define sfs_read_sr2()                                      sf_read_sr2(SFS_CUR_OBJ)
/// read sr with 16bits
#define sfs_read_sr_16bits()                                sf_read_sr_16bits(SFS_CUR_OBJ)
/// wait sr no busy
#define sfs_wait_sr_no_busy()                               sf_wait_sr_no_busy(SFS_CUR_OBJ)
/// write enable
#define sfs_write_enable()                                  sf_write_enable(SFS_CUR_OBJ)
/// write sr
#define sfs_write_sr(sr)                                    sf_write_sr(SFS_CUR_OBJ, sr)
/// write sr with 16bits
#define sfs_write_sr_16bits(sr)                             sf_write_sr_16bits(SFS_CUR_OBJ, sr)
/// write sr with mask
#define sfs_write_sr_mask(mask, value)                      sf_write_sr_mask(SFS_CUR_OBJ, mask, value)
/// write 16bits sr with mask
#define sfs_write_sr_mask_16bits(mask, value)               sf_write_sr_mask_16bits(SFS_CUR_OBJ, mask, value)
/// quad enable
#define sfs_quad_enable(enable)                             sf_quad_enable(SFS_CUR_OBJ, enable)
/// otp set
#define sfs_otp_set(lb_mask)                                sf_otp_set(SFS_CUR_OBJ, lb_mask)
/// otp get
#define sfs_otp_get()                                       sf_otp_get(SFS_CUR_OBJ)
/// low power enter
#define sfs_lowpower_enter()                                sf_lowpower_enter(SFS_CUR_OBJ)
/// low power leave
#define sfs_lowpower_leave()                                sf_lowpower_leave(SFS_CUR_OBJ)
/// unlock all
#define sfs_unlock_all()                                    sf_unlock_all(SFS_CUR_OBJ)
/// read id
#define sfs_read_id()                                       sf_read_id(SFS_CUR_OBJ)
/// read uid
#define sfs_read_uid_ex(data, length)                       sf_read_uid_ex(SFS_CUR_OBJ, data, length)
/// read uid
#define sfs_read_uid()                                      sf_read_uid(SFS_CUR_OBJ)
/// erase chip
#define sfs_erase_chip()                                    sf_erase_chip(SFS_CUR_OBJ)
//#define sfs_erase_sector(addr)                              sf_erase_sector(SFS_CUR_OBJ, addr)
/// erase block
#define sfs_erase_block(addr)                               sf_erase_block(SFS_CUR_OBJ, addr)
/// erase sec
#define sfs_erase_sec(addr)                                 sf_erase_sec(SFS_CUR_OBJ, addr)
/// erase
#define sfs_erase(addr, length)                             sf_erase(SFS_CUR_OBJ, addr, length)
/// write sec
#define sfs_write_sec(addr, data, length)                   sf_write_sec(SFS_CUR_OBJ, addr, data, length)
/// write
#define sfs_write(addr, data, length)                       sf_write(SFS_CUR_OBJ, addr, data, length)
/// read sec
#define sfs_read_sec(addr, data, length)                    sf_read_sec(SFS_CUR_OBJ, addr, data, length)
/// read
#define sfs_read(addr, data, length)                        sf_read(SFS_CUR_OBJ, addr, data, length)
/// status
#define sfs_status()                                        sf_status(SFS_CUR_OBJ)
/// get capacity
#define sfs_capacity()                                      sf_capacity(SFS_CUR_OBJ)
/// get sf id
#define sfs_id()                                            sf_id(SFS_CUR_OBJ)
/// enable
#define sfs_enable()                                        sf_enable(SFS_CUR_OBJ)
/// disable
#define sfs_disable()                                       sf_disable(SFS_CUR_OBJ)

/// erase sector
#define sfs_erase_sector(sector_n)                          sf_erase(SFS_CUR_OBJ, ((sector_n)<<SF_SECTOR_SHIFT), SF_SECTOR_SIZE)
/// write sector
#define sfs_write_sector(sector_n, offset, data, length)    sf_write(SFS_CUR_OBJ, ((sector_n)<<SF_SECTOR_SHIFT) + (offset), data, length)
/// read sector
#define sfs_read_sector(sector_n, offset, data, length)     sf_read(SFS_CUR_OBJ, ((sector_n)<<SF_SECTOR_SHIFT) + (offset), data, length)
/// is present ?
#define sfs_is_present()                                    (sfs_status() == SF_STATUS_PRESENT)

/**
 * @brief config
 *
 * @param[in] freq_hz  frequency in Hz
 * @param[in] width  width
 * @param[in] delay  delay
 **/
void sfs_config(uint32_t freq_hz, sf_width_t width, uint8_t delay);

/**
 * @brief sflash probe
 *
 * @param[in] locate  locate
 * @param[in] freq_hz  frequency in Hz
 *
 * @return errno
 **/
int sfs_probe(sfs_locate_t locate, uint32_t freq_hz);

/**
 * @brief select inside or outside flash
 *
 * @param[in] locate  locate
 *
 * @return errno
 **/
int sfs_select(sfs_locate_t locate);

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
                void *data, uint16_t data_bytes);

/**
 * @brief  sfs is auto disen
 *
 * @return is
 **/
bool sfs_is_auto_disen(void);

/**
 * @brief  sfs auto disen set
 *
 * @param[in] auto_disen  auto disen
 **/
void sfs_auto_disen_set(bool auto_disen);

/**
 * @brief  sfs is xip
 *
 * @return is
 **/
bool sfs_is_xip(void);

/**
 * @brief  sfs xip set
 *
 * @param[in] xip  xip
 **/
void sfs_xip_set(bool xip);

/**
 * @brief cache enable
 *
 * @param[in] enable  enable
 **/
void sfs_cache_enable(bool enable);

/**
 * @brief  sfs cache invalidate all
 **/
void sfs_cache_invalidate_all(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

