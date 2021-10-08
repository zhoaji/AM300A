/*********************************************************************
 * @file onmicro_dfu.c
 * @version V20210310.1.0
 */

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "onmicro_dfu.h"
#include "onmicro_dfu_config.h"
#include "onmicro_dfu_nvds.h"
#if (DFU_CTRL_SIGN_EN)
#include "uECC.h"
#include "sha256.h"
#define ECC_TYPE uECC_secp192r1()
#define PRIV_KEY_SIZE 24
extern uint8_t dfu_public_key[];
#elif  DFU_FORCE_CHECK_SHA256_EN
#include "sha256.h"
#define PRIV_KEY_SIZE 24
#endif

#if BLE_APP_ONMICRO_DFU

#define ONMICRO_DFU_DEBUG
#if defined(ONMICRO_DFU_DEBUG)
#define dfu_debug(fmt, ...) log_debug("[DFU] " fmt, ## __VA_ARGS__)
#define dfu_debug_array_ex log_debug_array_ex
#else
#define dfu_debug(...)
#define dfu_debug_array_ex(...)
#endif

#define DFU_PKG_ACTIVED_NONE 0
#define DFU_PKG_ACTIVED_CMD  1
#define DFU_PKG_ACTIVED_DATA 2

#define DFU_CMDPKG_OFFSET_MARK   0
#define DFU_CMDPKG_OFFSET_VER    4
#define DFU_CMDPKG_OFFSET_CTRL   8
#define DFU_CMDPKG_OFFSET_IMGCNT 10
#define DFU_CMDPKG_OFFSET_PKG    12
#define DFU_CMDPKG_MARK          0x01BFDF55
#define DFU_MAX_IMG_NUM          4
typedef struct {
    uint32_t type:8;
    uint32_t rvs:24;
    uint32_t version;
    uint32_t size;
}dfu_cmd_img_t;

typedef struct {
    uint32_t type:8;
    uint32_t nvds_itf_type:8;
    uint32_t rvs:16;
    uint32_t new_address;
    uint32_t size;
}dfu_cmd_raw_t;

typedef struct {
    uint8_t     *cmd_obj_buffer;
    uint32_t     obj_size;
    uint32_t     obj_recv_len;
    uint32_t     obj_crc32;
    uint8_t      obj_actived;
    uint8_t      prn;
    uint8_t      prn_cnt;
    int8_t       cmd_img_idx;
    uint32_t     cmd_img_size;
    uint32_t     cmd_img_recv_len;
    uint32_t     cmd_img_new_addr;
    const dfu_image_t *cmd_img_info;
    const dfu_nvds_itf_t *image_ops_itf;
    bool         cmd_obj_buffer_valid;
    uint8_t      status;
    uint16_t     cache_recv_len;
}dfu_env_t;

static dfu_env_t *p_env, env;
static uint32_t m_data_crc;
static uint32_t m_data_offset;
__ALIGNED(16) static uint8_t m_cache[DFU_CACHE_BUF_SIZE];
__ALIGNED(4) static uint8_t m_cmd_buf[DFU_COMMAND_OBJ_MAX_SIZE];


static void *dfu_malloc_cmd(uint16_t size)
{
    return &m_cmd_buf;
}
static void *dfu_malloc_env(uint16_t size)
{
    return &env;
}
static void dfu_free(void *p)
{
    return;
}
static dfu_env_t* dfu_env_init(void)
{
    if(p_env == NULL){
        p_env = (dfu_env_t*)dfu_malloc_env(sizeof(dfu_env_t));
        memset(p_env, 0, sizeof(dfu_env_t));
        if(dfu_cb_itf.dfu_begin_cb){
            dfu_cb_itf.dfu_begin_cb(0, NULL);
        }
    }
    return p_env;
}

static  uint32_t dfu_crc32(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc)
{
    uint32_t crc;
    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++){
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--) crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
    }
    return ~crc;
}

