/*********************************************************************
 * @file onmicro_dfu_nvds.h
 * @version V20210104.1.0
 */

#ifndef __ONMICRO_DFU_NVDS_H__
#define __ONMICRO_DFU_NVDS_H__
#include <stdint.h>

#define ONMICRO_DFU_NVDS_ST_SUCCESS 0
#define ONMICRO_DFU_NVDS_ST_FAILED  0xFF

enum image_type{
    IMAGE_TYPE_APP,
    IMAGE_TYPE_PATCH,
    IMAGE_TYPE_CONFIG,
    IMAGE_TYPE_MBR_USR1,
    IMAGE_TYPE_MBR_USR2,
    IMAGE_TYPE_MBR_USR3,
    IMAGE_TYPE_MBR_USR4,
    IMAGE_TYPE_MBR_MAX,
    IMAGE_TYPE_DUMMY = IMAGE_TYPE_MBR_MAX,
    IMAGE_TYPE_CUSTOM = 0x10,
    IMAGE_TYPE_RAW    = 0x5F,
};

enum dfu_nvds_itf_type{
    DFU_NVDS_ITF_TYPE_MBR,
    DFU_NVDS_ITF_TYPE_FLASH,
    DFU_NVDS_ITF_TYPE_CFG,
    DFU_NVDS_ITF_TYPE_EXT_FLASH,
    DFU_NVDS_ITF_TYPE_DUMMY,
    DFU_NVDS_ITF_TYPE_MAX,
};

typedef struct {
    uint32_t address;
    uint32_t length;
    uint16_t crc16;
}dfu_image_mbr_info;

typedef struct {
    uint32_t address;
    uint32_t length;
    uint32_t version;
}dfu_image_info;

typedef struct {
    uint8_t (*enable)(void);
    uint8_t (*get)(uint32_t id, uint32_t *lengthPtr, void *buf);
    uint8_t (*put)(uint32_t id, uint32_t length, void *buf);
    uint8_t (*del)(uint32_t id, uint32_t length);
    uint8_t (*disable)(void);
}dfu_nvds_itf_t;

extern const dfu_nvds_itf_t dfu_nvds_itf[DFU_NVDS_ITF_TYPE_MAX];

#endif /* __ONMICRO_DFU_NVDS_H__ */
