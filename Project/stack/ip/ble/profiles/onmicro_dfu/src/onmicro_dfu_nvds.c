/*********************************************************************
 * @file onmicro_dfu_nvds.c
 * @version V20210119.1.0
 */
 
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "onmicro_dfu.h"
#include "onmicro_dfu_nvds.h"

#include "peripheral.h"
#include "mbr.h"
#include "cfg.h"

const static int mbr_types[] = { // Correspond to enum image_type
    PART_TYPE_APP,
    PART_TYPE_PATCH,
    PART_TYPE_CFG,
    PART_TYPE_USR1,
    PART_TYPE_USR2,
    PART_TYPE_USR3,
    PART_TYPE_USR4
};

static uint8_t onmicro_dfu_nvds_enable_flash(void)
{
    sfs_enable();
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_get_flash(uint32_t addr, uint32_t *lengthPtr, void *buf)
{
    sfs_read(addr, buf, *lengthPtr);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_put_flash(uint32_t addr, uint32_t length, void *buf)
{
    sfs_write(addr, buf, length);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_erase_flash(uint32_t addr, uint32_t length)
{
    sfs_erase(addr, length);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_disable_flash(void)
{
    //sfs_disable();
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}

static uint8_t onmicro_dfu_nvds_enable_mbr(void)
{
    sfs_enable();
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_get_mbr(uint32_t id, uint32_t *lengthPtr, void *buf)
{
    dfu_image_mbr_info *info = (dfu_image_mbr_info*)buf;
    if(id <= sizeof(mbr_types)/sizeof(mbr_types[0]) &&
        mbr_read_part(mbr_types[id], &info->address, &info->length, &info->crc16) == 0){
        return ONMICRO_DFU_NVDS_ST_SUCCESS;
    }else{
        return ONMICRO_DFU_NVDS_ST_FAILED;
    }
}
static uint8_t onmicro_dfu_nvds_put_mbr(uint32_t id, uint32_t length, void *buf)
{
    dfu_image_mbr_info *info = (dfu_image_mbr_info*)buf;
    if(id < sizeof(mbr_types)/sizeof(mbr_types[0])){
#if defined(CONFIG_HS6621A3_RELEASE)
        CO_DISABLE_IRQ();
#endif
        mbr_write_part(mbr_types[id], info->address, info->length, info->crc16);
#if defined(CONFIG_HS6621A3_RELEASE)
        CO_RESTORE_IRQ();
#endif
        return ONMICRO_DFU_NVDS_ST_SUCCESS;
    }else{
        return ONMICRO_DFU_NVDS_ST_FAILED;
    }
}
static uint8_t onmicro_dfu_nvds_del_mbr(uint32_t id, uint32_t length)
{
    return ONMICRO_DFU_NVDS_ST_FAILED;
}
static uint8_t onmicro_dfu_nvds_disable_mbr(void)
{
    //sfs_disable();
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}


static uint8_t onmicro_dfu_nvds_enable_cfg(void)
{
    sfs_enable();
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_get_cfg(uint32_t id, uint32_t *lengthPtr, void *buf)
{
    uint16_t len = *lengthPtr;
    int res = cfg_get(CFG_SECTION_USR, id, buf, &len);
    *lengthPtr = len;
    if(res == 0){
        return ONMICRO_DFU_NVDS_ST_SUCCESS;
    }else{
        return ONMICRO_DFU_NVDS_ST_FAILED;
    }
}
static uint8_t onmicro_dfu_nvds_put_cfg(uint32_t id, uint32_t length, void *buf)
{
    int res = cfg_put(CFG_SECTION_USR, id, buf, length);
    if(res == 0){
        return ONMICRO_DFU_NVDS_ST_SUCCESS;
    }else{
        return ONMICRO_DFU_NVDS_ST_FAILED;
    }
}
static uint8_t onmicro_dfu_nvds_del_cfg(uint32_t id, uint32_t length)
{
    return ONMICRO_DFU_NVDS_ST_FAILED;
}
static uint8_t onmicro_dfu_nvds_disable_cfg(void)
{
    //sfs_disable();
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}


static uint8_t onmicro_dfu_nvds_enable_flash_ext(void)
{
    sfs_enable();
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_get_flash_ext(uint32_t addr, uint32_t *lengthPtr, void *buf)
{
    sfs_read(addr, buf, *lengthPtr);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_put_flash_ext(uint32_t addr, uint32_t length, void *buf)
{
    sfs_write(addr, buf, length);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_erase_flash_ext(uint32_t addr, uint32_t length)
{
    sfs_erase(addr, length);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_disable_flash_ext(void)
{
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}


static uint8_t onmicro_dfu_nvds_enable_dummy(void)
{
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_get_dummy(uint32_t id, uint32_t *lengthPtr, void *buf)
{
    memset(buf, 0xFF, *lengthPtr);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_put_dummy(uint32_t id, uint32_t length, void *buf)
{
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_del_dummy(uint32_t id, uint32_t length)
{
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_disable_dummy(void)
{
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}

const dfu_nvds_itf_t dfu_nvds_itf[] = {
    { //DFU_NVDS_ITF_TYPE_MBR,
        onmicro_dfu_nvds_enable_mbr,
        onmicro_dfu_nvds_get_mbr,
        onmicro_dfu_nvds_put_mbr,
        onmicro_dfu_nvds_del_mbr,
        onmicro_dfu_nvds_disable_mbr
    },
    { //DFU_NVDS_ITF_TYPE_FLASH,
        onmicro_dfu_nvds_enable_flash,
        onmicro_dfu_nvds_get_flash,
        onmicro_dfu_nvds_put_flash,
        onmicro_dfu_nvds_erase_flash,
        onmicro_dfu_nvds_disable_flash
    },
    { //DFU_NVDS_ITF_TYPE_CFG,
        onmicro_dfu_nvds_enable_cfg,
        onmicro_dfu_nvds_get_cfg,
        onmicro_dfu_nvds_put_cfg,
        onmicro_dfu_nvds_del_cfg,
        onmicro_dfu_nvds_disable_cfg
    },
    { //DFU_NVDS_ITF_TYPE_EXT_FLASH,
        onmicro_dfu_nvds_enable_flash_ext,
        onmicro_dfu_nvds_get_flash_ext,
        onmicro_dfu_nvds_put_flash_ext,
        onmicro_dfu_nvds_erase_flash_ext,
        onmicro_dfu_nvds_disable_flash_ext
    },
    { //DFU_NVDS_ITF_TYPE_DUMMY,
        onmicro_dfu_nvds_enable_dummy,
        onmicro_dfu_nvds_get_dummy,
        onmicro_dfu_nvds_put_dummy,
        onmicro_dfu_nvds_del_dummy,
        onmicro_dfu_nvds_disable_dummy
    },
};