static uint32_t get_new_img_address(dfu_cmd_img_t *img, const dfu_image_t *cmd_img_info)
{
	// base_address2 is used if no info saved or read info failed
	uint32_t new_addr = cmd_img_info->base_address2;
	if(img->type < IMAGE_TYPE_MBR_MAX){ // app,patch,cfg info should be read from MBR
		const dfu_nvds_itf_t *nvds_itf = (const dfu_nvds_itf_t*)&dfu_nvds_itf[DFU_NVDS_ITF_TYPE_MBR];
		uint32_t len = sizeof(dfu_image_mbr_info);
		dfu_image_mbr_info info;
		nvds_itf->enable();
		uint8_t res = nvds_itf->get(img->type, &len, &info);
		nvds_itf->disable();
		dfu_assert(res != ONMICRO_DFU_NVDS_ST_SUCCESS || len == sizeof(dfu_image_mbr_info));
		if(res == ONMICRO_DFU_NVDS_ST_SUCCESS){
			if(cmd_img_info->base_address2 <= info.address &&
					info.address <= cmd_img_info->base_address2 + cmd_img_info->max_length){
				new_addr = cmd_img_info->base_address1;
			}
		}
	}else if(img->type < IMAGE_TYPE_RAW){ // IMAGE_TYPE_CUSTOM images info read from it's own nvds itf
		const dfu_nvds_itf_t *nvds_itf = cmd_img_info->info_ops_itf;
		if(nvds_itf){
			uint32_t len = sizeof(dfu_image_info);
			dfu_image_info info;
			nvds_itf->enable();
			uint8_t res = nvds_itf->get(img->type, &len, &info);
			nvds_itf->disable();
			dfu_assert(res != ONMICRO_DFU_NVDS_ST_SUCCESS || len == sizeof(dfu_image_info));
			if(res == ONMICRO_DFU_NVDS_ST_SUCCESS){
				if(cmd_img_info->base_address2 <= info.address &&
						info.address <= cmd_img_info->base_address2 + cmd_img_info->max_length){
					new_addr = cmd_img_info->base_address1;
				}
			}
		}
	}else if(img->type == IMAGE_TYPE_RAW){
        new_addr = cmd_img_info->base_address1;
    }else{
        dfu_assert(0);
    }
    return new_addr;
}

