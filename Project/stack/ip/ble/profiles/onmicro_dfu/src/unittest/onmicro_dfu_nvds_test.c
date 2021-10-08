#include "stdio.h"
#include "assert.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "onmicro_dfu.h"
#include "onmicro_dfu_nvds.h"

#define FLASH_SIZE_BYTE     (16 << 20)
#define EXT_FLASH_SIZE_BYTE (16 << 20)
#define MBR_SIZE_BYTE       (1 << 20)
#define CFG_SIZE_BYTE       (1 << 20)
static uint8_t flash[FLASH_SIZE_BYTE];
static uint8_t ext_flash[EXT_FLASH_SIZE_BYTE];
//static uint8_t mbr[MBR_SIZE_BYTE];
//static uint8_t cfg[CFG_SIZE_BYTE];

#define CFG_SECTION_USR   100
#define PART_TYPE_APP     0x60
#define PART_TYPE_PATCH   0x61
#define PART_TYPE_CFG     0x62
#define PART_TYPE_USR1    0x04
#define PART_TYPE_USR2    0x05
#define PART_TYPE_USR3    0x06
#define PART_TYPE_USR4    0x07
const static int mbr_types[] = { // Correspond to enum image_type
    PART_TYPE_APP,
    PART_TYPE_PATCH,
    PART_TYPE_CFG,
    PART_TYPE_USR1,
    PART_TYPE_USR2,
    PART_TYPE_USR3,
    PART_TYPE_USR4
};

void nvds_init(void)
{
    memset(flash, 0xFF, FLASH_SIZE_BYTE);
    memset(ext_flash, 0xFF, FLASH_SIZE_BYTE);
}

static uint8_t onmicro_dfu_nvds_enable(void)
{
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_get_flash(uint32_t addr, uint32_t *lengthPtr, void *buf)
{
    assert(addr+*lengthPtr<FLASH_SIZE_BYTE);
    memcpy(buf, &flash[addr], *lengthPtr);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_put_flash(uint32_t addr, uint32_t length, void *buf)
{
    assert(addr+length<FLASH_SIZE_BYTE);
    memcpy(&flash[addr], buf, length);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_erase_flash(uint32_t addr, uint32_t length)
{
    assert(addr+length<FLASH_SIZE_BYTE);
    memset(&flash[addr], 0xFF, length);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_disable(void)
{
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
int mbr_read_part(uint8_t id, uint32_t *addr, uint32_t *length, uint16_t *crc16){ *addr = 0x3000; return 0; }
int mbr_write_part(uint8_t id, uint32_t addr, uint32_t length, uint16_t crc16){ return 0; }
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

int cfg_get(uint8_t section, uint8_t id, uint8_t *buf, uint16_t *length){ return 0; }
int cfg_put(uint8_t section, uint8_t id, uint8_t *buf, uint16_t length){ return 0; }
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

static uint8_t onmicro_dfu_nvds_get_flash_ext(uint32_t addr, uint32_t *lengthPtr, void *buf)
{
    assert(addr+*lengthPtr<EXT_FLASH_SIZE_BYTE);
    memcpy(buf, &ext_flash[addr], *lengthPtr);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_put_flash_ext(uint32_t addr, uint32_t length, void *buf)
{
    assert(addr+length<EXT_FLASH_SIZE_BYTE);
    memcpy(&ext_flash[addr], buf, length);
    return ONMICRO_DFU_NVDS_ST_SUCCESS;
}
static uint8_t onmicro_dfu_nvds_erase_flash_ext(uint32_t addr, uint32_t length)
{
    assert(addr+length<EXT_FLASH_SIZE_BYTE);
    memset(&ext_flash[addr], 0xFF, length);
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

const dfu_nvds_itf_t dfu_nvds_itf[] = {
    { //DFU_NVDS_ITF_TYPE_MBR,
        onmicro_dfu_nvds_enable,
        onmicro_dfu_nvds_get_mbr,
        onmicro_dfu_nvds_put_mbr,
        onmicro_dfu_nvds_del_mbr,
        onmicro_dfu_nvds_disable
    },
    { //DFU_NVDS_ITF_TYPE_FLASH,
        onmicro_dfu_nvds_enable,
        onmicro_dfu_nvds_get_flash,
        onmicro_dfu_nvds_put_flash,
        onmicro_dfu_nvds_erase_flash,
        onmicro_dfu_nvds_disable
    },
    { //DFU_NVDS_ITF_TYPE_CFG,
        onmicro_dfu_nvds_enable,
        onmicro_dfu_nvds_get_cfg,
        onmicro_dfu_nvds_put_cfg,
        onmicro_dfu_nvds_del_cfg,
        onmicro_dfu_nvds_disable
    },
    { //DFU_NVDS_ITF_TYPE_EXT_FLASH,
        onmicro_dfu_nvds_enable,
        onmicro_dfu_nvds_get_flash_ext,
        onmicro_dfu_nvds_put_flash_ext,
        onmicro_dfu_nvds_erase_flash_ext,
        onmicro_dfu_nvds_disable
    },
    { //DFU_NVDS_ITF_TYPE_DUMMY,
        onmicro_dfu_nvds_enable,
        onmicro_dfu_nvds_get_dummy,
        onmicro_dfu_nvds_put_dummy,
        onmicro_dfu_nvds_del_dummy,
        onmicro_dfu_nvds_disable
    },
};
