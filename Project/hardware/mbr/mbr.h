 /*********************************************************************
 * @file mbr.h
 * @brief 
 * @version 1.0
 * @date 20 April 2015
 * @author luwei
 *
 * @addtogroup mbr
 * @ingroup 
 * @details 
 *
 * @note 
 */

#ifndef _MBR_H
#define _MBR_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>

/*********************************************************************
 * TYPEDEFS
 */

/* Sector 0 */
#define OPT_JTAG_ON      (1 << 0) /* 1-jtag is on;  0-jtag is off */
#define OPT_ROP_OFF      (1 << 1) /* 1-ROP is off;  0-ROP is on   */
#define OPT_ENC_OFF      (1 << 2) /* 1-Enc is off;  0-Enc is on   */
#define OPT_NODELAY      (0xF<<4) /* Not delay */
#define OPT_NODELAY_FLAG (0xA<<4) /* Not delay */

#define OPT_BYTES_OFFSET 0
#define ENC_KEY_OFFSET   4
#define SEC_KEY_LEN      16 /* encryption key, private key in 128-bit */

/* OTP */
#define OTP_CHIP_ID      1 /* LB1 */
#define OTP_CHIP_FUNC    2 /* LB2 */
#define OTP_AUTH         3 /* LB3 */ 

/* Vectors */
//#define VEC_APP_SIZE     8
//#define VEC_APP_CRC      9
#define VEC_RUN2SF_MAGIC 10
#define VEC_APP_MAGIC    13

/* Security Registers */
#define SEC_CHIP_ID_OFFSET   (0*256)
#define SEC_CHIP_FUNC_OFFSET (1*256)
#define SEC_AUTH_PRIV_OFFSET (2*256)

/* Reserved flash sector */
#define RSV_TRACE_ID_OFFSET    12
#define RSV_TRACE_ID_LENGTH    8
#define RSV_GPADC_CALI_OFFSET  8
#define RSV_GPADC_CALI_LENGTH  8

#define OPT_SECTOR_START 0
#define MBR_SECTOR_START 1
#define APP_SECTOR_START 3

/* Sector 1 */
#define MBR_SIGNATURE UINT16_C(0xAA55)

#define FLASH_SETTINGS_SIZE     3
#define FLASH_CLOCK_OFFSET	    0x17b
#define FLASH_WIDTH_OFFSET	    0x17c
#define FLASH_DELAY_OFFSET	    0x17d
#define DOS_PART_TBL_OFFSET	    0x17e
#define DOS_PART_MAGIC_OFFSET	0x1fe

#define PART_TYPE_MASK_HS 0x60
#define PART_TYPE_APP     0x60
#define PART_TYPE_PATCH   0x61
#define PART_TYPE_CFG     0x62
#define PART_TYPE_RFU     0x63
#define PART_TYPE_USR1    0x04
#define PART_TYPE_USR2    0x05
#define PART_TYPE_USR3    0x06
#define PART_TYPE_USR4    0x07
#define MBR_PART_NUM      8


// Data for a single MBR partition record
// Note that firstSector and lastSector are in CHS addressing, which
// splits the bits up in a weird way.
// On read or write of MBR entries, firstLBA is an absolute disk sector.
// On read of logical entries, it's relative to the EBR record for that
// partition. When writing EBR records, it's relative to the extended
// partition's start.

typedef struct MBRRecord {
  uint8_t status;
  uint8_t firstSector[3];
  uint8_t partitionType;
  uint8_t lastSector[1];
  uint16_t crc16;
  uint32_t firstLBA;  /* starting sector counting from 0	*/
  uint32_t lengthLBA; /* nr of sectors in partition		*/
} mbr_part_t;

int mbr_get_security(uint32_t *p_option_bytes, uint8_t *p_key);
int mbr_set_security(uint32_t option_bytes, uint8_t *p_key);
int sec_get_chip_id(uint8_t *p_id);
int sec_get_max_app_size(uint32_t *p_app_size);
int sec_get_auth_pri(uint8_t *p_pri);

int mbr_probe(void);
int mbr_read_part(int img_type, uint32_t *p_addr, uint32_t *p_len, uint16_t *p_crc);
int mbr_alloc_part(int img_type, uint32_t *p_addr, uint32_t len);
int mbr_write_part(int img_type, uint32_t addr, uint32_t len, uint16_t crc);
int mbr_write_clock(uint8_t clock, uint8_t width, uint8_t delay);
void mbr_boost_clock(void);
void mbr_restore_clock(void);
int mbr_validate_app(uint32_t addr, uint32_t len);

int mbr_get_app(uint32_t *p_addr, uint32_t *p_len, uint16_t *p_crc);
int mbr_get_patch(uint32_t *p_addr, uint32_t *p_len, uint16_t *p_crc);
int mbr_get_cfg(uint32_t *p_addr, uint32_t *p_len, uint16_t *p_crc);

/**
 * @brief get cpft address and length
 *
 * @param[out] p_addr  cpft base address
 * @param[out] p_len  cpft length
 *
 * @return errno or 0 success
 **/
int mbr_get_cpft(uint32_t *p_addr, uint32_t *p_len);

/**
 * @brief get free space address and length
 *
 * @param[out] p_addr  free space address
 * @param[out] p_len  freee space length
 *
 * @return errno or 0 success
 **/
int mbr_get_free_area(uint32_t *p_addr, uint32_t *p_len);

/**
 * @brief mbr_should_run_to_sflash()
 *
 * @return 
 **/
bool mbr_should_run_to_sflash(void);

#ifdef __cplusplus
}
#endif
#endif