static void update_env_img_info_by_idx(void)
{
    uint8_t *cmd = p_env->cmd_obj_buffer;
    dfu_cmd_img_t *imgs = (dfu_cmd_img_t*)&cmd[DFU_CMDPKG_OFFSET_PKG];
    dfu_cmd_img_t *img = &imgs[p_env->cmd_img_idx];
    p_env->cmd_img_size = img->size;
    p_env->cmd_img_recv_len = 0;
    int j;
    if(img->type == IMAGE_TYPE_RAW){
        p_env->cmd_img_info = NULL;
        p_env->image_ops_itf = &dfu_nvds_itf[((dfu_cmd_raw_t*)img)->nvds_itf_type];
        //Cal new address
        dfu_image_t raw_img_info = {
            IMAGE_TYPE_RAW,
            ((dfu_cmd_raw_t*)img)->new_address,
            ((dfu_cmd_raw_t*)img)->new_address,
        };
        p_env->cmd_img_new_addr = get_new_img_address(img, &raw_img_info);
    }else{
        for(j=0;j<dfu_image_types_num;j++){
            if(img->type == dfu_image_types[j].type){
                p_env->cmd_img_info = &dfu_image_types[j];
                p_env->image_ops_itf = p_env->cmd_img_info->image_ops_itf;
                //Cal new address
                p_env->cmd_img_new_addr = get_new_img_address(img, p_env->cmd_img_info);
                break;
            }
        }
        dfu_assert(j<dfu_image_types_num);
    }
    dfu_debug("Flashing new_addr:0x%08X, size: %d (%s)\n",
                p_env->cmd_img_new_addr, img->size, img->type==IMAGE_TYPE_RAW?"Raw Data":p_env->cmd_img_info->describe);
}
static void write_image_data(uint8_t *data, uint32_t len)
{
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
    uint32_t pos = 0;
    while(pos < len){
        if(p_env->cmd_img_size == p_env->cmd_img_recv_len){ // Current image received done.
            uint16_t img_cnt = *(uint16_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_IMGCNT];
            if(img_cnt == p_env->cmd_img_idx + 1){ // All images received done.
                break;
            }
            p_env->cmd_img_idx++;
            update_env_img_info_by_idx();
            p_env->cache_recv_len = 0;
        }
        // Copy data to m_cache
        uint32_t remain_data = len - pos;
        uint32_t remain_cache = DFU_CACHE_BUF_SIZE - p_env->cache_recv_len;
        uint32_t remain_image = p_env->cmd_img_size - p_env->cmd_img_recv_len;
        uint32_t copy2cache_len = MIN(MIN(remain_data, remain_cache), remain_image);
        memcpy(&m_cache[p_env->cache_recv_len], &data[pos], copy2cache_len);
        pos += copy2cache_len;
        p_env->cache_recv_len += copy2cache_len;
        p_env->cmd_img_recv_len += copy2cache_len;

        // Write m_cache to flash
        if(p_env->cache_recv_len == DFU_CACHE_BUF_SIZE || p_env->cmd_img_recv_len == p_env->cmd_img_size){
            const dfu_nvds_itf_t *write_itf = p_env->image_ops_itf;
            write_itf->enable();
            uint32_t write_addr = p_env->cmd_img_new_addr+p_env->cmd_img_recv_len-p_env->cache_recv_len;
#if FLASH_ERASE_SIZE
            uint32_t erase_addr = (write_addr + FLASH_ERASE_SIZE - 1) & ~(FLASH_ERASE_SIZE - 1);
            for(;erase_addr < write_addr + p_env->cache_recv_len;erase_addr += FLASH_ERASE_SIZE){
                dfu_debug("Erasing 0x%08X-0x%08X (%s)\n", erase_addr, erase_addr+FLASH_ERASE_SIZE, p_env->cmd_img_info->describe);
                write_itf->del(erase_addr, FLASH_ERASE_SIZE);
            }
#endif
            write_itf->put(write_addr, p_env->cache_recv_len, m_cache);
            write_itf->disable();
            m_data_crc = dfu_crc32(m_cache, p_env->cache_recv_len, &m_data_crc);
            m_data_offset += p_env->cache_recv_len;
            p_env->cache_recv_len = 0;
        }
    }
}
static void update_env_img_info_by_cache(void)
{
    dfu_cmd_img_t *imgs = (dfu_cmd_img_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_PKG];
    uint16_t i, img_cnt = *(uint16_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_IMGCNT];
    p_env->obj_crc32 = m_data_crc;
    p_env->obj_recv_len = m_data_offset;
    p_env->obj_size = 0;
    p_env->cmd_img_idx = -1;
    for(i=0;i<img_cnt;i++){
        dfu_cmd_img_t *img = &imgs[i];
        p_env->obj_size += img->size;
        if(p_env->cmd_img_idx == -1 && p_env->obj_size > m_data_offset){
            p_env->cmd_img_idx = i;
            p_env->cmd_img_size = img->size;
            p_env->cmd_img_recv_len = m_data_offset - (p_env->obj_size - img->size);
        }
    }
    dfu_cmd_img_t *img = &imgs[p_env->cmd_img_idx];
    if(img->type == IMAGE_TYPE_RAW){
        p_env->cmd_img_new_addr = ((dfu_cmd_raw_t*)&imgs[p_env->cmd_img_idx])->new_address;
        p_env->cmd_img_info = NULL;
        uint32_t itf_type_idx = ((dfu_cmd_raw_t*)&imgs[p_env->cmd_img_idx])->nvds_itf_type;
        if(itf_type_idx >= DFU_NVDS_ITF_TYPE_MAX){
            m_data_offset = 0; // Param invalid, reset reception
        }else{
            p_env->image_ops_itf = &dfu_nvds_itf[itf_type_idx];
        }
    }else{
        int j;
        for(j=0;j<dfu_image_types_num;j++){
            if(img->type == dfu_image_types[j].type){
                p_env->cmd_img_info = &dfu_image_types[j];
                p_env->image_ops_itf = p_env->cmd_img_info->image_ops_itf;
                p_env->cmd_img_new_addr = get_new_img_address(img, p_env->cmd_img_info);
                break;
            }
        }
    }
}
static uint32_t cal_cmd_obj_size(uint8_t *cmd)
{
    uint16_t img_cnt = *(uint16_t*)&cmd[DFU_CMDPKG_OFFSET_IMGCNT];
//    dfu_cmd_img_t *imgs = (dfu_cmd_img_t*)&cmd[DFU_CMDPKG_OFFSET_PKG];
    // Check package length
    uint32_t res = sizeof(dfu_cmd_img_t) * img_cnt + DFU_CMDPKG_OFFSET_PKG;
#if (DFU_CTRL_SIGN_EN)
    res += PRIV_KEY_SIZE*2 + SHA256_BLOCK_SIZE;
#endif
    return res;
}

void dfu_reset(uint8_t state)
{
    if(p_env != NULL){
        uint16_t ctrl_flag = 0xFFFF;
        if(p_env->cmd_obj_buffer != NULL){
            ctrl_flag = *(uint16_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_CTRL];
            dfu_free(p_env->cmd_obj_buffer);
        }
        dfu_free(p_env);
        p_env = NULL;
        if(dfu_cb_itf.dfu_end_cb){
            dfu_cb_itf.dfu_end_cb(state, &ctrl_flag);
        }
    }
}

