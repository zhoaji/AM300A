/**
 * @file cfg.h
 * @brief Configuration system
 * @date Mon 13 Apr 2015 10:46:17 AM CST
 * @author luwei
 *
 * @defgroup CONFIG Configuration
 * @ingroup HS662X
 * @brief Configuration system
 * @details
 *
 * The configuration system is used for storing information
 * which should be saved when chip loses power.
 *
 * <pre>
 * Features:
 *   • Two Section: one is SYSTEM section, the othor is USER section
 *   • Coexistence with "Nordic Persistent Storage Manager"
 *   • Auto Power lost protection
 *   • The storage entity can be either Flash or SRAM
 *
 * </pre>
 *
 * @{
 *
 * @example example_configuration.c
 * This is an example of how to use the configuration on storage.
 *
 */

#ifndef __CFG_H__
#define __CFG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>
#include "rwip_config.h"

#if NVDS_EN_NEW
#include "cfg_nvds.h"
#endif

/*********************************************************************
 * MACROS
 */

/// Configuration section define for system section
#define CFG_SECTION_SYS             1
/// Configuration section define for user section
#define CFG_SECTION_USR             2

/// Max tag length
#if NVDS_EN_NEW
#define CFG_TAG_LENGTH_MAX          NVDS_MAX_LENGTH_OF_TAGS
#else
#define CFG_TAG_LENGTH_MAX          511
#endif

/// @cond

// Configration tag define for section: CFG_SECTION_SYS
#define CFG_TAG_BDADDR              1
#define CFG_TAG_BDNAME              2
#define CFG_TAG_SCA                 3
#define CFG_TAG_XTAL32M_CTUNE       4
#define CFG_TAG_XTAL32K_CTUNE       5
#define CFG_TAG_FREQ_OFFSET         6
#define CFG_TAG_DCDC_ENABLE         7
#define CFG_TAG_PRE_WAKEUP_TIME     8

#define CFG_TAG_LOCAL_ER            100
#define CFG_TAG_LOCAL_IR            101
#define CFG_TAG_OTAS_STATIC_ADDRESS 102
#define CFG_TAG_APP_STATIC_ADDRESS  103
#define CFG_TAG_CALIB_DATA_LOW_T    110
#define CFG_TAG_CALIB_DATA_NORMAL_T 112
#define CFG_TAG_CALIB_DATA_HIGH_T   114

// Configration tag length define for section: CFG_SECTION_SYS
#define CFG_TAG_BDADDR_LEN          6
#define CFG_TAG_BDNAME_LEN          32
#define CFG_TAG_SCA_LEN             1

/// @endcond

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

#if (NVDS_SUPPORT & NVDS_EN_NEW)

/// @cond
/**
 * @brief Configuration initialize
 *
 * @param[in] base_addr  The cfg's base address in ram, or 0 to probe flash
 * @param[in] length     The cfg's total length in ram; unused for flash
 *
 * @return errno
 **/
__STATIC_INLINE int cfg_init(uint32_t base_addr, uint32_t length)
{
    uint8_t res = om_nvds_init(base_addr);
    return res==NVDS_OK ? 0 : -1;
}
/// @endcond

/**
 * @brief Configuration reset
 *
 * @param[in] section  CFG_SECTION_SYS or CFG_SECTION_USR
 *
 * @return None
 **/
__STATIC_INLINE void cfg_reset(uint8_t section)
{
    om_nvds_reset();
}

/**
 * @brief put data to a section
 *
 * @param[in] section  section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 * @param[in] tag  tag index
 * @param[in] pdata  data
 * @param[in] len data length, excluding tag node size, range in (0, 511].
 *
 * @return errno or physical offset in section
 **/
__STATIC_INLINE int cfg_put(uint8_t section, uint8_t tag, const void *pdata, uint16_t len)
{
    uint8_t res = om_nvds_put((section==CFG_SECTION_USR) ? (tag+NVDS_TAG_APP_SPECIFIC_FIRST) : tag, len,  pdata);
    return res==NVDS_OK ? 0 : -1;
}

/**
 * @brief get data from a section
 *
 * @param[in] section  section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 * @param[in] tag  tag index
 * @param[in] pdata  data
 * @param[in] plen    pdata buffer length, if it is null, ignore it.
 * @param[out] plen'  real readed length, if plen' > plen,
 *
 * @return errno or physical offset in section
 **/
__STATIC_INLINE int cfg_get(uint8_t section, uint8_t tag, void *pdata, uint16_t *plen)
{
    uint8_t res = om_nvds_get((section==CFG_SECTION_USR) ? (tag+NVDS_TAG_APP_SPECIFIC_FIRST) : tag, plen, pdata);
    return res==NVDS_OK ? 0 : -1;
}

/**
 * @brief delete data from a section
 *
 * @param[in] section  section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 * @param[in] tag  tag index
 *
 * @return errno or 0
 **/
__STATIC_INLINE int cfg_del(uint8_t section, uint8_t tag)
{
    uint8_t res = om_nvds_del((section==CFG_SECTION_USR) ? (tag+NVDS_TAG_APP_SPECIFIC_FIRST) : tag);
    return res==NVDS_OK ? 0 : -1;
}

/**
 * @brief cfg_dump()
 *
 * @param[in] printf_dump_func
 *
 * @return
 **/
__STATIC_INLINE void cfg_dump(void *printf_dump_func)
{
    om_nvds_dump(printf_dump_func);
}

#else

/// @cond
/**
 * @brief Configuration initialize
 *
 * @param[in] base_addr  The cfg's base address in ram, or 0 to probe flash
 * @param[in] length     The cfg's total length in ram; unused for flash
 *
 * @return errno
 **/
int cfg_init(uint32_t base_addr, uint32_t length);
/// @endcond

/**
 * @brief Configuration reset
 *
 * @param[in] section  CFG_SECTION_SYS or CFG_SECTION_USR
 *
 * @return None
 **/
void cfg_reset(uint8_t section);

/**
 * @brief set a section readonly
 *
 * @param[in] section section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 *
 * @return None
 **/
void cfg_set_readonly(uint8_t section);

/**
 * @brief whether the section is valid
 *
 * @param[in] section  section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 *
 * @retval true valid
 * @retval false invalid
 **/
bool cfg_is_valid(uint8_t section);

/**
 * @brief put data to a section
 *
 * @param[in] section  section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 * @param[in] tag  tag index
 * @param[in] pdata  data
 * @param[in] len data length, excluding tag node size, range in (0, 511].
 *
 * @return errno or physical offset in section
 **/
int cfg_put(uint8_t section, uint8_t tag, const void *pdata, uint16_t len);

/**
 * @brief get data from a section
 *
 * @param[in] section  section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 * @param[in] tag  tag index
 * @param[in] pdata  data
 * @param[in] plen    pdata buffer length, if it is null, ignore it.
 * @param[out] plen'  real readed length, if plen' > plen,
 *
 * @return errno or physical offset in section
 **/
int cfg_get(uint8_t section, uint8_t tag, void *pdata, uint16_t *plen);

/**
 * @brief delete data from a section
 *
 * @param[in] section  section: @ref CFG_SECTION_SYS @ref CFG_SECTION_USR
 * @param[in] tag  tag index
 *
 * @return errno or 0
 **/
int cfg_del(uint8_t section, uint8_t tag);

/**
 * @brief cfg_dump()
 *
 * @param[in] printf_dump_func
 *
 * @return
 **/
void cfg_dump(void *printf_dump_func);

#endif /* (NVDS_SUPPORT & NVDS_EN_NEW) */

#ifdef __cplusplus
}
#endif

#endif

/** @} */


