/*********************************************************************
 * @file onmicro_dfu_config.h
 * @version V20210310.1.0
 */

#ifndef __ONMICRO_DFU_CONFIG_H__
#define __ONMICRO_DFU_CONFIG_H__

#include "stdlib.h"
#include "onmicro_dfu_nvds.h"

//#define FLASH_ERASE_SIZE 0x1000   // Image写入地址必须以擦除大小对齐

/******* Warning: 不要在 onmicro_dfu.c 以外的地方include该文件! *******/

// enum image_type{ //Defined in onmicro_dfu_nvds.h
//     IMAGE_TYPE_APP,
//     IMAGE_TYPE_PATCH,
//     IMAGE_TYPE_CONFIG,
//     IMAGE_TYPE_MBR_USR1,
//     IMAGE_TYPE_MBR_USR2,
//     IMAGE_TYPE_MBR_USR3,
//     IMAGE_TYPE_MBR_USR4,
//     IMAGE_TYPE_DUMMY,
//     IMAGE_TYPE_CUSTOM = 0x10,
//     IMAGE_TYPE_RAW    = 0x5F,
// };

#if CONFIG_HS6621C_VROM
#define IMAGE_TYPE_VROM IMAGE_TYPE_MBR_USR2 // 此处定义必须与VROM bin中定义一致
#endif

typedef struct {
	uint16_t type; // @ref enum image_type
	uint32_t base_address1;
	uint32_t base_address2;
	uint32_t max_length;
	const char *describe; // 功能标识，仅用于Debug
    const dfu_nvds_itf_t *image_ops_itf; // Imgage 读写接口
    const dfu_nvds_itf_t *info_ops_itf;  // Image info 存储接口
	uint16_t info_id; //Image 信息在 NVDS 中存放的位置
}dfu_image_t;

#define DFU_APP_DESCRIBE "Version1.0"

const dfu_image_t dfu_image_types[] = {
//----------------------------------------------------------------------------------------------------------
 /* Image type               |  Base address1 |  Base address2 |  Max length |  Image describe  |
        Image ops API                                | Image info ops API                       |  Info ID */
#if !CONFIG_HS6621C_VROM
//----------------------------------------------------------------------------------------------------------
	{IMAGE_TYPE_APP          ,  0x00003000    ,  0x00035000    ,   0x00032000,  "Application"   ,
        &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_FLASH]       , NULL                                     , 0x00    },
//----------------------------------------------------------------------------------------------------------
	{IMAGE_TYPE_MBR_USR2     ,  0x00003000    ,  0x00035000    ,   0x00032000,  "Flash App"     ,
        &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_FLASH]       , NULL                                     , 0x00    },
#else
//----------------------------------------------------------------------------------------------------------
	{IMAGE_TYPE_VROM         ,  0x00023000    ,  0x00023000    ,   0x00020000,  "VROM App"     ,
        &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_FLASH]       , NULL                                     , 0x00    },
#endif
//----------------------------------------------------------------------------------------------------------
	{IMAGE_TYPE_PATCH        ,  0x0007A000    ,  0x00075000    ,   0x00002000,  "Patch"         ,
        &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_FLASH]       , NULL                                     , 0x00    },
//----------------------------------------------------------------------------------------------------------
	{IMAGE_TYPE_CONFIG       ,  0x0007C000    ,  0x00077000    ,   0x00005000,  "Config"        ,
        &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_FLASH]       , NULL                                     , 0x00    },
//----------------------------------------------------------------------------------------------------------
	{IMAGE_TYPE_DUMMY        ,  0x00000000    ,  0x00000000    ,   0xFFFFFFFF,  "Dummy"     ,
        &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_DUMMY]       , NULL                                     , 0x00    },
//----------------------------------------------------------------------------------------------------------
};
const uint8_t dfu_image_types_num = sizeof(dfu_image_types) / sizeof(dfu_image_types[0]);

#endif /* __ONMICRO_DFU_CONFIG_H__ */