void dfu_write_cmd(uint8_t *data, uint32_t len, dfu_response_t *response)
{
    uint8_t opcode = data[0];
    response->rsp_code = DFU_CTRL_RESPONSE;
    response->opcode = opcode;
    response->result = DFU_SUCCESS;
    response->length = DFU_RESP_SIZE_NO_EXT_DATA;
    if(env.status != DFU_STATUS_ENABLED){
        dfu_debug("DFU disabled: %d.", env.status);
        response->result = DFU_OPCODE_NOT_SUPPORT;
        return;
    }
    switch(opcode){
        case DFU_CTRL_CREATE:{
            // PKG: 01 TYPE LL LL LL LL (LL: size)
            uint8_t type = data[1];
            uint32_t length = (data[2]<<0)|(data[3]<<8)|(data[4]<<16)|(data[5]<<24);
            dfu_env_init();
            p_env->obj_size = length;
            p_env->obj_recv_len = 0;
            p_env->obj_crc32 = 0;
            if(type == DFU_PKG_ACTIVED_CMD){
                if(length > DFU_COMMAND_OBJ_MAX_SIZE){
                    dfu_debug("Image length exceed DFU_COMMAND_OBJ_MAX_SIZE(0x%08X)", DFU_COMMAND_OBJ_MAX_SIZE);
                    response->result = DFU_INSUFFICIENT_RESOURCES;
                    break;
                }else{
                    if(p_env->cmd_obj_buffer == NULL){
                        p_env->cmd_obj_buffer = dfu_malloc_cmd(length);
                        dfu_assert(!((size_t)p_env->cmd_obj_buffer & (sizeof(uint32_t)-1)));
                        if(p_env->cmd_obj_buffer == NULL){
                            dfu_debug("Failed to malloc cmd_obj_buffer, size:0x%02X", length);
                            response->result = DFU_INSUFFICIENT_RESOURCES;
                            break;
                        }
                    }
                }
            }else if(type == DFU_PKG_ACTIVED_DATA){
                uint32_t sum_img_len = 0;
                int i,j;
                uint16_t img_cnt = *(uint16_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_IMGCNT];
                dfu_cmd_img_t *imgs = (dfu_cmd_img_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_PKG];
                for(i=0;i<img_cnt;i++){
                    dfu_image_t raw_img_info = { IMAGE_TYPE_RAW };
                    dfu_cmd_img_t *img = &imgs[i];
                    const dfu_image_t *cmd_img_info = NULL;
                    sum_img_len += img->size;
                    for(j=0;j<dfu_image_types_num;j++){
                        if(img->type == dfu_image_types[j].type){
                            cmd_img_info = &dfu_image_types[j];
                            break;
                        }else if(img->type == IMAGE_TYPE_RAW){
                            cmd_img_info = &raw_img_info;
                            raw_img_info.base_address1 = ((dfu_cmd_raw_t*)img)->new_address;
                            raw_img_info.base_address2 = ((dfu_cmd_raw_t*)img)->new_address;
                            raw_img_info.image_ops_itf = &dfu_nvds_itf[((dfu_cmd_raw_t*)img)->nvds_itf_type];
                            raw_img_info.describe = "Raw data";
                        }
                    }
                    dfu_assert(cmd_img_info);
#if !FLASH_ERASE_SIZE
                    // Get address for new image
                    uint32_t new_address = get_new_img_address(img, cmd_img_info);
                    // Erase flash for new image
                    const dfu_nvds_itf_t *nvds_itf = cmd_img_info->image_ops_itf;
                    dfu_debug("Erasing 0x%08X-0x%08X (%s)\n", new_address, new_address+img->size, cmd_img_info->describe);
                    nvds_itf->enable();
                    nvds_itf->del(new_address, img->size);
                    nvds_itf->disable();
#endif
                }

                p_env->cmd_img_idx = -1;
                p_env->cmd_img_size = p_env->cmd_img_recv_len = p_env->cache_recv_len = 0;
                m_data_offset = m_data_crc = 0;
                if(sum_img_len != length){
                    dfu_debug("Created size NOT match to images\n");
                    response->result = DFU_INVALID_OBJECT;
                }
            }else{
                response->result = DFU_INVALID_PARAMETER;
            }
        }   break;
        case DFU_CTRL_SET_PRN:{
            // PKG: 02 LL LL (LL: PRN)
            if(p_env != NULL){
                p_env->prn = data[1]|(data[2]<<8);
            }else{
                response->result = DFU_OPERATION_FAILED;
            }
        }   break;
        case DFU_CTRL_CAL_CHECKSUM:{
            // PKG: 03
            response->length = DFU_RESP_SIZE_CHECKSUM;
            if(p_env != NULL){
                response->data.checksum.offset = p_env->obj_recv_len;
                response->data.checksum.crc32 = p_env->obj_crc32;
            }else{
                response->result = DFU_OPERATION_FAILED;
            }
        }   break;
        case DFU_CTRL_EXECTUE:{
            // PKG: 04
            if(p_env && p_env->cmd_obj_buffer){
                if(p_env->obj_size != p_env->obj_recv_len){
                    response->result = DFU_INVALID_OBJECT;
                    break;
                }
                if(p_env->obj_actived == DFU_PKG_ACTIVED_CMD){
                    uint8_t *cmd = p_env->cmd_obj_buffer;
                    // Check header mark
                    if(p_env->obj_recv_len < DFU_CMDPKG_OFFSET_PKG ||
                        *(uint32_t*)&cmd[DFU_CMDPKG_OFFSET_MARK] != DFU_CMDPKG_MARK){
                        response->result = DFU_INVALID_CODE;
                        break;
                    }
#if (DFU_CTRL_SIGN_EN)
                    uint16_t ctrl_flag = *(uint16_t*)&cmd[DFU_CMDPKG_OFFSET_CTRL];
                    if(!(ctrl_flag & DFU_CTRL_BIT_SIGN)){
                        dfu_debug("Packet should be signed\n");
                        response->result = DFU_INVALID_PARAMETER;
                        break;
                    }
                    uint8_t hash[SHA256_BLOCK_SIZE];
                    SHA256_CTX ctx;
                    sha256_init(&ctx);
                    sha256_update(&ctx, cmd, p_env->obj_size - PRIV_KEY_SIZE*2);
                    sha256_final(&ctx, hash);
                    int res = uECC_verify(dfu_public_key, hash, SHA256_BLOCK_SIZE, cmd + p_env->obj_size - PRIV_KEY_SIZE*2, ECC_TYPE);
                    if(!res){
                        dfu_debug("Error signature\n");
                        response->result = DFU_OPERATION_NOT_PERMITTED;
                        break;
                    }
#endif
                    uint32_t exp_length = cal_cmd_obj_size(cmd);
                    if(exp_length > p_env->obj_recv_len){
                        dfu_debug("Error length of data received: 0x%08X(0x%08X expected)",
                                    p_env->obj_recv_len, exp_length);
                        response->result = DFU_INVALID_PARAMETER;
                        break;
                    }

                    // Check Images type & size
                    uint16_t img_cnt = *(uint16_t*)&cmd[DFU_CMDPKG_OFFSET_IMGCNT];
                    dfu_cmd_img_t *imgs = (dfu_cmd_img_t*)&cmd[DFU_CMDPKG_OFFSET_PKG];
                    int i, j;
                    for(i=0;i<img_cnt&&sizeof(dfu_cmd_img_t)*i+DFU_CMDPKG_OFFSET_PKG<p_env->obj_recv_len;i++){
                        response->result = DFU_UNSUPPORTED_TYPE;
                        for(j=0;j<dfu_image_types_num;j++){
                            if(imgs[i].type == dfu_image_types[j].type){ // Is type available
                                if(imgs[i].size <= dfu_image_types[j].max_length){ // Is size available
                                    response->result = DFU_SUCCESS;
                                }else{
                                    dfu_debug("The image '%s' execeeds the size limit.(%d>%d)\n",
                                        dfu_image_types[j].describe, imgs[i].size, dfu_image_types[j].max_length);
                                    response->result = DFU_INSUFFICIENT_RESOURCES;
                                }
                                break;
                            }else if(imgs[i].type == IMAGE_TYPE_RAW){
                                response->result = DFU_SUCCESS;
                                break;
                            }
                        }
#if CONFIG_HS6621C_VROM
                        uint16_t ctrl_flag = *(uint16_t*)&cmd[DFU_CMDPKG_OFFSET_CTRL];
                        if(imgs[i].type == IMAGE_TYPE_DUMMY && (ctrl_flag & DFU_CTRL_BIT_MORE_IMG)){
                            uint32_t version = imgs[i].version;
                            extern int VROM_ID;
                            if(version != 0 && version != 0xFFFFFFFF && version != (size_t)&VROM_ID){
                                response->result = DFU_VERSION_NOT_MATCH;
                                dfu_debug("ROM ID(0x%02X) does NOT match:  image(0x%02X)\n", (size_t)&VROM_ID, version);
                                break;
                            }
                        }else if(imgs[i].type == IMAGE_TYPE_VROM){
                            uint32_t version = imgs[i].version;
                            extern int VROM_ID;
                            if(version != (size_t)&VROM_ID){
                                response->result = DFU_VERSION_NOT_MATCH;
                                dfu_debug("ROM ID(0x%02X) does NOT match:  image(0x%02X)\n", (size_t)&VROM_ID, version);
                                break;
                            }
                        }
#endif
                        if(response->result != DFU_SUCCESS){
                            dfu_debug("Not supported image type: 0x%02X\n", imgs[i].type);
                            break;
                        }
                    }
                    if(response->result == DFU_SUCCESS){
                        p_env->cmd_obj_buffer_valid = true;
                    }
                }else if(p_env->obj_actived == DFU_PKG_ACTIVED_DATA){
                    m_data_offset = 0; // m_cache used for cal crc32, so reset offest.
                    if(p_env->cmd_obj_buffer_valid){
                        uint16_t img_cnt = *(uint16_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_IMGCNT];
                        dfu_cmd_img_t *imgs = (dfu_cmd_img_t*)&p_env->cmd_obj_buffer[DFU_CMDPKG_OFFSET_PKG];
                        dfu_assert(img_cnt <= DFU_MAX_IMG_NUM);
                        uint32_t cmd_img_new_addr[DFU_MAX_IMG_NUM], obj_crc32 = 0;
#if (DFU_CTRL_SIGN_EN | DFU_FORCE_CHECK_SHA256_EN)
                        uint8_t imgs_hash[SHA256_BLOCK_SIZE];
                        SHA256_CTX imgs_ctx;
                        sha256_init(&imgs_ctx);
#endif

                        const dfu_nvds_itf_t *info_ops_itf[DFU_MAX_IMG_NUM];// used for nvds info update
                        uint8_t info_id[DFU_MAX_IMG_NUM];

                        uint16_t sys_crc16[IMAGE_TYPE_MBR_MAX] = {0}; // used for mbr update

                        uint32_t i, j;  static uint32_t TEST_LEN = 0;
                        for(i=0;i<img_cnt;i++){ // Cal all CRC32
                            dfu_cmd_img_t *img = &imgs[i];
                            dfu_image_t raw_img_info = { IMAGE_TYPE_RAW };
                            const dfu_image_t *cmd_img_info = NULL;
                            cmd_img_new_addr[i] = 0;
                            info_ops_itf[i] = NULL;
                            for(j=0;j<dfu_image_types_num;j++){
                                if(img->type == dfu_image_types[j].type){
                                    cmd_img_info = &dfu_image_types[j];
                                    //Cal new address
                                    cmd_img_new_addr[i] = get_new_img_address(img, cmd_img_info);
                                    info_ops_itf[i] = cmd_img_info->info_ops_itf;
                                    info_id[i] = cmd_img_info->info_id;
                                    break;
                                }else if(img->type == IMAGE_TYPE_RAW){
                                    //Cal new address
                                    raw_img_info.base_address1 = ((dfu_cmd_raw_t*)img)->new_address;
                                    raw_img_info.base_address2 = ((dfu_cmd_raw_t*)img)->new_address;
                                    raw_img_info.image_ops_itf = &dfu_nvds_itf[((dfu_cmd_raw_t*)img)->nvds_itf_type];
                                    cmd_img_info = &raw_img_info;
                                    cmd_img_new_addr[i] = get_new_img_address(img, &raw_img_info);
                                    break;
                                }
                            }
                            dfu_assert(j != dfu_image_types_num);

                            cmd_img_info->image_ops_itf->enable();
                            for(j=cmd_img_new_addr[i];j<cmd_img_new_addr[i]+img->size;){
                                // TODO: If it takes too long to calculate the CRC, you need to add an action here to restart the watchdog timer
                                uint32_t len = MIN(DFU_CACHE_BUF_SIZE, cmd_img_new_addr[i]+img->size-j);
                                cmd_img_info->image_ops_itf->get(j, &len, m_cache);
                                obj_crc32 = dfu_crc32(m_cache, len, &obj_crc32); //Cal all images'CRC
#if (DFU_CTRL_SIGN_EN | DFU_FORCE_CHECK_SHA256_EN)
                                sha256_update(&imgs_ctx, m_cache, len);
#endif
                                TEST_LEN += len;
                                if(img->type < IMAGE_TYPE_MBR_MAX){
                                    sys_crc16[img->type] = co_crc16_ccitt(sys_crc16[img->type], m_cache, len);
                                }
                                j += len;
                            }
                            cmd_img_info->image_ops_itf->disable();
                            //mbr_validate_app(new_base_addr, ew_size);
                        }
                        //All crc cal done.
                        if(obj_crc32 != p_env->obj_crc32){
                            dfu_debug("Data object CRC NOT matched.\n");
                            response->result = DFU_INSUFFICIENT_RESOURCES;
                            break;
                        }
#if (DFU_CTRL_SIGN_EN | DFU_FORCE_CHECK_SHA256_EN)
                        sha256_final(&imgs_ctx, imgs_hash);
                        //All hash cal done.
                        int sha256_result;
#if (DFU_CTRL_SIGN_EN)
                        uint8_t *p_hash = p_env->cmd_obj_buffer + cal_cmd_obj_size(p_env->cmd_obj_buffer) - PRIV_KEY_SIZE * 2 - SHA256_BLOCK_SIZE;
                        sha256_result = memcmp(imgs_hash, p_hash, SHA256_BLOCK_SIZE);
#elif (DFU_FORCE_CHECK_SHA256_EN)
                        sha256_result = dfu_sha256_cmp(imgs_hash, SHA256_BLOCK_SIZE);
#endif
                        if(sha256_result){
                            dfu_debug("Data object Hash NOT matched.\n");
                            response->result = DFU_OPERATION_NOT_PERMITTED;
                            break;
                        }
#endif
						//Enable sys image info in MBR
                        const dfu_nvds_itf_t *mbr_itf = &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_MBR];
                        mbr_itf->enable();
                        for(i=0;i<img_cnt;i++){
                            dfu_cmd_img_t *img = &imgs[i];
                            if(img->type < IMAGE_TYPE_MBR_MAX){ //system image
                                dfu_image_mbr_info info = {
                                    cmd_img_new_addr[i],
                                    img->size,
                                    sys_crc16[img->type],
                                };
                                dfu_debug("Update MBR(%d) Addr: 0x%08X CRC: 0x%08X Size: %d.\n", img->type,
                                            info.address, info.crc16, info.length);
                                int res = mbr_itf->put(img->type, sizeof(info), &info);
                                if(res != ONMICRO_DFU_NVDS_ST_SUCCESS){
                                    dfu_debug("Update MBR(%d) failed.\n", img->type);
                                    response->result = DFU_INSUFFICIENT_RESOURCES;
                                    break;
                                }
                            }
                        }
                        mbr_itf->disable();

						//Update all image info in NVDS
                        for(i=0;i<img_cnt;i++){
                            dfu_cmd_img_t *img = &imgs[i];
                            const dfu_nvds_itf_t *info_itf = info_ops_itf[i];
                            if(info_itf){
                                info_itf->enable();
                                dfu_image_info info = {
                                    cmd_img_new_addr[i],
                                    img->size,
                                    img->version,
                                };
                                int res = info_itf->put(info_id[i], sizeof(info), &info);
                                if(res != ONMICRO_DFU_NVDS_ST_SUCCESS){
                                    dfu_debug("Update NVDS info(%d) failed.\n", img->type);
                                }
                                info_itf->disable();
                            }
                        }
                    }else{
                        dfu_debug("Command buffer not inited.\n");
                        response->result = DFU_INVALID_PARAMETER;
                    }
                    if(response->result == DFU_SUCCESS){
                        env.status = DFU_STATUS_LOCKED; // Update successfully. DFU disabled until reboot.
                        dfu_reset(DFU_UPDATE_ST_SUCCESS);
                    }
                }else{
                    response->result = DFU_OPERATION_FAILED;
                }
            }else{
                response->result = DFU_OPERATION_FAILED;
            }
        }   break;
        case DFU_CTRL_SELECT:{
            // PKG: 06 XX (XX: Type)
            uint8_t type = data[1];
            dfu_env_init();
            response->length = DFU_RESP_SIZE_SELECT;
            response->data.select_data.max_size = DFU_DATA_MAX_SIZE;
            p_env->prn_cnt = 0;
            if(type == DFU_PKG_ACTIVED_CMD){
                p_env->obj_actived = DFU_PKG_ACTIVED_CMD;
                response->data.select_data.offset = 0;
                response->data.select_data.crc32 = 0;
            }else if(type == DFU_PKG_ACTIVED_DATA){
                p_env->obj_actived = DFU_PKG_ACTIVED_DATA;
                if(m_data_offset > 0 && p_env->cmd_obj_buffer_valid){ // Cal current image info
                    update_env_img_info_by_cache();
                    p_env->cache_recv_len = 0;
                }else{
                    p_env->cmd_img_idx = -1;
                    m_data_offset = 0;
                }
                response->data.select_data.offset = m_data_offset;
                response->data.select_data.crc32 = m_data_crc;
            }else{
                response->result = DFU_INVALID_OBJECT;
            }
        }   break;
        default:{
            response->result = DFU_OPCODE_NOT_SUPPORT;
        }   break;
    }
    if(response->result != DFU_SUCCESS){
        dfu_reset(DFU_UPDATE_ST_FAILED);
    }
}

