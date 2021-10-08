/**
 * @file nvds.h
 * @brief Non Volatile Data Storage (NVDS) system
 * @date 10 Dec 2020
 * @author wangyc
 *
 * @defgroup NVDS
 * @ingroup HS662X
 * @brief NVDS system
 * @details
 *
 * The NVDS system is used for storing information
 * which should be saved when chip loses power.
 *
 * <pre>
 * Features:
 *   • Two Sectors: one is NVDS sector, the othor one is BKUP sector
 *
 * </pre>
 *
 * @{
 *
 * @example example_nvds.c
 * This is an example of how to use the NVDS on storage.
 *
 */
#ifndef _CFG_NVDS_H_
#define _CFG_NVDS_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>
#include <stdint.h>
#include "rwip_config.h"
#include "nvds_tags.h"
#include "peripheral.h"

#if (NVDS_SUPPORT & NVDS_EN_NEW)
/*
 * DEFINES
 ****************************************************************************************
 */
/// Dynimic sanity with MBR
#define CONFIG_CFG_BASE_DYN_IN_MBR

/// Maximum number of tags
#define NVDS_MAX_NUM_OF_TAGS        255

/// Maximum length of tags
#define NVDS_MAX_LENGTH_OF_TAGS     504

typedef uint16_t nvds_tag_len_t;
typedef uint32_t nvds_addr_len_t;
typedef enum NVDS_TAGS nvds_tag_id_t;

/*
 * ENUMERATION DEFINITIONS
 ****************************************************************************************
 */

/// Possible Returned Status
enum NVDS_STATUS
{
    /// NVDS status OK
    NVDS_OK,
    /// generic NVDS status KO
    NVDS_FAIL,
    /// NVDS TAG unrecognized
    NVDS_TAG_NOT_DEFINED,
    /// No space for NVDS
    NVDS_NO_SPACE_AVAILABLE,
    /// Length violation
    NVDS_LENGTH_OUT_OF_RANGE,
    /// NVDS parameter locked
    NVDS_PARAM_LOCKED,
    /// NVDS corrupted
    NVDS_CORRUPT,
    /// NVDS setup error
    NVDS_SETUP_ERROR,
    /// NVDS crc error
    NVDS_CRC_ERROR
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 * @brief NVDS initialize
 *
 * @param[in] flash_base  The NVDS's base address in flash
 *
 * @return errno
 **/
uint8_t om_nvds_init(nvds_addr_len_t flash_base);

/**
 * @brief get the newest tag
 *
 * @param[in] tag       tag index
 * @param[in] length    pdata buffer length, if it is null, ignore it.
 * @param[out]length'   real readed length, if length(in) > length(real),
 * @param[out] buf      A pointer to the buffer allocated by the caller to be filled with
 *                      the DATA part of the TAG
 *
 * @return errno
 **/
uint8_t om_nvds_get(uint8_t tag, nvds_tag_len_t *length, void *buf);

/**
 * @brief delete tag
 *
 * @param[in] tag  tag index
 *
 * @return errno
 **/
uint8_t om_nvds_del(uint8_t tag);

/**
 * @brief put tag to NVDS sector
 *
 * @param[in] tag       tag index
 * @param[in] length    data length, excluding tag node size, range in [1, 504].
 * @param[in] buf       Pointer to the buffer containing the DATA part of the TAG to add to
 *                      the NVDS
 *
 * @return errno
 **/
uint8_t om_nvds_put(uint8_t tag, nvds_tag_len_t length, const void *buf);


/**
 * @brief Print debug information
 *
 * @param[in] printf_dump_func
 *
 * @return None
 **/
void om_nvds_dump(void *printf_dump_func);

/**
 * @brief NVDS reset
 *
 * @return None
 **/
void om_nvds_reset(void);

#endif /* #if (NVDS_SUPPORT & NVDS_EN_NEW) */
/// @} NVDS
#ifdef __cplusplus
}
#endif

#endif // _CFG_NVDS_H_