void dfu_write_data(uint8_t *data, uint32_t len, dfu_response_t *response)
{
    response->length = DFU_RESP_SIZE_NO_DATA;
    if(env.status != DFU_STATUS_ENABLED){
        return;
    }
    if(!p_env || !p_env->cmd_obj_buffer){
        return;
    }
    if(p_env->obj_actived == DFU_PKG_ACTIVED_CMD){
        if(p_env->obj_recv_len + len <= p_env->obj_size){
            memcpy(&p_env->cmd_obj_buffer[p_env->obj_recv_len], data, len);
        }
    }else if(p_env->obj_actived == DFU_PKG_ACTIVED_DATA){
        write_image_data(data, len);
        struct {
            uint32_t offset;
            uint32_t max_length;
        } cb_info = {
            p_env->obj_recv_len,
            p_env->obj_size,
        };
        if(dfu_cb_itf.dfu_prog_cb){
            dfu_cb_itf.dfu_prog_cb(0, &cb_info);
        }
    }else{
        return;
    }
    // Cal CRC
    p_env->obj_crc32 = dfu_crc32(data, len, &p_env->obj_crc32);
    p_env->obj_recv_len += len;
    p_env->prn_cnt++;
    if(p_env->prn && p_env->prn <= p_env->prn_cnt){
        p_env->prn_cnt = 0;
        response->length = DFU_RESP_SIZE_CHECKSUM;
        response->rsp_code = DFU_CTRL_RESPONSE;
        response->opcode = DFU_CTRL_CAL_CHECKSUM;
        response->result = DFU_SUCCESS;
        response->data.checksum.offset = p_env->obj_recv_len;
        response->data.checksum.crc32 = p_env->obj_crc32;
    }
}

void dfu_read_version_char(dfu_version_t *version)
{
    version->desc_data.rsp_length = strlen(DFU_APP_DESCRIBE);
    version->desc_data.app_describe = DFU_APP_DESCRIBE;
}

void dfu_write_version_char(uint32_t cmd, dfu_version_t *version)
{
    memset(version, 0xFF, sizeof(dfu_version_t));
    if(cmd == 0x01BFDF5F){
        if(p_env){
            dfu_reset(DFU_UPDATE_ST_FAILED);
        }else{
            dfu_env_init();
        }
    }else if((cmd & 0xDFDFDFDF) == 0x524F4944){ // "ROID" recieved, Notify ROM ID
        memset(version, 0, sizeof(dfu_version_t));
        int res;
        uint32_t len;
        dfu_image_mbr_info mbr_info;
        const dfu_nvds_itf_t *mbr_itf = &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_MBR];
        mbr_itf->enable();
        len = sizeof(dfu_image_mbr_info);
        res = mbr_itf->get(IMAGE_TYPE_APP, &len, &mbr_info);
        if(res >= 0){
            uint32_t buf[5];
            const dfu_nvds_itf_t *flash_itf = &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_FLASH];
            flash_itf->enable();
            len = sizeof(buf);
            res = flash_itf->get(mbr_info.address+9*sizeof(uint32_t), &len, buf);
            if(res >= 0){
                version->version_data.address = buf[0];
                version->version_data.size = buf[1];
                version->version_data.version = buf[4];
            }
        }
        return;
    }else if((cmd & 0xDFDFDFDF) == 0x5645524E){ // "VERN" recieved, Notify Version
        memset(version, 0, sizeof(dfu_version_t));
        version->version_data.address = DFU_DATE_VERSION;
        version->version_data.size = DFU_TYPE_VERSION;
        version->version_data.version = DFU_PROTOCOL_VERSION;
        return;
    }
#if 0 // For debug
    uint8_t type = cmd;
    if(p_env){
        uint32_t addr = cmd & 0xFFFFFF00;
        if(type < DFU_NVDS_ITF_TYPE_MAX){
            const dfu_nvds_itf_t *nvds_itf = &dfu_nvds_itf[type];
            nvds_itf->enable();
            uint32_t len = sizeof(uint32_t) * 3;
            nvds_itf->get(addr, &len, version);
            nvds_itf->disable();
        }
    }else{
        int i;
        for(i=0;i<dfu_image_types_num;i++){
            if(type == dfu_image_types[i].type){
                const dfu_nvds_itf_t *info_itf = dfu_image_types[i].info_ops_itf;
                if(info_itf){
                    info_itf->enable();
                    uint32_t len = sizeof(dfu_version_t);
                    info_itf->get(dfu_image_types[i].info_id, &len, version);
                    info_itf->disable();
                }
                break;
            }
        }
        if(type < IMAGE_TYPE_MBR_MAX){
            const dfu_nvds_itf_t *info_itf = &dfu_nvds_itf[DFU_NVDS_ITF_TYPE_MBR];
            dfu_image_mbr_info mbr_info;
            info_itf->enable();
            uint32_t len = sizeof(mbr_info);
            int res = info_itf->get(type, &len, &mbr_info);
            info_itf->disable();
            if(res == ONMICRO_DFU_NVDS_ST_SUCCESS){
                version->version_data.address = mbr_info.address;
                version->version_data.size = mbr_info.length;
            }
        }
    }
#endif
}

int dfu_set_enable(bool enabled)
{
    if(env.status != DFU_STATUS_LOCKED){
        env.status = enabled?DFU_STATUS_ENABLED:DFU_STATUS_DISABLED;
    }
    return env.status;
}

#if DFU_FORCE_CHECK_SHA256_EN
__WEAK int dfu_sha256_cmp(uint8_t *sha256_resule, uint8_t sha256_len)
{
    return 0;
}
#endif

#endif /* #if BLE_APP_ONMICRO_DFU */
